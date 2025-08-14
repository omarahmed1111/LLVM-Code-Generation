#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h" // For the new PassManager.
#include "llvm/Transforms/Utils/Mem2Reg.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Passes/StandardInstrumentations.h"

using namespace llvm;

void runYourTurnPassPipelineForNewPM(Module &MyModule) {
    FunctionAnalysisManager FAM;
    ModuleAnalysisManager MAM;

    PassInstrumentationCallbacks PIC;
    PrintPassOptions PrintPassOpts;
    PrintPassOpts.Verbose = true;
    PrintPassOpts.SkipAnalyses = false;
    PrintPassOpts.Indent = true;

    StandardInstrumentations SI(MyModule.getContext(), true, false, PrintPassOpts);
    SI.registerCallbacks(PIC, &MAM);
    
    MAM.registerPass([&]{ return PassInstrumentationAnalysis(&PIC); });
    FAM.registerPass([&] { return PassInstrumentationAnalysis(&PIC); });

    MAM.registerPass([&] { return FunctionAnalysisManagerModuleProxy(FAM); });
    FAM.registerPass([&] { return ModuleAnalysisManagerFunctionProxy(MAM); });
    
    PassBuilder PB;
    PB.registerFunctionAnalyses(FAM);
    PB.registerModuleAnalyses(MAM);

    ModulePassManager MPM;

    MPM.addPass(createModuleToFunctionPassAdaptor(
          PromotePass()));
    MPM.addPass(createModuleToFunctionPassAdaptor(
          InstCombinePass()));

    MPM.addPass(AlwaysInlinerPass());

    MPM.run(MyModule, MAM);
}
