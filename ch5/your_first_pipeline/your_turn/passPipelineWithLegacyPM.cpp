#include "llvm/IR/LegacyPassManager.h" // For legacy::PassManager.
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"

using namespace llvm;

void runYourTurnPassPipelineForLegacyPM(Module &MyModule) {
    legacy::PassManager LegacyPM;

    LegacyPM.add(createPromoteMemoryToRegisterPass());
    LegacyPM.add(createInstructionCombiningPass());
    LegacyPM.add(createAlwaysInlinerLegacyPass());

    LegacyPM.run(MyModule);
}
