//===-- SAYACMCTargetDesc.h - SAYAC target descriptions -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SAYAC_MCTARGETDESC_SAYACMCTARGETDESC_H
#define LLVM_LIB_TARGET_SAYAC_MCTARGETDESC_SAYACMCTARGETDESC_H

#include "llvm/Support/DataTypes.h"

#include <memory>

namespace llvm {

class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;
class StringRef;
class Target;
class Triple;
class raw_pwrite_stream;
class raw_ostream;

MCCodeEmitter *createSAYACMCCodeEmitter(const MCInstrInfo &MCII,
                                       const MCRegisterInfo &MRI,
                                       MCContext &Ctx);

MCAsmBackend *createSAYACMCAsmBackend(const Target &T,
                                     const MCSubtargetInfo &STI,
                                     const MCRegisterInfo &MRI,
                                     const MCTargetOptions &Options);

std::unique_ptr<MCObjectTargetWriter> createSAYACObjectWriter(uint8_t OSABI);
} // end namespace llvm

// Defines symbolic names for SAYAC registers.
// This defines a mapping from register name to register number.
#define GET_REGINFO_ENUM
#include "SAYACGenRegisterInfo.inc"

// Defines symbolic names for the SAYAC instructions.
#define GET_INSTRINFO_ENUM
#include "SAYACGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "SAYACGenSubtargetInfo.inc"

#endif