// This file is modified from ASAP, used to identify user checks and sanitizer checks.
// Note that ASAP only identifies sanitizer checks.

#include "SCIPass.h"
#include "utils.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/CFG.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/Module.h"
#define DEBUG_TYPE "sanity-check-instructions"

using namespace llvm;

bool SCIPass::runOnModule(Module &M) {
    dbgs() << "Start SCIPass on " << M.getSourceFileName() << "\n";
    for (Function &F: M) {
        SanityCheckBlocks[&F] = BlockSet();
        SanityCheckInstructions[&F] = InstructionSet();
        SCBranches[&F] = InstructionVec();
        UCBranches[&F] = InstructionVec();
        findInstructions(&F);

        MDNode *MD = MDNode::get(M.getContext(), {});
        for (Instruction *Inst: SanityCheckInstructions[&F]) {
            Inst->setMetadata("sanitycheck", MD);
        }
    }
    dbgs() << "End SCIPass on " << M.getSourceFileName() << "\n";
    return true;
}

void SCIPass::findInstructions(Function *F) {

    // A list of instructions that are used by sanity checks. They become sanity
    // check instructions if it turns out they're not used by anything else.
    std::set<Instruction*> Worklist;
    
    // A list of basic blocks that contain sanity check instructions. They
    // become sanity check blocks if it turns out they don't contain anything
    // else.
    std::set<BasicBlock*>   BlockWorklist;

    // A map from instructions to the checks that use them.
    
    // FILE *ff = fopen("./Cov/checkh.txt", "ab");
    for (BasicBlock &BB: *F) {

        if (findSanityCheckCall(&BB)) {
            SanityCheckBlocks[F].insert(&BB);

            // All instructions inside sanity check blocks are sanity check instructions
            for (Instruction &I: BB) {
                Worklist.insert(&I);
            }

            // All branches to sanity check blocks are sanity check branches
            for (User *U: BB.users()) {
                if (Instruction *Inst = dyn_cast<Instruction>(U)) {
                    Worklist.insert(Inst);
                }
                BranchInst *BI = dyn_cast<BranchInst>(U);
                if (BI && BI->isConditional()) {
                    SCBranches[F].push_back(BI);
                    // fprintf(ff, "%s ", F->getName());
                    // for (Instruction &I: *BI->getParent()){
                    //     fprintf(ff, ":%s",I.getOpcodeName());
                    // }
                    // fprintf(ff, "\n");
                    UCBranches[F].remove(dyn_cast<Instruction>(U));
                    ChecksByInstruction[BI].insert(BI);
                }
            }
        }
        // ******
        else{
            // User checks but contain potential sanity checks
            for (Instruction &I: BB) {
                BranchInst *BI = dyn_cast<BranchInst>(&I);
                if (BI && BI->isConditional()) {
                    UCBranches[F].push_back(&I);
                }
            }
        }
        // ******
    }

    while (!Worklist.empty()) {
        // Alternate between emptying the worklist...
        while (!Worklist.empty()) {
            Instruction *Inst = *Worklist.begin();
            Worklist.erase(Inst);
            if (onlyUsedInSanityChecks(Inst)) {
                if (SanityCheckInstructions[F].insert(Inst).second) {
                    UCBranches[F].remove(Inst);
                    for (Use &U: Inst->operands()) {
                        if (Instruction *Op = dyn_cast<Instruction>(U.get())) {
                            Worklist.insert(Op);

                            // Copy ChecksByInstruction from Inst to Op
                            auto CBI = ChecksByInstruction.find(Inst);
                            if (CBI != ChecksByInstruction.end()) {
                                ChecksByInstruction[Op].insert(CBI->second.begin(), CBI->second.end());
                            }
                        }
                    }

                    BlockWorklist.insert(Inst->getParent());

                    // Fill InstructionsBySanityCheck from the inverse ChecksByInstruction
                    auto CBI = ChecksByInstruction.find(Inst);
                    if (CBI != ChecksByInstruction.end()) {
                        for (Instruction *CI : CBI->second) {
                            InstructionsBySanityCheck[CI].insert(Inst);
                        }
                    }
                }
            }
        }

        // ... and checking whether this causes basic blocks to contain only
        // sanity checks. This would in turn cause terminators to be added to
        // the worklist.
        while (!BlockWorklist.empty()) {
            BasicBlock *BB = *BlockWorklist.begin();
            BlockWorklist.erase(BB);
            
            bool allInstructionsAreSanityChecks = true;
            for (Instruction &I: *BB) {
                if (!SanityCheckInstructions.at(BB->getParent()).count(&I)) {
                    allInstructionsAreSanityChecks = false;
                    break;
                }
            }
            
            if (allInstructionsAreSanityChecks) {
                // SanityCheckBlocksPlus[F].insert(BB);
                for (User *U: BB->users()) {
                    if (Instruction *Inst = dyn_cast<Instruction>(U)) {
                        Worklist.insert(Inst);
                        BranchInst *BI = dyn_cast<BranchInst>(Inst);
                        if (BI && BI->isConditional()) {
                            for (Instruction &I: *BB) {
                                auto CBI = ChecksByInstruction.find(&I);
                                if (CBI != ChecksByInstruction.end() && ChecksByInstruction.find(BI) == ChecksByInstruction.end()) {
                                    ChecksByInstruction[BI].insert(CBI->second.begin(), CBI->second.end());
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    // fclose(ff);
}

const CallInst *SCIPass::findSanityCheckCall(BasicBlock* BB) const {
    for (const Instruction &I: *BB) {
        if (const CallInst *CI = dyn_cast<CallInst>(&I)) {
            if (isAbortingCall(CI)) {
                return CI;
            }
        }
    }
    return 0;
}

bool SCIPass::onlyUsedInSanityChecks(Value* V) {
    for (User *U: V->users()) {
        Instruction *Inst = dyn_cast<Instruction>(U);
        if (!Inst) return false;
        
        Function *F = Inst->getParent()->getParent();
        if (!(SanityCheckInstructions[F].count(Inst))) {
            return false;
        }
    }
    return true;
}

char SCIPass::ID = 0;

static RegisterPass<SCIPass> X("sci",
        "Finds branches belonging to sanity checks and user checks", false, false);
