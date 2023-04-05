//===-- SAYACInstrInfo.h - SAYAC instruction information ----------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_SAYAC_SAYACINSTRINFO_H
#define LLVM_LIB_TARGET_SAYAC_SAYACINSTRINFO_H

#include "SAYAC.h"
#include "SAYACRegisterInfo.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include <cstdint>

#define GET_INSTRINFO_HEADER
#include "SAYACGenInstrInfo.inc"

namespace llvm {

class SAYACSubtarget;

class SAYACInstrInfo : public SAYACGenInstrInfo {
  const SAYACRegisterInfo RI;
  SAYACSubtarget &STI;

  virtual void anchor();

public:
  explicit SAYACInstrInfo(SAYACSubtarget &STI);

  // Return the SAYACRegisterInfo, which this class owns.
  const SAYACRegisterInfo &getRegisterInfo() const { return RI; }
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_SAYAC_SAYACINSTRINFO_H