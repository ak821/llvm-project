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

public:
  SAYACFrameLowering();

  // Override TargetFrameLowering.
  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  bool hasFP(const MachineFunction &MF) const override;
};
} // end namespace llvm

#endif