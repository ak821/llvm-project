//===-- M88kMCTargetDesc.cpp - M88k target descriptions -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "M88kMCTargetDesc.h"
#include "M88kInstPrinter.h"
#include "M88kMCAsmInfo.h"
#include "TargetInfo/M88kTargetInfo.h"
#include "llvm/MC/MCDwarf.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#include "M88kGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "M88kGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "M88kGenRegisterInfo.inc"

static MCAsmInfo *createM88kMCAsmInfo(const MCRegisterInfo &MRI,
                                      const Triple &TT,
                                      const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new M88kMCAsmInfo(TT);
  return MAI;
}

static MCInstrInfo *createM88kMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitM88kMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createM88kMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitM88kMCRegisterInfo(X, M88k::R1);
  return X;
}

static MCSubtargetInfo *createM88kMCSubtargetInfo(const Triple &TT,
                                                  StringRef CPU, StringRef FS) {
  return createM88kMCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);
}

static MCInstPrinter *createM88kMCInstPrinter(const Triple &T,
                                              unsigned SyntaxVariant,
                                              const MCAsmInfo &MAI,
                                              const MCInstrInfo &MII,
                                              const MCRegisterInfo &MRI) {
  return new M88kInstPrinter(MAI, MII, MRI);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeM88kTargetMC() {
  // Register the MCAsmInfo.
  TargetRegistry::RegisterMCAsmInfo(getTheM88kTarget(), createM88kMCAsmInfo);

  // Register the MCCodeEmitter.
  TargetRegistry::RegisterMCCodeEmitter(getTheM88kTarget(),
                                        createM88kMCCodeEmitter);

  // Register the MCInstrInfo.
  TargetRegistry::RegisterMCInstrInfo(getTheM88kTarget(),
                                      createM88kMCInstrInfo);
  // Register the MCRegisterInfo.
  TargetRegistry::RegisterMCRegInfo(getTheM88kTarget(),
                                    createM88kMCRegisterInfo);

  // Register the MCSubtargetInfo.
  TargetRegistry::RegisterMCSubtargetInfo(getTheM88kTarget(),
                                          createM88kMCSubtargetInfo);
  // Register the MCAsmBackend.
  TargetRegistry::RegisterMCAsmBackend(getTheM88kTarget(),
                                       createM88kMCAsmBackend);
  // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(getTheM88kTarget(),
                                        createM88kMCInstPrinter);
}