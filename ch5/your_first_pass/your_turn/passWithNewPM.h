#ifndef YOURTURN_PASSWITHNEWPM_H
#define YOURTURN_PASSWITHNEWPM_H
#include "llvm/IR/PassManager.h" // For PassInfoMixin.

namespace llvm {
class Function;
};

class YourTurnConstantPropagationNewPass : public llvm::PassInfoMixin<YourTurnConstantPropagationNewPass> {
public:
    friend PassInfoMixin<YourTurnConstantPropagationNewPass>;
    llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM);
};

/* TODO: Change this into 1 when you're ready to try */
#define YOUR_TURN_IS_READY 1
#endif
