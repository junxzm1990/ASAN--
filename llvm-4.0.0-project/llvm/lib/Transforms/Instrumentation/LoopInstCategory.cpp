#include "SlimasanProject.h"

using namespace llvm;

enum addrType addrTypeCalculation(bool BOstatus, std::vector<addrType> addrTypeCombination) {

  if (std::find(addrTypeCombination.begin(), addrTypeCombination.end(), UNKNOWN) != addrTypeCombination.end()) {
    return UNKNOWN;
  }

  // The status of base address and offset are known
  if (BOstatus) {

    if (addrTypeCombination[0] == IBIO && addrTypeCombination[1] == IBIO) {
      return IBIO;
    } 
    if (addrTypeCombination[0] == IBIO && addrTypeCombination[1] != IBIO) {
      return IBVO;
    }
    if (addrTypeCombination[0] != IBIO && addrTypeCombination[1] == IBIO) {
      return VBIO;
    }
    if (addrTypeCombination[0] != IBIO && addrTypeCombination[1] != IBIO) {
      return VBVO;
    }
  }
  // The status of base address and offset are unknown
  else {
    for (auto type : addrTypeCombination) {
      if (type != IBIO) {
        return VBVO;
      }
    }
    return IBIO;
  }

}

bool conservativeCheck(llvm::User *U) {

  if (CallInst *Call_Inst = dyn_cast<CallInst>(U)) {
    return false;
  }
  IntrinsicInst *II = dyn_cast<IntrinsicInst>(U);
  // Here we check if Intrinsic ID is lifetime_end
  if (II && II->getIntrinsicID() == Intrinsic::lifetime_end) {
    return false;
  }
  return true;
}

enum addrType checkAddrType(Value *addr, std::vector<Value *> backs, std::vector<Value *> &processedAddr, ScalarEvolution *SE, Loop *L) {

  // Check if current address is in backs vector
  if (std::find(backs.begin(), backs.end(), addr) == backs.end()) {
    for (User *U : addr->users()) {
      if (Instruction *UsrInst = dyn_cast<Instruction>(U)) {
        if (!L->contains(UsrInst)) {
          continue;
        }
      }
      if (CastInst *i_cast = dyn_cast<CastInst>(U)) {
        for (User *U : i_cast->users()) {
          if (!conservativeCheck(U)) {
            return UNKNOWN;
          }
        }
      }
      if (!conservativeCheck(U)) {
        return UNKNOWN;
      }
    } 
    return IBIO;
  }

  // Check if encounterd previously processed address
  if (std::find(processedAddr.begin(), processedAddr.end(), addr) != processedAddr.end()) {
    return VBVO;
  }

  // Check if current address is used by other functions
  for (User *U : addr->users()) {
    if (CallInst *Call_Inst = dyn_cast<CallInst>(U)) {
      if (!L->contains(Call_Inst)) {
        continue;
      }
      return UNKNOWN;
    }
  } 

  // mark that we have processed this address
  processedAddr.push_back(addr);

  if(CmpInst *Cmp_Inst = dyn_cast<CmpInst>(addr)){
    // Set the BOstatus to false since two operands are parallel to each other
    bool BOstatus = false;
    std::vector<addrType> addrTypeCombination;

    addrType firstOpType = checkAddrType(Cmp_Inst->getOperand(0), backs, processedAddr, SE, L);
    addrType secondOpType = checkAddrType(Cmp_Inst->getOperand(1), backs, processedAddr, SE, L);

    addrTypeCombination.push_back(firstOpType);
    addrTypeCombination.push_back(secondOpType);

    return addrTypeCalculation(BOstatus, addrTypeCombination);
  }

   if (GetElementPtrInst *Gep_Inst = dyn_cast<GetElementPtrInst>(addr)) {
    // If the status of base address and offset are known, set BOstatus to true, otherwise, set to false.
    bool BOstatus = true;
    std::vector<addrType> addrTypeCombination;

    addrType baseType = checkAddrType(Gep_Inst->getPointerOperand(), backs, processedAddr, SE, L);
    addrType offsetType = IBIO;

    for (auto& index : make_range(Gep_Inst->idx_begin(), Gep_Inst->idx_end())) {
      offsetType = checkAddrType(index, backs, processedAddr, SE, L);
      if (offsetType != IBIO) {
        offsetType = VBVO;
        break;
      }
    }

    addrTypeCombination.push_back(baseType);
    addrTypeCombination.push_back(offsetType);

    return addrTypeCalculation(BOstatus, addrTypeCombination);
  }

  if (SelectInst *Select_Inst = dyn_cast<SelectInst>(addr)) {
    // Set the BOstatus to false since the conditions are parallel to each other
    bool BOstatus = false;
    std::vector<addrType> addrTypeCombination;

    addrType trueType = checkAddrType(Select_Inst->getTrueValue(), backs, processedAddr, SE, L);
    addrType falseType = checkAddrType(Select_Inst->getFalseValue(), backs, processedAddr, SE, L);

    addrTypeCombination.push_back(trueType);
    addrTypeCombination.push_back(falseType);

    return addrTypeCalculation(BOstatus, addrTypeCombination);
  }

  if (BranchInst *Br_Inst = dyn_cast<BranchInst>(addr)) {
    // only process the condition value
    if (Br_Inst->isConditional()) {
      return checkAddrType(Br_Inst->getCondition(), backs, processedAddr, SE, L);
    }
  }

  if (CastInst *Cast_Inst = dyn_cast<CastInst>(addr)) {
    return checkAddrType(Cast_Inst->getOperand(0), backs, processedAddr, SE, L);
  }
  
  if (PHINode *Phi_Node = dyn_cast<PHINode>(addr)) {
    // Set the BOstatus to false since the conditions are parallel to each other

    bool BOstatus = false;
    std::vector<addrType> addrTypeCombination;

    // Phi node can have more than two conditions 
    unsigned num = Phi_Node->getNumIncomingValues();
    for (int i = 0; i < num; i++) {
      addrTypeCombination.push_back(checkAddrType(Phi_Node->getIncomingValue(i), backs, processedAddr, SE, L));
    }

    return addrTypeCalculation(BOstatus, addrTypeCombination);
  }

  if (StoreInst *Store_Inst = dyn_cast<StoreInst>(addr)) {

    // Set the BOstatus to false since two operands are parallel to each other
    bool BOstatus = false;
    std::vector<addrType> addrTypeCombination;

    addrType pointerType = checkAddrType(Store_Inst->getPointerOperand(), backs, processedAddr, SE, L);
    addrType valueType = checkAddrType(Store_Inst->getValueOperand(), backs, processedAddr, SE, L);

    addrTypeCombination.push_back(pointerType);
    addrTypeCombination.push_back(valueType);

    return addrTypeCalculation(BOstatus, addrTypeCombination);
  }

  if (LoadInst *Load_Inst = dyn_cast<LoadInst>(addr)) {

    // First check the user of address, to see if the address is modified by store instruction.
    Value *def_use_check = Load_Inst->getPointerOperand();
    for (Value *U : def_use_check->users()) {
      if (ConstantExpr *i_cexp = dyn_cast<ConstantExpr>(U)) {
        for (Value *U : i_cexp->users()) {
          if (StoreInst *i_store = dyn_cast<StoreInst>(U)) {
            if (!L->contains(i_store)) {
              continue;
            }
            return VBVO;
          }
        }
      }

      if (CastInst *i_cast = dyn_cast<CastInst>(U)) {
        for (Value *U : i_cast->users()) {
          if (StoreInst *i_store = dyn_cast<StoreInst>(U)) {
            if (!L->contains(i_store)) {
              continue;
            }
            return VBVO;
          }
        }
      }

      if (StoreInst *i_store = dyn_cast<StoreInst>(U)) {
        if (!L->contains(i_store)) {
          continue;
        }
        return VBVO;
      }
    }

    return checkAddrType(Load_Inst->getPointerOperand(), backs, processedAddr, SE, L);
  }

  if (BinaryOperator *BinOp_Inst = dyn_cast<BinaryOperator>(addr)) {
    
    // Set the BOstatus to false since two operands are parallel to each other
    bool BOstatus = false;
    std::vector<addrType> addrTypeCombination;

    addrType firstOpType = checkAddrType(BinOp_Inst->getOperand(0), backs, processedAddr, SE, L);
    addrType secondOpType = checkAddrType(BinOp_Inst->getOperand(1), backs, processedAddr, SE, L);

    addrTypeCombination.push_back(firstOpType);
    addrTypeCombination.push_back(secondOpType);

    return addrTypeCalculation(BOstatus, addrTypeCombination);
  }

  return UNKNOWN;
}
