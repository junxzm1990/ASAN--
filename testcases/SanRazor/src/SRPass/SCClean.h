// This file is used to clean insturctions related to removed sanitizer checks.

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Pass.h"
#include "llvm/IR/Instructions.h"



#include <map>
#include <set>

namespace llvm {
    class AnalysisUsage;
    class BasicBlock;
    class CallInst;
    class Function;
    class Instruction;
    class Value;
    class StringRef;
    class dyn_cast;
}


struct SCClean : public llvm::ModulePass {
    static char ID;

    SCClean() : ModulePass(ID) {}

    virtual bool runOnModule(llvm::Module &M);

    virtual void getAnalysisUsage(llvm::AnalysisUsage& AU) const {
        AU.setPreservesAll();
    }
    
private:

    void cleanSCInstructions(llvm::Instruction *Inst);
};
