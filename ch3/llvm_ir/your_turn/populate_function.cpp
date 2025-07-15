#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"    // For ConstantInt.
#include "llvm/IR/DerivedTypes.h" // For PointerType, FunctionType.
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Debug.h" // For errs().

#include <memory> // For unique_ptr

using namespace llvm;

// The goal of this function is to build a Module that
// represents the lowering of the following foo, a C function:
// extern int baz();
// extern void bar(int);
// void foo(int a, int b) {
//   int var = a + b;
//   if (var == 0xFF) {
//     bar(var);
//     var = baz();
//   }
//   bar(var);
// }
//
// The IR for this snippet (at O0) is:
// define void @foo(i32 %arg, i32 %arg1) {
// bb:
//   %i = alloca i32
//   %i2 = alloca i32
//   %i3 = alloca i32
//   store i32 %arg, ptr %i
//   store i32 %arg1, ptr %i2
//   %i4 = load i32, ptr %i
//   %i5 = load i32, ptr %i2
//   %i6 = add i32 %i4, %i5
//   store i32 %i6, ptr %i3
//   %i7 = load i32, ptr %i3
//   %i8 = icmp eq i32 %i7, 255
//   br i1 %i8, label %bb9, label %bb12
//
// bb9:
//   %i10 = load i32, ptr %i3
//   call void @bar(i32 %i10)
//   %i11 = call i32 @baz()
//   store i32 %i11, ptr %i3
//   br label %bb12
//
// bb12:
//   %i13 = load i32, ptr %i3
//   call void @bar(i32 %i13)
//   ret void
// }
//
// declare void @bar(i32)
// declare i32 @baz(...)
std::unique_ptr<Module> myBuildModule(LLVMContext &Ctxt) { 
    std::unique_ptr<Module> ModulePtr = std::make_unique<Module>("MyModule", Ctxt);

    FunctionType* bazFnType = FunctionType::get(Type::getInt32Ty(Ctxt), {}, false);
    FunctionType* barFnType = FunctionType::get(Type::getVoidTy(Ctxt), {Type::getInt32Ty(Ctxt)}, false);
    ModulePtr->getOrInsertFunction("baz", bazFnType);
    ModulePtr->getOrInsertFunction("bar", barFnType);

    FunctionType* fooFnType = FunctionType::get(Type::getVoidTy(Ctxt), {Type::getInt32Ty(Ctxt), Type::getInt32Ty(Ctxt)}, false);
    Function *fooFn = Function::Create(fooFnType, GlobalValue::ExternalLinkage, 0, "foo", &*ModulePtr);

    // Basic blocks
    BasicBlock* bb = BasicBlock::Create(Ctxt, "bb", fooFn);
    BasicBlock* bb9 = BasicBlock::Create(Ctxt, "bb9", fooFn);
    BasicBlock* bb12 = BasicBlock::Create(Ctxt, "bb12", fooFn);

    // IR Builders
    IRBuilder bbB(bb);
    IRBuilder bb9B(bb9);
    IRBuilder bb12B(bb12);
    
    // bb instructions
    auto *alloca1 = bbB.CreateAlloca(Type::getInt32Ty(Ctxt));
    auto *alloca2 = bbB.CreateAlloca(Type::getInt32Ty(Ctxt));
    auto *alloca3 = bbB.CreateAlloca(Type::getInt32Ty(Ctxt));
    bbB.CreateStore(fooFn->getArg(0), alloca1);
    bbB.CreateStore(fooFn->getArg(1), alloca2);
    auto* load1 = bbB.CreateLoad(Type::getInt32Ty(Ctxt), alloca1);
    auto* load2 = bbB.CreateLoad(Type::getInt32Ty(Ctxt), alloca2);
    auto* add = bbB.CreateAdd(load1, load2);
    bbB.CreateStore(add, alloca3);
    auto* load3 = bbB.CreateLoad(Type::getInt32Ty(Ctxt), alloca3);
    auto* icmp = bbB.CreateICmp(CmpInst::ICMP_EQ, load3, ConstantInt::get(Type::getInt32Ty(Ctxt), 255));
    bbB.CreateCondBr(icmp, bb9, bb12);

    // bb9 instructions
    auto* load4 = bb9B.CreateLoad(Type::getInt32Ty(Ctxt), alloca3);
    bb9B.CreateCall(barFnType, ModulePtr->getNamedValue("bar"), {load4});
    auto* call = bb9B.CreateCall(bazFnType, ModulePtr->getNamedValue("baz"), {});
    bb9B.CreateStore(call, alloca3);
    bb9B.CreateBr(bb12);

    // bb12 instructions
    auto* load5 = bb12B.CreateLoad(Type::getInt32Ty(Ctxt), alloca3);
    bb12B.CreateCall(barFnType, ModulePtr->getNamedValue("bar"), {load5});
    ReturnInst::Create(Ctxt, bb12);

    return ModulePtr;
}
