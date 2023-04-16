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
#include "llvm/Target/TargetMachine.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;


// Materialize an offset for a ADD/SUB stack operation.
// Return zero if the offset fits into the instruction as an immediate,
// or the number of the register where the offset is materialized.
static unsigned materializeOffset(MachineFunction &MF, MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator MBBI,
                                  unsigned Offset) {
    return 0;
//   const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
//   DebugLoc dl = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
//   const uint64_t MaxSubImm = 0xfff;
//   if (Offset <= MaxSubImm) {
//     // The stack offset fits in the ADD/SUB instruction.
//     return 0;
//   } else {
//     // The stack offset does not fit in the ADD/SUB instruction.
//     // Materialize the offset using MOVLO/MOVHI.
//     unsigned OffsetReg = LEG::R4;
//     unsigned OffsetLo = (unsigned)(Offset & 0xffff);
//     unsigned OffsetHi = (unsigned)((Offset & 0xffff0000) >> 16);
//     BuildMI(MBB, MBBI, dl, TII.get(LEG::MOVLOi16), OffsetReg)
//         .addImm(OffsetLo)
//         .setMIFlag(MachineInstr::FrameSetup);
//     if (OffsetHi) {
//       BuildMI(MBB, MBBI, dl, TII.get(LEG::MOVHIi16), OffsetReg)
//           .addReg(OffsetReg)
//           .addImm(OffsetHi)
//           .setMIFlag(MachineInstr::FrameSetup);
//     }
//     return OffsetReg;
//   }
}

inline uint64_t RoundUpToAlignment(uint64_t Value, uint64_t Align,
                                   uint64_t Skew = 0) {
  Skew %= Align;
  return (Value + Align - 1 - Skew) / Align * Align + Skew;
}

SAYACFrameLowering::SAYACFrameLowering()
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(16), 0,
                          Align(16), false /* StackRealignable */),
      RegSpillOffsets(0) {}

uint64_t SAYACFrameLowering::computeStackSize(MachineFunction &MF) const {
  MachineFrameInfo MFI = MF.getFrameInfo();
  uint64_t StackSize = MFI.getStackSize();
  unsigned StackAlign = getStackAlignment();
  if (StackAlign > 0) {
    StackSize = RoundUpToAlignment(StackSize, StackAlign);
  }
  return StackSize;
}



void SAYACFrameLowering::emitPrologue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  // Compute the stack size, to determine if we need a prologue at all.
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc dl = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
  uint64_t StackSize = computeStackSize(MF);
  dbgs() << StackSize << '\n';
  if (!StackSize) {
    return;
  }

  // Adjust the stack pointer.
  unsigned StackReg = SAYAC::R13; // Stack Pointer
  unsigned OffsetReg = materializeOffset(MF, MBB, MBBI, (unsigned)StackSize);
  if (OffsetReg) {
    BuildMI(MBB, MBBI, dl, TII.get(SAYAC::SUBrr), StackReg)
        .addReg(StackReg)
        .addReg(OffsetReg)
        .setMIFlag(MachineInstr::FrameSetup);
  } else {
    BuildMI(MBB, MBBI, dl, TII.get(SAYAC::SUBri), StackReg)
        .addReg(StackReg)
        .addImm(StackSize)
        .setMIFlag(MachineInstr::FrameSetup);
  }
}


void SAYACFrameLowering::emitEpilogue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  // Compute the stack size, to determine if we need an epilogue at all.
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  DebugLoc dl = MBBI->getDebugLoc();
  uint64_t StackSize = computeStackSize(MF);
  if (!StackSize) {
    return;
  }

  // Restore the stack pointer to what it was at the beginning of the function.
  unsigned StackReg = SAYAC::R13;   // Stack Pointer
  unsigned OffsetReg = materializeOffset(MF, MBB, MBBI, (unsigned)StackSize);
  if (OffsetReg) {
    BuildMI(MBB, MBBI, dl, TII.get(SAYAC::ADDrr), StackReg)
        .addReg(StackReg)
        .addReg(OffsetReg)
        .setMIFlag(MachineInstr::FrameSetup);
  } else {
    BuildMI(MBB, MBBI, dl, TII.get(SAYAC::ADDri), StackReg)
        .addReg(StackReg)
        .addImm(StackSize)
        .setMIFlag(MachineInstr::FrameSetup);
  }
}


bool SAYACFrameLowering::hasFP(const MachineFunction &MF) const { return false; }