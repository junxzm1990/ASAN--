#include "SlimasanProject.h"

#define SLICE_DEBUG 

using namespace llvm;

// globals
Dumper DUMP;

bool btraceInLoop(Value *v, std::vector<Value *> &backs, Loop *L) {
  
  if (auto* inst = dyn_cast<Instruction>(v)) {
    
    # ifdef SLICE_DEBUG
    LLVMContext& C = (*inst).getContext();
    MDNode* N = MDNode::get(C, MDString::get(C, "Backward Slicing Inst"));
    (*inst).setMetadata("ASAN.Backward_Slicing_Inst", N);
    # endif

		//return if going out of loop
		if (!L->contains(inst->getParent())) {      
			return false;
		}
	}
  // test if we have backtraced this value
  if(std::find(backs.begin(), backs.end(), v) != backs.end()){
    return false;
  }

  // check if v is a constant
  if(isa<ConstantData>(v) || isa<ConstantAggregate>(v)){
    return false;
  }

  if(isa<Argument>(v) || isa<GlobalObject>(v) || isa<AllocaInst>(v)){
    return false;
  }

  // mark that we have backtraced this value
  backs.push_back(v);

  // further decompose v
  bool res = false;

  if(ConstantExpr *i_cexp = dyn_cast<ConstantExpr>(v)){
    // treat constant expr as regular instructions
    User::op_iterator oi = i_cexp->op_begin(), oe = i_cexp->op_end();
    for(; oi != oe; ++oi){
      if(btraceInLoop(*oi, backs, L)){
        res = true;
      }
    }
  }

  else if(BranchInst *i_brch = dyn_cast<BranchInst>(v)){
    // only trace the condition value
    if(i_brch->isConditional()){
      if(btraceInLoop(i_brch->getCondition(), backs, L)){
        res = true;
      }
    }
  }

  else if(SwitchInst *i_swch = dyn_cast<SwitchInst>(v)){
    // If there is only the default destination, just branch.
    if (!i_swch->getNumCases()) {
      if(btraceInLoop(i_swch->getDefaultDest(), backs, L)){
        res = true;
      }
    } 
    else {
      // else only trace the condition value
      if(btraceInLoop(i_swch->getCondition(), backs, L)){
        res = true;
      }
    }
  }

  else if(CallInst *i_call = dyn_cast<CallInst>(v)){
    // NOTE: CallInst is a black hole which might suck up everything,
    // so take caution not to trace unwanted conditions
    if(i_call->isInlineAsm()){
      InlineAsm *bin = dyn_cast<InlineAsm>(i_call->getCalledValue());
      assert(bin != nullptr);
      std::string fn = bin->getAsmString();

      if(fn.empty()){
        llvm_unreachable("Asm string cannot be empty");
      }

// #define ASMCALL_BACK
// #include "Asmcall.def"
// #undef ASMCALL_BACK

      else {
        // default to ignore the rest
        /*
        errs() 
          << i_call->getParent()->getParent()->getName() << "::" 
          << fn << "\n";
        */
        // TODO handle the rest of the calls
      }
    }

    else {
      Function *tar = i_call->getCalledFunction();
      if(tar == nullptr){
        // TODO handle indirect call
      }

      else {
        // direct call, selectively trace the arguments
        std::string fn = tar->getName().str();
        if(tar->isIntrinsic()){
          Helper::convertDotInName(fn);
        }

        if(fn.empty()){
          llvm_unreachable("Function name cannot be empty");
        }

// #define LIBCALL_BACK
// #include "Libcall.def"
// #undef LIBCALL_BACK

        else {
          // default to ignore the rest
          /*
          errs() 
            << i_call->getParent()->getParent()->getName() << "::" 
            << fn << "\n";
          */
          // TODO handle the rest of the calls
        }
      }
    }
  }

  else if(isa<ExtractElementInst>(v) || isa<InsertElementInst>(v)){
    // handle these rare cases in a generic way
    Instruction *inst = cast<Instruction>(v);

    User::op_iterator oi = inst->op_begin(), oe = inst->op_end();
    for(; oi != oe; ++oi){
      if(btraceInLoop(*oi, backs, L)){
        res = true;
      }
    }
  }

  else if(BinaryOperator *i_bin = dyn_cast<BinaryOperator>(v)) {
    if(btraceInLoop(i_bin->getOperand(0), backs, L)){
      res = true;
    }

    if(btraceInLoop(i_bin->getOperand(1), backs, L)){
      res = true;
    }
  }

  else if(CastInst *i_cast = dyn_cast<CastInst>(v)){
    if(btraceInLoop(i_cast->getOperand(0), backs, L)){
      res = true;
    }
  }

  else if(CmpInst *i_cmp = dyn_cast<CmpInst>(v)){
    if(btraceInLoop(i_cmp->getOperand(0), backs, L)){
      res = true;
    }

    if(btraceInLoop(i_cmp->getOperand(1), backs, L)){
      res = true;
    }
  }

  else if(ExtractValueInst *i_ext = dyn_cast<ExtractValueInst>(v)){
    if(btraceInLoop(i_ext->getAggregateOperand(), backs, L)){
      res = true;
    }
  }

  else if(GetElementPtrInst *i_gep = dyn_cast<GetElementPtrInst>(v)){
    if(btraceInLoop(i_gep->getPointerOperand(), backs, L)){
      res = true;
    }

    User::op_iterator oi = i_gep->idx_begin(), oe = i_gep->idx_end();
    for(; oi != oe; ++oi){
      if(btraceInLoop(*oi, backs, L)){
        res = true;
      }
    }
  }

  else if(LoadInst *i_load = dyn_cast<LoadInst>(v)){
    if(btraceInLoop(i_load->getPointerOperand(), backs, L)){
      res = true;
    }
    //get all the users of the pointer operand
    //if a user is a store, inlude the store instruction into the slicing process
    for (User *U : i_load->getPointerOperand()->users()) {
      if (StoreInst *i_store = dyn_cast<StoreInst>(U)) {
        if(btraceInLoop(U, backs, L)){
          res = true;
        }
      }
    }
  }

  else if(PHINode *i_phi = dyn_cast<PHINode>(v)){
    unsigned num = i_phi->getNumIncomingValues();
    for(int i = 0; i < num; i++){
      if(btraceInLoop(i_phi->getIncomingValue(i), backs, L)){
        res = true;
      }

      TerminatorInst *pt = i_phi->getIncomingBlock(i)->getTerminator();
      // assert(isa<BranchInst>(pt) || isa<SwitchInst>(pt));

      if(btraceInLoop(pt, backs, L)){
        res = true;
      }
    }
  }

  else if(SelectInst *i_sel = dyn_cast<SelectInst>(v)){
    if(btraceInLoop(i_sel->getTrueValue(), backs, L)){
      res = true;
    }

    if(btraceInLoop(i_sel->getFalseValue(), backs, L)){
      res = true;
    }

    if(btraceInLoop(i_sel->getCondition(), backs, L)){
      res = true;
    }
  }

  else if(StoreInst *i_store = dyn_cast<StoreInst>(v)){
    if(btraceInLoop(i_store->getValueOperand(), backs, L)){
      res = true;
    }

    if(btraceInLoop(i_store->getPointerOperand(), backs, L)){
      res = true;
    }
  }

  else if(InsertValueInst *i_insert = dyn_cast<InsertValueInst>(v)){
    if(btraceInLoop(i_insert->getAggregateOperand(), backs, L)){
        res = true;
    }

    if(btraceInLoop(i_insert->getInsertedValueOperand(), backs, L)){
        res = true;
    }
  }
  
  else {
    // should have enumerated all
    // DUMP.typedValue(v);
    return false;
    // llvm_unreachable("Unknown value type to btrace");
  }

  return res;
}
