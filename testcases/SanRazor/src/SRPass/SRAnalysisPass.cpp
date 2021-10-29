// This file is used to identify redundant checks and remove them.

#include "SRAnalysisPass.h"
#include "SCIPass.h"
#include "utils.h"
#include "CostModel.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include <algorithm>
#include <memory>
#include <system_error>
#define DEBUG_TYPE "SRAnalysisPass"

using namespace llvm;


static cl::opt<std::string>
InputLevel("sropt-level", cl::desc("<L0, L1, L2>"), cl::init(""), cl::Hidden);

static cl::opt<std::string>
InputSCOV("scov", cl::desc("<input scov file>"), cl::init(""), cl::Hidden);

static cl::opt<std::string>
InputUCOV("ucov", cl::desc("<input ucov file>"), cl::init(""), cl::Hidden);

static cl::opt<std::string>
InputLOG("log", cl::desc("<input log file>"), cl::init(""), cl::Hidden);

static cl::opt<double>
Threshold("use-asap", cl::desc("Fraction of dynamic checks to be preserved"), cl::init(-1.0));

bool SRAnalysisPass::runOnModule(Module &m) {
    SCI = &getAnalysis<SCIPass>();
    TargetTransformInfoWrapperPass &TTIWP = getAnalysis<TargetTransformInfoWrapperPass>();
    std::string filename = m.getSourceFileName();
    filename = filename.substr(0, filename.rfind("."));

    struct brInfo {
        uint64_t id;
        uint64_t count[3];
    } BrInfo;

    struct brStat {
        uint64_t id;
        uint64_t LB;
        uint64_t RB;
        Instruction* SC;
        double CostLevel1;
        double CostLevel2;
        double NumLevel1;
        double NumLevel2;
        uint64_t count[3];
    };

    std::map<Instruction*, brStat> SC_Stat;
    std::map<uint64_t, std::vector<brStat>> SC_Pattern;
    std::map<uint64_t, uint64_t> reducedSC;
    std::map<uint64_t, std::vector<brStat>> CostLevelRange;

    FILE *fp_sc = fopen(Twine(InputSCOV).str().c_str(), "rb");
    FILE *fp_uc = fopen(Twine(InputUCOV).str().c_str(), "rb");
    if (fp_sc != NULL && fp_uc != NULL) {
        dbgs() << "SRAnalysisPass on file: "<<filename << ", SC dynamic pattern file: " << Twine(InputSCOV).str().c_str() << "\n";

        uint64_t num_SC = 0, total_cost_SC = 0; // Number of SCs
        uint64_t num_UC = 0, total_cost_UC = 0; // Number of UCs
        uint64_t num_SC_opt = 0, total_cost_SC_opt = 0; // Number of SCs after the redundant SCs about UCs are reduced
        uint64_t num_SC_opts = 0, total_cost_SC_opts = 0; // Number of SCs after the redundant SCs about SCs are reduced
        uint64_t count_remaining_SC = 0, count_remaining_cost_SC = 0, count_reduced_SC = 0;
        bool is_reduced = true;

        // Start reading and storing SC coverage records from InputSCOV
        for (Function &F: m) {
            for (Instruction *Inst: SCI->getSCBranches(&F)) {
                assert(Inst->getParent()->getParent() == &F && "SCI must only contain instructions of the current function.");
                BranchInst *BI = dyn_cast<BranchInst>(Inst);
                assert(BI && BI->isConditional() && "SCBranches must not contain instructions that aren't conditional branches.");
                fread(&BrInfo, sizeof(BrInfo), 1, fp_sc);

                BrInfo.count[0] = BrInfo.count[1] + BrInfo.count[2];
                
                for (size_t i=0;i<3;i++){
                    SC_Stat[Inst].count[i] = BrInfo.count[i];
                }
                SC_Stat[Inst].id = BrInfo.id;
                SC_Stat[Inst].CostLevel1 = 0;
                SC_Stat[Inst].CostLevel2 = 0;
                SC_Stat[Inst].NumLevel1 = 0;
                SC_Stat[Inst].NumLevel2 = 0;
                SC_Stat[Inst].LB = BrInfo.count[1];
                SC_Stat[Inst].RB = BrInfo.count[2];
                SC_Stat[Inst].SC = Inst;
                // Store the dynamic patterns of instructions in SCBranches in SC_Pattern (totalCovTime->SC_Info)
                // SC_Info contains id, LB, RB, and *Inst
                // struct brStat SC_Info;
                // SC_Info.id = BrInfo.id;
                // SC_Info.LB = BrInfo.count[1];
                // SC_Info.RB = BrInfo.count[2];
                // SC_Info.SC = Inst;
                SC_Pattern[BrInfo.count[0]].push_back(SC_Stat[Inst]);
                num_SC += 1;
                total_cost_SC += BrInfo.count[0];
            }
        }
        fclose(fp_sc);
        num_SC_opt = num_SC;
        total_cost_SC_opt = total_cost_SC;
        dbgs() << "No. of SCs: " << num_SC << ", total cost of SCs: " <<total_cost_SC << ".\n";
        // Finish reading and storing SC coverage records from InputSCOV

        // Start reading and storing UC coverage records from InputUCOV
        // During this process, each UC will be compared with all SCs in SC_Pattern
        // For each SC in SC_Pattern, if its coverage pattern matches that of UC
        // And if the variable it operates is the same as that operated by UC
        // Then, the SC can be reduced.

        for (Function &F: m) {
            for (Instruction *Inst: SCI->getUCBranches(&F)) {
                assert(Inst->getParent()->getParent() == &F && "SCI must only contain instructions of the current function.");
                BranchInst *BI = dyn_cast<BranchInst>(Inst);
                assert(BI && BI->isConditional() && "UCBranches must not contain instructions that aren't conditional branches.");
                fread(&BrInfo, sizeof(BrInfo), 1, fp_uc);

                num_UC += 1;
                total_cost_UC += BrInfo.count[0];

                // For each instruction in UCBranch, check whether its coverage pattern matches certain patterns in SC_Pattern
                if (SC_Pattern.count(BrInfo.count[0]) > 0 && BrInfo.count[0] > 0) {
                    // If UC and SC have ompletely the same dynamic pattern A+B:A:B
                    for (brStat Info: SC_Pattern[BrInfo.count[0]]) {
                        if ((Info.LB == BrInfo.count[1] && Info.RB == BrInfo.count[2]) || (Info.LB == BrInfo.count[2] && Info.RB == BrInfo.count[1])) {
                            // If UC and SC operate the same variable
                            // dbgs() << "TTT"<<Info.id << "---";
                            if (compareValueDependency(Inst, Info.SC,BrInfo.id,Info.id) && reducedSC.count(Info.id) == 0) {
                                optimizeCheckAway(Info.SC);
                                // dbgs() << "Redundant UC id:" << BrInfo.id<<", SC id:"<<Info.id <<", covered times: "<< BrInfo.count[0]<<".\n";
                                num_SC_opt -= 1;
                                total_cost_SC_opt -= BrInfo.count[0];
                                reducedSC[Info.id] = BrInfo.count[0];
                            }
                        }
                    }
                }
                else if (SC_Pattern.count(BrInfo.count[1]) > 0 && BrInfo.count[1] > 0) {
                    // UC has pattern A+B:A:B, while SC has pattern A:A:0
                    for (brStat Info: SC_Pattern[BrInfo.count[1]]) {
                        if (Info.LB == 0 || Info.RB == 0) {
                            // If UC and SC operate the same variable
                            if (compareValueDependency(Inst, Info.SC,BrInfo.id,Info.id) && reducedSC.count(Info.id) == 0) {
                                optimizeCheckAway(Info.SC);
                                // dbgs() << "Redundant UC id:" << BrInfo.id<<", SC id:"<<Info.id <<", covered times: "<< BrInfo.count[1]<<".\n";
                                num_SC_opt -= 1;
                                total_cost_SC_opt -= BrInfo.count[1];
                                reducedSC[Info.id] = BrInfo.count[1];
                            }
                        }
                    }
                }
                else if (SC_Pattern.count(BrInfo.count[2]) > 0 && BrInfo.count[2] > 0) {
                    // UC has pattern A+B:A:B, while SC has pattern B:B:0
                    for (brStat Info: SC_Pattern[BrInfo.count[2]]) {
                        if (Info.LB == 0 || Info.RB == 0){
                            if (compareValueDependency(Inst, Info.SC, BrInfo.id,Info.id) && reducedSC.count(Info.id) == 0) {
                                optimizeCheckAway(Info.SC);
                                // dbgs() << "Redundant UC id:" << BrInfo.id<<", SC id:"<<Info.id <<", covered times: "<< BrInfo.count[2]<<".\n";
                                num_SC_opt -= 1;
                                total_cost_SC_opt -= BrInfo.count[2];
                                reducedSC[Info.id] = BrInfo.count[2];
                            }
                        }
                    }
                }
            }
        }
        fclose(fp_uc);
        num_SC_opts = num_SC_opt;
        total_cost_SC_opts = total_cost_SC_opt;
        dbgs() << "After removing SCs redundant with UCs, No. of SCs: " << num_SC_opt << ", remaining cost of SCs: " << total_cost_SC_opt << ".\n";
        
        // Finish reading and storing UC coverage records from InputUCOV
        // Reduce redundant SCs among SCs
        // For each SC read from SCOV file
        // Check whether its pattern matches any SCs in SC_Pattern
        // If it is, this SC can be reduced
        fp_sc = fopen(Twine(InputSCOV).str().c_str(), "rb");
        uint64_t Cost = 0, total_asap_cost = 0, total_asap_cost_opt = 0;
        for (Function &F: m) {
            const TargetTransformInfo &TTI = TTIWP.getTTI(F);
            for (Instruction *Inst: SCI->getSCBranches(&F)) {
                assert(Inst->getParent()->getParent() == &F && "SCI must only contain instructions of the current function.");
                BranchInst *BI = dyn_cast<BranchInst>(Inst);
                assert(BI && BI->isConditional() && "SCBranches must not contain instructions that aren't conditional branches.");
                fread(&BrInfo, sizeof(BrInfo), 1, fp_sc);
                // dbgs() <<"SC id:"<<BrInfo.id << ", dynamic pattern: <"<< BrInfo.count[0] <<", "<< BrInfo.count[1] <<", "<< BrInfo.count[2] << ">.\n";
                BrInfo.count[0] = BrInfo.count[1] + BrInfo.count[2];

                // Calculate cost of each checks
                Cost = 0;
                for (Instruction *CI: SCI->getInstructionsBySanityCheck(BI)) {
                    unsigned CurrentCost = CheckCost::getInstructionCost(CI, &TTI);
                    if (CurrentCost == (unsigned)(-1)) {
                        CurrentCost = 1;
                    }
                    Cost += CurrentCost * BrInfo.count[0];
                }
                total_asap_cost += Cost;
                total_asap_cost_opt += Cost;
                CostLevelRange[Cost].push_back(SC_Stat[Inst]);


                // Set a flag to record whether the SC can be reduced
                is_reduced = false;
                // For each instruction in SCBranch, check whether its dynamic pattern matches certain patterns in SC_Pattern
                if (SC_Pattern.count(BrInfo.count[0]) > 0 && BrInfo.count[0] > 0) {
                    // New SC and existing SC have ompletely the same dynamic pattern A+B:A:B
                    // Check all SCs in SC_Pattern
                    for (brStat Info: SC_Pattern[BrInfo.count[0]]) {
                        if (BrInfo.count[1] == Info.LB || BrInfo.count[2] == Info.LB) {
                            // Also the same operation variable 
                            if (BrInfo.id != Info.id && compareValueDependency(Info.SC, Inst, Info.id, BrInfo.id)) {
                                is_reduced = true;
                                if (reducedSC.count(BrInfo.id) == 0) {
                                    optimizeCheckAway(Inst);
                                    // dbgs() << "Redundant SC id:" << BrInfo.id<<", SC id:"<<Info.id <<", covered times: "<< BrInfo.count[0]<<".\n";
                                    num_SC_opts -= 1;
                                    total_cost_SC_opts -= BrInfo.count[0];
                                    reducedSC[BrInfo.id] = BrInfo.count[0];
                                }
                            }
                        }
                    }
                } 
                // Calculate reduced asap cost
                if (reducedSC.count(BrInfo.id) == 1) {
                    total_asap_cost_opt -= Cost;
                    count_reduced_SC += 1;
                }
                // Calculate remaining SC and cost
                if (!is_reduced) {
                     if (reducedSC.count(BrInfo.id) == 0) {
                        count_remaining_SC += 1;
                        count_remaining_cost_SC += BrInfo.count[0];
                    }
                }
            }
        }
        fclose(fp_sc);
        dbgs() << "No. of UCs: " << num_UC << ", No. of SCs: " << num_SC << ".\n";
        dbgs() << "SC percent after removing SCs redundant with UCs: " << num_SC_opt * 1.0 / (num_SC + 0.000000001) * 100 << "\%.\n";
        dbgs() << "SC percent after removing SCs redundant with SCs: " << num_SC_opts * 1.0 / (num_SC + 0.000000001) * 100 << "\%.\n";
        dbgs() << "SC cost percent after removing SCs redundant with UCs: " << total_cost_SC_opt * 1.0 / (total_cost_SC + 0.000000001) * 100 << "\%.\n";
        dbgs() << "SC cost percent after removing SCs redundant with SCs: " << total_cost_SC_opts * 1.0 / (total_cost_SC + 0.000000001) * 100 << "\%.\n";
        dbgs() << "Remaining SCs:" << count_remaining_SC << "=" << num_SC_opts << ", remaining cost of SCs:" << count_remaining_cost_SC << "=" << total_cost_SC_opts << ".\n";
        dbgs() << "Removed SCs:" << count_reduced_SC << "=" << num_SC - num_SC_opts << ".\n";
        dbgs() << "Total check cost by ASAP:" << total_asap_cost << ", remaining check cost by ASAP: " << total_asap_cost_opt << ".\n";

        FILE *fpp = fopen(Twine(InputLOG).str().c_str(), "ab");
        fprintf(fpp, "%s %lu %lu %lu %lu %lu %lu %lu %lu\n", filename.c_str(), num_SC, num_SC_opt,num_SC_opts,total_cost_SC,total_cost_SC_opt,total_cost_SC_opts, total_asap_cost, total_asap_cost_opt);

        // Finish reading and storing SC coverage records from InputSCOV
        // Calculate the cost level of 
        double CostLevel1 = 0, CostLevel2 = 0, NumLevel1 = 0, NumLevel2 = 0;
        double b1=0, b2=0, b3=0, b4=0;
        if (total_asap_cost == 0) {
            total_asap_cost = 1;
        }
        if (num_SC == 0) {
            num_SC = 1;
        }
        dbgs() << "Total asap cost: " << total_asap_cost << ", total No. SCs: " << num_SC << ".\n";
        double budget_remove = 0, budget_count = 0;
        // for (std::map<uint64_t, uint64_t>::iterator i=reducedSC.begin(); i!=reducedSC.end(); ++i){
        //     dbgs() << "SC id: " << i->first << ", covered times: " << i->second << ".\n";
        // }
        for (std::map<uint64_t, std::vector<brStat>>::iterator I=CostLevelRange.begin(); I!=CostLevelRange.end(); ++I) {
            for (uint64_t i=0; i<I->second.size(); i++) {

                CostLevel2 += I->first; //  * I->second.size();
                NumLevel2 += 1; // I->second.size();
                I->second[i].CostLevel1 = CostLevel1 / total_asap_cost;
                I->second[i].CostLevel2 = CostLevel2 / total_asap_cost;
                I->second[i].NumLevel1 = NumLevel1 / num_SC;
                I->second[i].NumLevel2 = NumLevel2 / num_SC;
                SC_Stat[I->second[i].SC].CostLevel1 = CostLevel1 / total_asap_cost;
                SC_Stat[I->second[i].SC].CostLevel2 = CostLevel2 / total_asap_cost;
                SC_Stat[I->second[i].SC].NumLevel1 = NumLevel1 / num_SC;
                SC_Stat[I->second[i].SC].NumLevel2 = NumLevel2 / num_SC;
                CostLevel1 = CostLevel2;
                NumLevel1 = NumLevel2;

                if (budget_count / total_asap_cost < 0.05 && (budget_count+I->first) / total_asap_cost > 0.05){
                    dbgs() << "ASAP budget: " << 0.05 << ", budget removed by SR:" << budget_remove / total_asap_cost << ", budget new:" << (budget_count - budget_remove) / total_asap_cost << "\n";
                    b1 = budget_remove / total_asap_cost;
                }
                else if (budget_count / total_asap_cost < 0.1 && (budget_count+I->first) / total_asap_cost > 0.1){
                    dbgs() << "ASAP budget: " << 0.10 << ", budget removed by SR:" << budget_remove / total_asap_cost << ", budget new:" << (budget_count - budget_remove) / total_asap_cost << "\n";
                    b2 = budget_remove / total_asap_cost;
                }
                else if (budget_count / total_asap_cost < 0.15 && (budget_count+I->first) / total_asap_cost > 0.15){
                    dbgs() << "ASAP budget: " << 0.15 << ", budget removed by SR:" << budget_remove / total_asap_cost << ", budget new:" << (budget_count - budget_remove) / total_asap_cost << "\n";
                    b3 = budget_remove / total_asap_cost;
                }
                else if (budget_count / total_asap_cost < 0.3 && (budget_count+I->first) / total_asap_cost > 0.3){
                    dbgs() << "ASAP budget: " << 0.30 << ", budget removed by SR:" << budget_remove / total_asap_cost << ", budget new:" << (budget_count - budget_remove) / total_asap_cost << "\n";
                    b4 = budget_remove / total_asap_cost;
                }
                
                budget_count += I->first;
                
                if (reducedSC.count(SC_Stat[I->second[i].SC].id) == 1){
                    budget_remove += I->first;
                    // dbgs() << SC_Stat[I->second[i].SC].id;
                }
                if (budget_count / total_asap_cost > Threshold){
                    if (reducedSC.count(SC_Stat[I->second[i].SC].id) == 0) {
                        optimizeCheckAway(I->second[i].SC);
                    }
                }
            }
        }
        fprintf(fpp, "%s %f %f %f %f\n", filename.c_str(), b1,b2,b3,b4);
        fclose(fpp);
        dbgs() << "end\n";
    }
    return true;
}


bool SRAnalysisPass::compareValueDependency(Instruction *UC_Inst, Instruction *SC_Inst, uint64_t id1, uint64_t id2) {
    std::set<Instruction*> UC_list;
    std::set<Value*> UC_Slist;
    UC_list.insert(UC_Inst);
    std::set<Instruction*> SC_list;
    std::set<Value*> SC_Slist;
    SC_list.insert(SC_Inst);
    bool flag = true;
    uint64_t count = 0;
    while (!SC_list.empty()) {
        flag = false;
        Instruction *Inst = *SC_list.begin();
        SC_list.erase(Inst);
        StringRef OpName = Inst->getOpcodeName();
        if (OpName == "phi") {
            SC_Slist.insert(Inst);
        }
        else if (BranchInst *BI = dyn_cast<BranchInst>(Inst)) {
            if (Instruction *Op=dyn_cast<Instruction>(Inst->getOperand(0))) {
                SC_list.insert(Op);
            }
        }
        else {
            flag = true;
        }
        if (flag) {
            count = 0;
            for (Use &U: Inst->operands()) {
                if (Instruction *Op = dyn_cast<Instruction>(U.get())) {
                    SC_list.insert(Op);
                }
                else if (Constant *C = dyn_cast<Constant>(U.get())) {
                    count += 1;
                    if (InputLevel == "L0") {
                        SC_Slist.insert(C);
                    }
                    if (InputLevel == "L1" && OpName == "icmp") {
                        SC_Slist.insert(C);
                    }
                }
                else {
                    SC_Slist.insert(U.get());
                }
            }
            if (count == Inst->getNumOperands()) {
                SC_Slist.insert(Inst);
            }
        }
    }

    flag = true;
    while (!UC_list.empty()) {
        flag = false;
        Instruction *Inst = *UC_list.begin();
        UC_list.erase(Inst);
        StringRef OpName = Inst->getOpcodeName();
        if (OpName == "phi") {
            UC_Slist.insert(Inst);
        }
        else if (BranchInst *BI = dyn_cast<BranchInst>(Inst)) {
            if (Instruction *Op=dyn_cast<Instruction>(Inst->getOperand(0))) {
                UC_list.insert(Op);
            }
        }
        else {
            flag = true;
        }

        if (flag) {
            count = 0;
            for (Use &U: Inst->operands()) {
                if (Instruction *Op = dyn_cast<Instruction>(U.get())) {
                    UC_list.insert(Op);
                }
                else if (Constant *C = dyn_cast<Constant>(U.get())) {
                    count += 1;
                    if (InputLevel == "L0") {
                        UC_Slist.insert(C);
                    }
                    if (InputLevel == "L1" && OpName == "icmp") {
                        UC_Slist.insert(C);
                    }
                }
                else {
                    UC_Slist.insert(U.get());
                }
            }
            if (count == Inst->getNumOperands()) {
                UC_Slist.insert(Inst);
            }
        }
    }

    flag = false;
    if (SC_Slist.size() == UC_Slist.size()) {
        while (!SC_Slist.empty()) {
            flag = false;
            Value *SC_Inst = *SC_Slist.begin();
            SC_Slist.erase(SC_Inst);
            if (UC_Slist.count(SC_Inst) > 0) {
                flag = true;
            }
            if (!flag) {
                break;
            }
        }
    }

    return flag;
}

// Tries to remove a sanity check; returns true if it worked.
void SRAnalysisPass::optimizeCheckAway(Instruction *Inst) {
    BranchInst *BI = cast<BranchInst>(Inst);
    assert(BI->isConditional() && "Sanity check must be conditional branch.");
    
    unsigned int RegularBranch = getRegularBranch(BI, SCI);
    
    if (RegularBranch == 0) {
        BI->setCondition(ConstantInt::getTrue(Inst->getContext()));
    } else if (RegularBranch == 1) {
        BI->setCondition(ConstantInt::getFalse(Inst->getContext()));
    } else {
        dbgs() << "Warning: Sanity check with no regular branch found.\n";
        dbgs() << "The sanity check has been kept intact.\n";
    }
}


void SRAnalysisPass::getAnalysisUsage(AnalysisUsage& AU) const {
    AU.addRequired<TargetTransformInfoWrapperPass>();
    AU.addRequired<SCIPass>();
    AU.setPreservesAll();
}


char SRAnalysisPass::ID = 0;
static RegisterPass<SRAnalysisPass> X("sr-analysis",
        "Finds costs of sanity checks", false, false);



