#include "llvm/IR/Function.h"
#include "llvm/Pass.h"          // For FunctionPass & INITIALIZE_PASS.
#include "llvm/Support/Debug.h" // For errs().

using namespace llvm;

extern bool solutionConstantPropagation(llvm::Function &);

// The implementation of this function is generated at the end of this file. See
// INITIALIZE_PASS.
namespace llvm {
void initializeYourTurnConstantPropagationPass(PassRegistry &);
};

namespace {
class YourTurnConstantPropagation : public FunctionPass {
public:
  static char ID;

  YourTurnConstantPropagation() : FunctionPass(ID) {
    llvm::initializeYourTurnConstantPropagationPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override { return solutionConstantPropagation(F); }
};

char YourTurnConstantPropagation::ID = 0;

} // End anonymous namespace.

INITIALIZE_PASS(YourTurnConstantPropagation, "your-turn-constant-propagation", "My wonderful pass", false, false);
// TODO: Remove and add proper implementation
// void llvm::initializeYourTurnConstantPropagationPass(PassRegistry &) {}

Pass *createYourTurnPassForLegacyPM() {
  return new YourTurnConstantPropagation();
}
