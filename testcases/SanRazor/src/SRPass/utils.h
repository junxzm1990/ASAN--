// This file is modified from ASAP, containing some useful functions.

#ifndef ETHPASS_UTILS_H
#define	ETHPASS_UTILS_H

#include "llvm/IR/DebugLoc.h"

namespace llvm {
    class BranchInst;
    class CallInst;
    class LLVMContext;
    class raw_ostream;
}

struct SCIPass;

bool isAbortingCall(const llvm::CallInst *CI);

unsigned int getRegularBranch(llvm::BranchInst *BI, SCIPass *SCI);

bool getCheckType(llvm::Instruction *Inst, SCIPass *SCI);

#endif	/* ETHPASS_UTILS_H */