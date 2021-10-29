// This file is used to identify redundant checks and remove them.

#include "llvm/Pass.h"
#include <utility>
#include <vector>
#include <map>
#include <set>
#include "llvm/Analysis/TargetTransformInfo.h"

namespace sanitychecks {
    class GCOVFile;
}

namespace llvm {
    class BranchInst;
    class raw_ostream;
    class Instruction;
    class Value;
}

struct SCIPass;

struct SRAnalysisPass : public llvm::ModulePass {
    static char ID;
    std::map<llvm::Instruction*,llvm::Instruction*> ReducedInst;

    SRAnalysisPass() : ModulePass(ID) {}

    virtual bool runOnModule(llvm::Module &M);

    virtual void getAnalysisUsage(llvm::AnalysisUsage& AU) const;
    
    void optimizeCheckAway(llvm::Instruction *Inst);
    bool compareValueDependency(llvm::Instruction *UC_Inst, llvm::Instruction *SC_Inst, uint64_t id1, uint64_t id2);
    bool reduceInstByCheck(llvm::Instruction *Inst);
private:

    SCIPass *SCI;
};
