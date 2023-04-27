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

static SmallVector<CalleeSavedInfo, 8>
getNonLibcallCSI(const std::vector<CalleeSavedInfo> &CSI) {
  SmallVector<CalleeSavedInfo, 8> NonLibcallCSI;

  for (auto &CS : CSI)
    if (CS.getFrameIdx() >= 0)
      NonLibcallCSI.push_back(CS);

  return NonLibcallCSI;
}

SAYACFrameLowering::SAYACFrameLowering(const SAYACSubtarget &STI)
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown,
                          Align(8) /*16bit architecture*/, 0, Align(8),
                          false /* StackRealignable */),
      RegSpillOffsets(0),
      STI(STI) {}

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
  MachineFrameInfo &MFI = MF.getFrameInfo();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc dl = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();

  determineFrameLayout(MF);

  uint64_t StackSize = MFI.getStackSize();
  // dbgs() << StackSize << '\n';
  if (!StackSize) {
    return;
  }

  // Adjust the stack pointer.
  unsigned StackReg = SAYAC::R2; // Stack Pointer
  unsigned OffsetReg = materializeOffset(MF, MBB, MBBI, -(unsigned)StackSize);
  if (OffsetReg) {
    BuildMI(MBB, MBBI, dl, TII.get(SAYAC::ADDrr), StackReg)
        .addReg(StackReg)
        .addReg(OffsetReg)
        .setMIFlag(MachineInstr::FrameSetup);
  } else {
    BuildMI(MBB, MBBI, dl, TII.get(SAYAC::ADDri), StackReg)
        .addReg(StackReg)
        .addImm(-StackSize)
        .setMIFlag(MachineInstr::FrameSetup);
  }

  const auto &CSI = MFI.getCalleeSavedInfo();

  // The frame pointer is callee-saved, and code has been generated for us to
  // save it to the stack. We need to skip over the storing of callee-saved
  // registers as the frame pointer must be modified after it has been saved
  // to the stack, not before.
  // FIXME: assumes exactly one instruction is used to save each callee-saved
  // register.
  std::advance(MBBI, getNonLibcallCSI(CSI).size());

  // Make FP point to top of frame.
  
  unsigned FramePointerReg = SAYAC::R3;
  Register ScratchReg = MRI.createVirtualRegister(&SAYAC::GPRRegClass);

  BuildMI(MBB, MBBI, dl, TII.get(SAYAC::MSI), ScratchReg)
        .addImm(StackSize)
        .setMIFlag(MachineInstr::FrameSetup);

  BuildMI(MBB, MBBI, dl, TII.get(SAYAC::ADDrr), FramePointerReg)
        .addReg(StackReg)
        .addReg(ScratchReg)
        .setMIFlag(MachineInstr::FrameSetup);
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
  unsigned StackReg = SAYAC::R2;   // Stack Pointer

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


bool SAYACFrameLowering::hasFP(const MachineFunction &MF) const {
  return true;
  // const TargetRegisterInfo *RegInfo = MF.getSubtarget().getRegisterInfo();

  // const MachineFrameInfo &MFI = MF.getFrameInfo();
  // return MF.getTarget().Options.DisableFramePointerElim(MF) ||
  //        RegInfo->needsStackRealignment(MF) || MFI.hasVarSizedObjects() ||
  //        MFI.isFrameAddressTaken();
}

//Determines the size of the frame and maximum call frame size.
void SAYACFrameLowering::determineFrameLayout(MachineFunction &MF) const {
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const SAYACRegisterInfo *RI = STI.getRegisterInfo();

  // Get the number of bytes to allocate from the FrameInfo.
  uint64_t FrameSize = MFI.getStackSize();

  // Get the alignment.
  unsigned StackAlign = getStackAlignment();
  if (RI->needsStackRealignment(MF)) {
    unsigned MaxStackAlign = std::max(StackAlign, MFI.getMaxAlignment());
    FrameSize += (MaxStackAlign - StackAlign);
    StackAlign = MaxStackAlign;
  }

  // Set Max Call Frame Size
  uint64_t MaxCallSize = alignTo(MFI.getMaxCallFrameSize(), StackAlign);
  MFI.setMaxCallFrameSize(MaxCallSize);

  // Make sure the frame is aligned.
  FrameSize = alignTo(FrameSize, StackAlign);

  // Update frame info.
  MFI.setStackSize(FrameSize);
}

// This function eliminates ADJCALLSTACKDOWN, ADJCALLSTACKUP pseudo
// instructions
MachineBasicBlock::iterator SAYACFrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator I) const {
  return MBB.erase(I);
}


void SAYACFrameLowering::determineCalleeSaves(MachineFunction &MF,
                                              BitVector &SavedRegs,
                                              RegScavenger *RS) const {
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
  // Unconditionally spill RA and FP only if the function uses a frame
  // pointer.
  if (hasFP(MF)) {
    SavedRegs.set(SAYAC::R1);
    SavedRegs.set(SAYAC::R3);
  }
  // // Mark BP as used if function has dedicated base pointer.
  // if (hasBP(MF))
  //   SavedRegs.set(RISCVABI::getBPReg());
}

bool SAYACFrameLowering::spillCalleeSavedRegisters(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
    ArrayRef<CalleeSavedInfo> CSI, const TargetRegisterInfo *TRI) const {
  if (CSI.empty())
    return true;

  MachineFunction *MF = MBB.getParent();
  const TargetInstrInfo &TII = *MF->getSubtarget().getInstrInfo();
  DebugLoc DL;
  if (MI != MBB.end() && !MI->isDebugInstr())
    DL = MI->getDebugLoc();

  // Manually spill values not spilled by libcall.
  const auto &NonLibcallCSI = getNonLibcallCSI(CSI);
  for (auto &CS : NonLibcallCSI) {
    // Insert the spill to the stack frame.
    Register Reg = CS.getReg();
    const TargetRegisterClass *RC = TRI->getMinimalPhysRegClass(Reg);
    TII.storeRegToStackSlot(MBB, MI, Reg, true, CS.getFrameIdx(), RC, TRI);
  }

  return true;
}

bool SAYACFrameLowering::restoreCalleeSavedRegisters(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
    MutableArrayRef<CalleeSavedInfo> CSI, const TargetRegisterInfo *TRI) const {
  if (CSI.empty())
    return true;

  MachineFunction *MF = MBB.getParent();
  const TargetInstrInfo &TII = *MF->getSubtarget().getInstrInfo();
  DebugLoc DL;
  if (MI != MBB.end() && !MI->isDebugInstr())
    DL = MI->getDebugLoc();

  // Manually restore values not restored by libcall. Insert in reverse order.
  // loadRegFromStackSlot can insert multiple instructions.
  const auto &NonLibcallCSI = getNonLibcallCSI(CSI);
  for (auto &CS : reverse(NonLibcallCSI)) {
    Register Reg = CS.getReg();
    const TargetRegisterClass *RC = TRI->getMinimalPhysRegClass(Reg);
    TII.loadRegFromStackSlot(MBB, MI, Reg, CS.getFrameIdx(), RC, TRI);
    assert(MI != MBB.begin() && "loadRegFromStackSlot didn't insert any code!");
  }

  return true;
}
