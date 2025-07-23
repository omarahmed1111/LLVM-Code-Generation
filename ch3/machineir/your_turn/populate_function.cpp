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

// The goal of this function is to build a MachineFunction that
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
// The proposed ABI is:
// - 32-bit arguments are passed through registers: w0, w1
// - 32-bit returned values are passed through registers: w0, w1
// w0 and w1 are given as argument of this Function.
//
// The local variable named var is expected to live on the stack.
MachineFunction *populateMachineIR(MachineModuleInfo &MMI, Function &Foo,
                                   Register W0, Register W1) {
  MachineFunction &MF = MMI.getOrCreateMachineFunction(Foo);

  // The type for bool.
  LLT I1 = LLT::scalar(1);
  // The type of var.
  LLT I32 = LLT::scalar(32);

  // To use to create load and store for var.
  MachinePointerInfo PtrInfo;
  Align VarStackAlign(4);

  // The type for the address of var.
  LLT VarAddrLLT = LLT::pointer(/*AddressSpace=*/0, /*SizeInBits=*/64);

  // The stack slot for var.
  int FrameIndex = MF.getFrameInfo().CreateStackObject(32, VarStackAlign,
                                                       /*IsSpillSlot=*/false);

  // TODO: Populate MF.
  auto* bb0 = MF.CreateMachineBasicBlock();
  auto* bb1 = MF.CreateMachineBasicBlock();
  auto* bb2 = MF.CreateMachineBasicBlock();
  
  MF.push_back(bb0);
  MF.push_back(bb1);
  MF.push_back(bb2);

  // successors
  bb0->addSuccessor(bb1);
  bb0->addSuccessor(bb2);
  bb1->addSuccessor(bb2);

  // bb0 instructions
  MachineIRBuilder bb0MB(*bb0, bb0->end());
  Register _0 = bb0MB.buildCopy(I32, W0).getReg(0);
  Register _1 = bb0MB.buildCopy(I32, W1).getReg(0);
  Register _2 = bb0MB.buildFrameIndex(VarAddrLLT, FrameIndex).getReg(0);
  Register _3 = bb0MB.buildAdd(I32, _0, _1).getReg(0);
  bb0MB.buildStore(_3, _2, PtrInfo, VarStackAlign);
  Register _4 = bb0MB.buildConstant(I32, 255).getReg(0);
  Register _5 = bb0MB.buildLoad(I32, _2, PtrInfo, VarStackAlign).getReg(0);
  Register _6 = bb0MB.buildICmp(CmpInst::ICMP_EQ, I1, _5, _4).getReg(0);
  bb0MB.buildBrCond(_6, *bb1);
  bb0MB.buildBr(*bb2);
  
  // bb1 instructions
  MachineIRBuilder bb1MB(*bb1, bb1->end());
  Register _7 = bb1MB.buildLoad(I32, _2, PtrInfo, VarStackAlign).getReg(0);
  bb1MB.buildCopy(W0, _7);
  bb1MB.buildInstr(TargetOpcode::INLINEASM, {}, {}).addExternalSymbol("bl @bar").addImm(0).addReg(W0, RegState::Implicit);
  bb1MB.buildInstr(TargetOpcode::INLINEASM, {}, {}).addExternalSymbol("bl @baz").addImm(0).addReg(W0, RegState::Implicit | RegState::Define);
  Register _8 = bb1MB.buildCopy(I32, W0).getReg(0);
  bb1MB.buildStore(_8, _2, PtrInfo, VarStackAlign);

  // bb2 instructions
  MachineIRBuilder bb2MB(*bb2, bb2->end());
  Register _9 = bb2MB.buildLoad(I32, _2, PtrInfo, VarStackAlign).getReg(0);
  bb2MB.buildCopy(W0, _9);
  bb2MB.buildInstr(TargetOpcode::INLINEASM, {}, {}).addExternalSymbol("bl @bar").addImm(0).addReg(W0, RegState::Implicit);
  bb2MB.buildInstr(TargetOpcode::INLINEASM, {}, {}).addExternalSymbol("ret").addImm(0);

  return &MF;
}
