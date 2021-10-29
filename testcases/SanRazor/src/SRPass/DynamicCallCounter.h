// This file is used to insert counters before checks, in order to get their dynamic patterns.

#include "llvm/ADT/DenseMap.h"
#include "llvm/Pass.h"

namespace llvm {
  class Instruction;
  class Value;
  class AnalysisUsage;
}

struct DynamicCallCounter : public llvm::ModulePass {
  static char ID;
  uint64_t num_SC = 0, num_UC = 0;
  llvm::DenseMap<llvm::Instruction*, uint64_t> ids_SC;
  llvm::DenseMap<llvm::Instruction*, uint64_t> ids_UC;

  DynamicCallCounter() : llvm::ModulePass(ID) {}

  virtual bool runOnModule(llvm::Module& m);
  virtual void getAnalysisUsage(llvm::AnalysisUsage& AU) const;

  void handleCalledBranch(llvm::Module& m, llvm::Instruction& f, llvm::Value* counter, std::string str, std::string filename);

};

