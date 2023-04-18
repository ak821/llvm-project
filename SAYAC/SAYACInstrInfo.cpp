//===-- SAYACInstrInfo.cpp - SAYAC instruction information ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the SAYAC implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "SAYACInstrInfo.h"
#include "SAYAC.h"
#include "MCTargetDesc/SAYACMCTargetDesc.h"
//#include "SAYACInstrBuilder.h"
#include "SAYACSubtarget.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/LiveInterval.h"
#include "llvm/CodeGen/LiveIntervals.h"
#include "llvm/CodeGen/LiveVariables.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SlotIndexes.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/Support/BranchProbability.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Target/TargetMachine.h"
#include <cassert>
#include <cstdint>
#include <iterator>

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#define GET_INSTRMAP_INFO
#include "SAYACGenInstrInfo.inc"

#define DEBUG_TYPE "SAYAC-ii"

// Pin the vtable to this file.
void SAYACInstrInfo::anchor() {}

SAYACInstrInfo::SAYACInstrInfo(SAYACSubtarget &STI)
    : SAYACGenInstrInfo(), RI(), STI(STI) {}


void SAYACInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator I,
                                 const DebugLoc &DL, MCRegister DestReg,
                                 MCRegister SrcReg, bool KillSrc) const {
  BuildMI(MBB, I, I->getDebugLoc(), get(SAYAC::ADDri), DestReg)
      .addReg(SrcReg, getKillRegState(KillSrc));
}

void SAYACInstrInfo::movImm(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MBBI,
                            const DebugLoc &DL, Register DstReg, uint16_t Val,
                            MachineInstr::MIFlag Flag) const {
  MachineFunction *MF = MBB.getParent();
  MachineRegisterInfo &MRI = MF->getRegInfo();

  int16_t Hi8 = ((Val + 0x80) >> 8) & 0xFF;
  int16_t Lo8 = SignExtend64<8>(Val);

  if(!Hi8) {
    BuildMI(MBB, MBBI, DL, get(SAYAC::MSI), DstReg)
        .addImm(Lo8)
        .setMIFlag(Flag);
  } else {
    BuildMI(MBB, MBBI, DL, get(SAYAC::MSI), DstReg)
        .addImm(Lo8)
        .setMIFlag(Flag);
    BuildMI(MBB, MBBI, DL, get(SAYAC::MHI), DstReg)
        .addImm(Hi8)
        .setMIFlag(Flag);
  }
}