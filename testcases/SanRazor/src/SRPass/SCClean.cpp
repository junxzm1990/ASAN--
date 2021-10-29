// This file is used to clean insturctions related to removed sanitizer checks.

#include "SCClean.h"

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/CFG.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/Module.h"
#define DEBUG_TYPE "clean-sanitizer-check"

using namespace llvm;

bool SCClean::runOnModule(Module &M) {
    for (Function &F: M) {
        for (BasicBlock &BB: F) {
            for (Instruction &Inst: BB) {
                std::string OpName = Inst.getOpcodeName();
                if (OpName == "icmp" && Inst.getNumUses() == 0)
                    cleanSCInstructions(&Inst);
            }
        }
    }
    return false;
}

void SCClean::cleanSCInstructions(Instruction *Inst) {

    SmallPtrSet<Instruction*, 128> Worklist;
    Worklist.insert(Inst);
    while (!Worklist.empty()) {
        Instruction *I = *Worklist.begin();
        for (Use &U: I->operands()) {
            if (Instruction *Op = dyn_cast<Instruction>(U.get())) {
                if (Op->getNumUses()==1) {
                    Worklist.insert(Op);
                }
            }
        }
        Worklist.erase(I);
        if (I->getNumUses()==1) {
            I->replaceAllUsesWith(UndefValue::get(I->getType()));
            I->eraseFromParent();
        }
    }
}

char SCClean::ID = 0;

static RegisterPass<SCClean> X("scclean",
        "Finds branches belonging to sanity checks and user checks", false, false);
