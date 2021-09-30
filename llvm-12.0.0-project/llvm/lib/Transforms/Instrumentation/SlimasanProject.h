#ifndef SLIMASAN_H_
#define SLIMASAN_H_

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopInfoImpl.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/MC/MCSectionMachO.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/SwapByteOrder.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/ASanStackFrameLayout.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include <algorithm>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <system_error>
#include "llvm/ADT/None.h"
#include "llvm/ADT/Optional.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Casting.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

using namespace llvm;

class Dumper {
  public:
    Dumper() {}
    ~Dumper() {}

    // LLVM value
    void valueName(Value *val);
    void typedValue(Value *val);
    void ctypeValue(Value *val);
};

class Helper {
  public:
    // LLVM value
    static std::string getValueName(Value *v);
    static std::string getValueType(Value *v);
    static std::string getValueRepr(Value *v);
    static std::string getCtypeRepr(Type *t);

    // string conversion
    static void convertDotInName(std::string &name);
};

enum addrType {IBIO, VBIO, IBVO, VBVO, UNKNOWN};

enum SCEVType {SEIncrease, SEDecrease, SEConstant, SELoopInvariant, SEUnknown};

bool btraceInLoop(Value *v, std::vector<Value *> &backs, Loop *L);

bool btraceIndex(Value *v, std::vector<Value *> &backs, Instruction *DefineInst);

enum addrType checkAddrType(Value *addr, std::vector<Value *> backs, std::vector<Value *> &processedAddr, ScalarEvolution *SE, Loop *L);

ICmpInst *getLatchCmpInst(const Loop &L);

Value *findFinalIVValue(Value *op0, Value *op1);

#endif /* SLIMASAN_H_ */