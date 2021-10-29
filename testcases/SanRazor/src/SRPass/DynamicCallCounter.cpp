// This file is used to insert counters before checks, in order to get their dynamic patterns.

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"

#include "DynamicCallCounter.h"
#include "SCIPass.h"

#define DEBUG_TYPE "dccpass"

using namespace llvm;


// Returns a map (Function* -> uint64_t).
static DenseMap<Instruction*, uint64_t>
computeFunctionIDs(ArrayRef<Instruction*> instructions, uint64_t nextID) {
    DenseMap<Instruction*, uint64_t> idMap;

    for (auto inst : instructions) {
        idMap[inst] = nextID;
        ++nextID;
    }

    return idMap;
}

// Create the CCOUNT(functionInfo) table used by the runtime library.
static void
createBranchTable(Module& m, std::vector<Instruction*> toCount, uint64_t numBranchInsts, std::string str) {
    LLVMContext& context = m.getContext();

    // Create the component types of the table
    auto* int64Ty    = Type::getInt64Ty(context);
    auto* int64ArrTy = ArrayType::get(int64Ty, 3);
    // auto* stringTy   = Type::getInt8PtrTy(context);
    // ArrayRef<Type*> fieldTys = {stringTy, int64ArrTy};
    auto* structTy   = StructType::get(context, {int64Ty, int64ArrTy}, false);
    auto* tableTy    = ArrayType::get(structTy, numBranchInsts);
    ArrayRef<uint64_t> COUNT = {0, 0, 0};
    auto* COUNTs = ConstantDataArray::get(context, COUNT);
    auto* ID = ConstantInt::get(int64Ty, 0, false);

    // Compute and store an externally visible array of branch information.
    std::vector<Constant*> values;
    std::transform(
        toCount.begin(),
        toCount.end(),
        std::back_inserter(values),
        [&m, ID, COUNTs, structTy, str](Instruction* Inst) {
            Constant* structFields[] = {ID, COUNTs};
        return ConstantStruct::get(structTy, structFields);
        });
    auto* BranchTable = ConstantArray::get(tableTy, values);
    new GlobalVariable(m,
                     tableTy,
                     false,
                     GlobalValue::ExternalLinkage,
                     BranchTable,
                     "COUNTER_"+str);
}

// For an analysis pass, runOnModule should perform the actual analysis and
// compute the results. The actual output, however, is produced separately.
bool
DynamicCallCounter::runOnModule(Module &m) {

    LLVMContext& context = m.getContext();
    std::string filename = m.getSourceFileName();
    filename = filename.substr(0, filename.rfind("."));
    std::replace(filename.begin(), filename.end(), '/', '_');
    std::replace(filename.begin(), filename.end(), '-', '_');
    std::replace(filename.begin(), filename.end(), '.', '_');
    std::replace(filename.begin(), filename.end(), '+', '_');
    dbgs() << filename << "\n";

  // First identify the functions we wish to track
    std::vector<Instruction*> CountSC;
    std::vector<Instruction*> CountUC;
    SCIPass &SCI = getAnalysis<SCIPass>();

    for (Function &F: m) {
        LLVM_DEBUG(dbgs() << "Get SCs (sanitizer checks) and UCs (user checks) on file: " << F.getName() << ".\n");
        for (Instruction *Inst: SCI.getSCBranches(&F)) {
            assert(Inst->getParent()->getParent() == &F && "SCI must only contain instructions of the current function.");
            BranchInst *BI = dyn_cast<BranchInst>(Inst);
            assert(BI && BI->isConditional() && "SCBranches must not contain instructions that aren't conditional branches.");
            CountSC.push_back(BI);
        }
        for (Instruction *Inst: SCI.getUCBranches(&F)) {
            assert(Inst->getParent()->getParent() == &F && "SCI must only contain instructions of the current function.");
            BranchInst *BI = dyn_cast<BranchInst>(Inst);
            assert(BI && BI->isConditional() && "UCBranches must not contain instructions that aren't conditional branches.");
            CountUC.push_back(BI);
        }
    }


    ids_SC = computeFunctionIDs(CountSC, num_SC);
    ids_UC = computeFunctionIDs(CountUC, num_UC);
    auto const numSCBranches = CountSC.size();
    auto const numUCBranches = CountUC.size();
    dbgs() << "File name:" << m.getSourceFileName() << ", No. of SCs: " << CountSC.size() << ", No. of UCs: " <<CountUC.size()<<".\n";


    auto* int64Ty = Type::getInt64Ty(context);
    auto* int1Ty = Type::getInt1Ty(context);
    auto* numSCBranchesGlobal = ConstantInt::get(int64Ty, numSCBranches, false);
    auto* numUCBranchesGlobal = ConstantInt::get(int64Ty, numUCBranches, false);
    new GlobalVariable(m,
                     int64Ty,
                     true,
                     GlobalValue::ExternalLinkage,
                     numSCBranchesGlobal,
                     "COUNTER_numSCBranches"+filename);
    new GlobalVariable(m,
                     int64Ty,
                     true,
                     GlobalValue::ExternalLinkage,
                     numUCBranchesGlobal,
                     "COUNTER_numUCBranches"+filename);

    createBranchTable(m, CountSC, numSCBranches, "SCBranchInfo"+filename);
    createBranchTable(m, CountUC, numUCBranches, "UCBranchInfo"+filename);


    Type* voidTy  = Type::getVoidTy(context);
    Value* printerSC = m.getOrInsertFunction("COUNTER_printSC"+filename, voidTy).getCallee();
    appendToGlobalDtors(m, llvm::cast<Function>(printerSC), 0);
    Value* printerUC = m.getOrInsertFunction("COUNTER_printUC"+filename, voidTy).getCallee();
    appendToGlobalDtors(m, llvm::cast<Function>(printerUC), 0);

    FunctionType* countSCTy = FunctionType::get(voidTy, {int64Ty, int64Ty}, false);
    Value* counterSC  = m.getOrInsertFunction("COUNTER_calledSC"+filename, countSCTy).getCallee();

    dbgs() << "Insert call functions for SC branches in file: "<<m.getSourceFileName()<<"!\n";
    for (Instruction* I : CountSC) {
        handleCalledBranch(m, *I, counterSC, "SC", filename);
    }

    FunctionType* countUCTy = FunctionType::get(voidTy, {int64Ty, int1Ty}, false);
    Value* counterUC  = m.getOrInsertFunction("COUNTER_calledUC"+filename, countUCTy).getCallee();

    dbgs() << "Insert call functions for UC branches in file: "<<m.getSourceFileName()<<"!\n";
    for (Instruction* I : CountUC) {
        handleCalledBranch(m, *I, counterUC, "UC", filename);
    }
    dbgs() << "DCC Pass completed.\n";

    return true;
}


void
DynamicCallCounter::handleCalledBranch(Module& m, Instruction& I, Value* counter, std::string str, std::string filename) {
    BranchInst *BI = dyn_cast<BranchInst>(&I);
    if (BI && BI->isConditional()) {
        if (str == "SC") {
            uint64_t type = 0;
            IRBuilder<> builderI(&I);
            builderI.CreateCall(counter, {builderI.getInt64(ids_SC[&I]),builderI.getInt64(type)});

            BasicBlock *BB = BI->getSuccessor(0);
            IRBuilder<> builderA(&*BB->getFirstInsertionPt());
            type = 1;
            builderA.CreateCall(counter, {builderA.getInt64(ids_SC[&I]),builderA.getInt64(type)});

            BB = BI->getSuccessor(1);
            IRBuilder<> builderB(&*BB->getFirstInsertionPt());
            type = 2;
            builderB.CreateCall(counter, {builderB.getInt64(ids_SC[&I]),builderB.getInt64(type)});
        }
        else if (str == "UC") {
            IRBuilder<> builderI(&I);
            builderI.CreateCall(counter, {builderI.getInt64(ids_UC[&I]), I.getOperand(0)});
        }
    }
}

void DynamicCallCounter::getAnalysisUsage(AnalysisUsage& AU) const {
    AU.addRequired<SCIPass>();
    AU.setPreservesAll();
}


char DynamicCallCounter::ID = 0;
static RegisterPass<DynamicCallCounter> X("dcc",
        "Count analysis", false, false);
