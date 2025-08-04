#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/MachineFrameInfo.h" // For CreateStackObject.
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineMemOperand.h" // For MachinePointerInfo.
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/CodeGen/TargetOpcodes.h"     // For INLINEASM.
#include "llvm/CodeGenTypes/LowLevelType.h" // For LLT.
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h" // For ICMP_EQ.

using namespace llvm;
// Takes \p Foo and apply a simple constant propagation optimization.
// \returns true if \p Foo was modified (i.e., something had been constant
// propagated), false otherwise.
bool myConstantPropagation(Function &Foo) {
  SmallVector<Instruction*> ToDelete(8);

  for (auto& BB : Foo) {
    for (auto& I : BB) {
      if (auto* shlI = dyn_cast<ShlOperator>(&I)) {
        Value* OperandOne = shlI->getOperand(0);
        Value* OperandTwo = shlI->getOperand(1);
        auto* ConstOne = dyn_cast<ConstantInt>(OperandOne);
        auto* ConstTwo = dyn_cast<ConstantInt>(OperandTwo);

        if (ConstOne && ConstTwo) {
          APInt NewConst = ConstOne->getValue() << ConstTwo->getValue();
          shlI->replaceAllUsesWith(ConstantInt::get(Foo.getContext(), NewConst));
        }
        ToDelete.push_back(&I);
      } else if (auto* binary = dyn_cast<BinaryOperator>(&I)) {
        if (binary->getOpcode() == Instruction::UDiv) {
          Value* OperandOne = binary->getOperand(0);
          Value* OperandTwo = binary->getOperand(1);
          auto* ConstOne = dyn_cast<ConstantInt>(OperandOne);
          auto* ConstTwo = dyn_cast<ConstantInt>(OperandTwo);

          if (ConstOne && ConstTwo) {
            APInt NewConst = ConstOne->getValue().udiv(ConstTwo->getValue());
            binary->replaceAllUsesWith(ConstantInt::get(Foo.getContext(), NewConst));
          }
          ToDelete.push_back(&I);
        } else if (binary->getOpcode() == Instruction::Or) {
          Value* OperandOne = binary->getOperand(0);
          Value* OperandTwo = binary->getOperand(1);
          auto* ConstOne = dyn_cast<ConstantInt>(OperandOne);
          auto* ConstTwo = dyn_cast<ConstantInt>(OperandTwo);

          if (ConstOne && ConstTwo) {
            APInt NewConst = ConstOne->getValue() | ConstTwo->getValue();
            binary->replaceAllUsesWith(ConstantInt::get(Foo.getContext(), NewConst));
          }
          ToDelete.push_back(&I);
        }
      }
    }
  }

  // for (auto* I : ToDelete) {
  //   I->removeFromParent();
  // }
  return true;
}
