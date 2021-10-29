// This file is modified from ASAP, containing some useful functions.

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "SCIPass.h"
using namespace llvm;

bool isAbortingCall(const CallInst *CI) {
    if (CI->getCalledFunction()) {
        StringRef name = CI->getCalledFunction()->getName();
        if (name.startswith("__ubsan_handle")) {
            return true;
        }
        // if (name.startswith("__softboundcets_") && name.endswith("_abort")) {
        //     return true;
        // }
        if (name.startswith("__asan_report")) {
            return true;
        }
        // if (name == "__assert_fail" || name == "__assert_rtn") {
        //     return true;
        // }
    }
    return false;
}

unsigned int getRegularBranch(BranchInst *BI, SCIPass *SCI) {
    unsigned int RegularBranch = (unsigned)(-1);
    Function *F = BI->getParent()->getParent();
    for (unsigned int I = 0, E = BI->getNumSuccessors(); I != E; ++I) {
        if (!SCI->getSanityCheckBlocks(F).count(BI->getSuccessor(I))) {
            assert(RegularBranch == (unsigned)(-1) && "More than one regular branch?");
            RegularBranch = I;
        }
    }
    if (RegularBranch == (unsigned)(-1)) {
        bool flag = false;
        for (unsigned int I = 0;I<2;I++) {
            if (!flag) {
                for (const Instruction &Inst: *(BI->getSuccessor(I))) {
                    if (const CallInst *CI = dyn_cast<CallInst>(&Inst)) {
                        if (isAbortingCall(CI)) {
                            flag = true;
                            break;
                            RegularBranch = 1 - I;
                        }
                    }
                }
            }
        }
    }
    return RegularBranch;
}

bool getCheckType(Instruction *Inst, SCIPass *SCI) {
    BranchInst *BI = cast<BranchInst>(Inst);
    unsigned int RegularBranch = (unsigned)(-1);
    Function *F = BI->getParent()->getParent();
    for (unsigned int I = 0, E = BI->getNumSuccessors(); I != E; ++I) {
        if (!SCI->getSanityCheckBlocks(F).count(BI->getSuccessor(I))) {
            assert(RegularBranch == (unsigned)(-1) && "More than one regular branch?");
            RegularBranch = I;
        }
    }
    StringRef name = "";
    if (RegularBranch == 0) {
        for (const Instruction &Inst: *(BI->getSuccessor(1))) {
            if (const CallInst *CI = dyn_cast<CallInst>(&Inst)) {
                if (CI->getCalledFunction()) {
                    name = CI->getCalledFunction()->getName();
                }
            }
        }
    }
    else if (RegularBranch == 1) {
        for (const Instruction &Inst: *(BI->getSuccessor(0))) {
            if (const CallInst *CI = dyn_cast<CallInst>(&Inst)) {
                if (CI->getCalledFunction()) {
                    name = CI->getCalledFunction()->getName();
                }
            }
        }
    }
    if (name.startswith("__asan_report")) {
        return true;
    }
    else {
        return false;
    }
}