//===-- SAYACFrameLowering.cpp - Frame lowering for SAYAC -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SAYACFrameLowering.h"
//#include "SAYACCallingConv.h"
//#include "SAYACInstrBuilder.h"
//#include "SAYACInstrInfo.h"
//#include "SAYACMachineFunctionInfo.h"
#include "SAYACRegisterInfo.h"
#include "SAYACSubtarget.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/IR/Function.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

SAYACFrameLowering::SAYACFrameLowering()
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(8), 0,
                          Align(8), false /* StackRealignable */),
      RegSpillOffsets(0) {}

void SAYACFrameLowering::emitPrologue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {}

void SAYACFrameLowering::emitEpilogue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {}

bool SAYACFrameLowering::hasFP(const MachineFunction &MF) const { return false; }