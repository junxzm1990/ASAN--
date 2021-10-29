// This file is modified from ASAP, used to identify user checks and sanitizer checks.
// Note that ASAP only identifies sanitizer checks.

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Pass.h"
#include "llvm/IR/Instructions.h"
#include <map>
#include <set>
#include <list>

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

struct SCIPass : public llvm::ModulePass {
    static char ID;

    SCIPass() : ModulePass(ID) {}

    virtual bool runOnModule(llvm::Module &M);

    virtual void getAnalysisUsage(llvm::AnalysisUsage& AU) const {
        AU.setPreservesAll();
    }

    // Types used to store sanity check blocks / instructions
    typedef std::set<llvm::BasicBlock*> BlockSet;
    typedef std::set<llvm::Instruction*> InstructionSet;
    typedef std::list<llvm::Instruction*> InstructionVec;

    const InstructionVec &getSCBranches(llvm::Function *F) const {
        return SCBranches.at(F);
    }

    const InstructionVec &getSCBranchesV(llvm::Function *F) const {
        return SCBranchesV.at(F);
    }
    
    const BlockSet &getSanityCheckBlocks(llvm::Function *F) const {
        return SanityCheckBlocks.at(F);
    }

    const InstructionVec &getUCBranches(llvm::Function *F) const {
        return UCBranches.at(F);
    }

    const InstructionSet &getInstructionsBySanityCheck(llvm::Instruction *Inst) const {
        return InstructionsBySanityCheck.at(Inst);
    }

    const InstructionSet &getChecksByInstruction(llvm::Instruction *Inst) const {
        return ChecksByInstruction.at(Inst);
    }

    // Searches the given basic block for a call instruction that corresponds to
    // a sanity check and will abort the program (e.g., __assert_fail).
    const llvm::CallInst *findSanityCheckCall(llvm::BasicBlock *BB) const;
    
private:

    // All blocks that abort due to sanity checks
    std::map<llvm::Function*, BlockSet> SanityCheckBlocks;
    // std::map<llvm::Function*, BlockSet> SanityCheckBlocksPlus;
    std::map<llvm::Instruction*, InstructionSet> ChecksByInstruction;

    // All instructions that belong to sanity checks
    std::map<llvm::Function*, InstructionSet> SanityCheckInstructions;
    std::map<llvm::Instruction*, InstructionSet> InstructionsBySanityCheck;
    
    // All sanity checks themselves (branch instructions that could lead to an abort)
    std::map<llvm::Function*, InstructionVec> SCBranches;
    std::map<llvm::Function*, InstructionVec> SCBranchesV;

    std::map<llvm::Function*, InstructionVec> UCBranches;

    void findInstructions(llvm::Function *F);
    bool onlyUsedInSanityChecks(llvm::Value *V);
};
