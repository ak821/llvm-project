//===-- SAYACMCTargetDesc.cpp - SAYAC target descriptions -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SAYACMCTargetDesc.h"
#include "SAYACInstPrinter.h"
#include "SAYACMCAsmInfo.h"
#include "TargetInfo/SAYACTargetInfo.h"
#include "llvm/MC/MCDwarf.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#include "SAYACGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "SAYACGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "SAYACGenRegisterInfo.inc"

static MCAsmInfo *createSAYACMCAsmInfo(const MCRegisterInfo &MRI,
                                      const Triple &TT,
                                      const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new SAYACMCAsmInfo(TT);
  return MAI;
}

static MCInstrInfo *createSAYACMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitSAYACMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createSAYACMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitSAYACMCRegisterInfo(X, SAYAC::R1);
  return X;
}

static MCSubtargetInfo *createSAYACMCSubtargetInfo(const Triple &TT,
                                                  StringRef CPU, StringRef FS) {
  return createSAYACMCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);
}

static MCInstPrinter *createSAYACMCInstPrinter(const Triple &T,
                                              unsigned SyntaxVariant,
                                              const MCAsmInfo &MAI,
                                              const MCInstrInfo &MII,
                                              const MCRegisterInfo &MRI) {
  return new SAYACInstPrinter(MAI, MII, MRI);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSAYACTargetMC() {
  // Register the MCAsmInfo.
  TargetRegistry::RegisterMCAsmInfo(getTheSAYACTarget(), createSAYACMCAsmInfo);

  // Register the MCCodeEmitter.
  TargetRegistry::RegisterMCCodeEmitter(getTheSAYACTarget(),
                                        createSAYACMCCodeEmitter);

  // Register the MCInstrInfo.
  TargetRegistry::RegisterMCInstrInfo(getTheSAYACTarget(),
                                      createSAYACMCInstrInfo);
  // Register the MCRegisterInfo.
  TargetRegistry::RegisterMCRegInfo(getTheSAYACTarget(),
                                    createSAYACMCRegisterInfo);

  // Register the MCSubtargetInfo.
  TargetRegistry::RegisterMCSubtargetInfo(getTheSAYACTarget(),
                                          createSAYACMCSubtargetInfo);
  // Register the MCAsmBackend.
  TargetRegistry::RegisterMCAsmBackend(getTheSAYACTarget(),
                                       createSAYACMCAsmBackend);
  // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(getTheSAYACTarget(),
                                        createSAYACMCInstPrinter);
}