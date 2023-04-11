//===-- SAYACRegisterInfo.cpp - SAYAC Register Information ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the SAYAC implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "SAYACRegisterInfo.h"
#include "SAYAC.h"
//#include "SAYACMachineFunctionInfo.h"
#include "SAYACSubtarget.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h" 

using namespace llvm;

#define GET_REGINFO_TARGET_DESC
#include "SAYACGenRegisterInfo.inc"

SAYACRegisterInfo::SAYACRegisterInfo() : SAYACGenRegisterInfo(SAYAC::R1) {}

const MCPhysReg *
SAYACRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_SAYAC_SaveList;
}

BitVector SAYACRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());

  // R2 is the stack pointer.
  // Reserved.set(SAYAC::R2);

  // R0 is wired to zero
  // Reserved.set(SAYAC::R0);

  // R15 stores flag bits
  // Reserved.set(SAYAC::R15);

  return Reserved;
}

void SAYACRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator MI,
                                           int SPAdj, unsigned FIOperandNum,
                                           RegScavenger *RS) const {}

Register SAYACRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return SAYAC::R3;
}
