#include "llvm/IR/Function.h"
#include "llvm/Support/Debug.h" // For errs().

#include "passWithNewPM.h"

using namespace llvm;

extern bool solutionConstantPropagation(llvm::Function &);

// TODO: Fill in the blanks.

PreservedAnalyses YourTurnConstantPropagationNewPass::run(Function &F, FunctionAnalysisManager &FAM) {
    PreservedAnalyses PA;

    solutionConstantPropagation(F);
    return PA;
}
