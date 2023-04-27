//===-- SAYACFrameLowering.h - Frame lowering for SAYAC -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SAYAC_SAYACFRAMELOWERING_H
#define LLVM_LIB_TARGET_SAYAC_SAYACFRAMELOWERING_H

#include "llvm/ADT/IndexedMap.h"
#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {
class SAYACTargetMachine;
class SAYACSubtarget;

class SAYACFrameLowering : public TargetFrameLowering {
  IndexedMap<unsigned> RegSpillOffsets;
  uint64_t computeStackSize(MachineFunction &MF) const;
  void determineFrameLayout(MachineFunction &MF) const;

protected:
  const SAYACSubtarget &STI;

public:
  SAYACFrameLowering(const SAYACSubtarget &STI);

  // Override TargetFrameLowering.
  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  bool hasFP(const MachineFunction &MF) const override;
  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I) const override;
  
  void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs,
                            RegScavenger *RS) const override;

  bool spillCalleeSavedRegisters(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MI,
                                 ArrayRef<CalleeSavedInfo> CSI,
                                 const TargetRegisterInfo *TRI) const override;
  bool
  restoreCalleeSavedRegisters(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator MI,
                              MutableArrayRef<CalleeSavedInfo> CSI,
                              const TargetRegisterInfo *TRI) const override;
  
};
} // end namespace llvm

#endif