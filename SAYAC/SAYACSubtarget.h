//===-- SAYACSubtarget.h - Define Subtarget for SAYAC -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the SAYAC specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SAYAC_SAYACSUBTARGET_H
#define LLVM_LIB_TARGET_SAYAC_SAYACSUBTARGET_H

#include "SAYACFrameLowering.h"
#include "SAYACISelLowering.h"
#include "SAYACInstrInfo.h"
#include "SAYACRegisterInfo.h"
//#include "SAYACSelectionDAGInfo.h"
#include "llvm/CodeGen/SelectionDAGTargetInfo.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/DataLayout.h"
#include <string>

#define GET_SUBTARGETINFO_HEADER
#include "SAYACGenSubtargetInfo.inc"

namespace llvm {
class StringRef;

class SAYACSubtarget : public SAYACGenSubtargetInfo {
  Triple TargetTriple;
  virtual void anchor();

  SAYACInstrInfo InstrInfo;
  SAYACTargetLowering TLInfo;
  // SelectionDAGTargetInfo TSInfo;
  SAYACFrameLowering FrameLowering;

public:
  SAYACSubtarget(const Triple &TT, const std::string &CPU, const std::string &FS,
                const TargetMachine &TM);

  // Automatically generated by tblgen.
  void ParseSubtargetFeatures(StringRef CPU, StringRef TuneCPU, StringRef FS);

  const TargetFrameLowering *getFrameLowering() const override {
    return &FrameLowering;
  }
  const SAYACInstrInfo *getInstrInfo() const override { return &InstrInfo; }
  const SAYACRegisterInfo *getRegisterInfo() const override {
    return &InstrInfo.getRegisterInfo();
  }
  const SAYACTargetLowering *getTargetLowering() const override {
    return &TLInfo;
  }
#if 0
  const SelectionDAGTargetInfo *getSelectionDAGInfo() const override {
    return &TSInfo;
  }
#endif
};

} // end namespace llvm

#endif