//===-- M88kSubtarget.h - Define Subtarget for M88k -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the M88k specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_M88K_M88KSUBTARGET_H
#define LLVM_LIB_TARGET_M88K_M88KSUBTARGET_H

#include "M88kFrameLowering.h"
#include "M88kISelLowering.h"
#include "M88kInstrInfo.h"
#include "M88kRegisterInfo.h"
//#include "M88kSelectionDAGInfo.h"
#include "llvm/CodeGen/SelectionDAGTargetInfo.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/DataLayout.h"
#include <string>

#define GET_SUBTARGETINFO_HEADER
#include "M88kGenSubtargetInfo.inc"

namespace llvm {
class StringRef;

class M88kSubtarget : public M88kGenSubtargetInfo {
  Triple TargetTriple;
  virtual void anchor();

  M88kInstrInfo InstrInfo;
  M88kTargetLowering TLInfo;
  // SelectionDAGTargetInfo TSInfo;
  M88kFrameLowering FrameLowering;

public:
  M88kSubtarget(const Triple &TT, const std::string &CPU, const std::string &FS,
                const TargetMachine &TM);

  // Automatically generated by tblgen.
  void ParseSubtargetFeatures(StringRef CPU, StringRef TuneCPU, StringRef FS);

  const TargetFrameLowering *getFrameLowering() const override {
    return &FrameLowering;
  }
  const M88kInstrInfo *getInstrInfo() const override { return &InstrInfo; }
  const M88kRegisterInfo *getRegisterInfo() const override {
    return &InstrInfo.getRegisterInfo();
  }
  const M88kTargetLowering *getTargetLowering() const override {
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