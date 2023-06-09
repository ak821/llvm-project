//===-- M88kMCCodeEmitter.cpp - Convert M88k code to machine code ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the M88kMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//

//#include "MCTargetDesc/M88kMCFixups.h"
#include "MCTargetDesc/M88kMCTargetDesc.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>
#include <cstdint>

using namespace llvm;

#define DEBUG_TYPE "mccodeemitter"

namespace {

class M88kMCCodeEmitter : public MCCodeEmitter {
  const MCInstrInfo &MCII;
  const MCRegisterInfo &MRI;
  MCContext &Ctx;

public:
  M88kMCCodeEmitter(const MCInstrInfo &MCII, const MCRegisterInfo &MRI,
                    MCContext &Ctx)
      : MCII(MCII), MRI(MRI), Ctx(Ctx) {}

  ~M88kMCCodeEmitter() override = default;

  // OVerride MCCodeEmitter.
  void encodeInstruction(const MCInst &MI, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

  // Generated by TableGen.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  // Functions called by the generated function.

  /// \brief Return the binary encoding of operand. If the machine
  /// operand requires relocation, record the relocation and return zero.
  unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  unsigned getPC26Encoding(const MCInst &MI, unsigned OpNo,
                           SmallVectorImpl<MCFixup> &Fixups,
                           const MCSubtargetInfo &STI) const;
};

} // end anonymous namespace

void M88kMCCodeEmitter::encodeInstruction(const MCInst &MI, raw_ostream &OS,
                                          SmallVectorImpl<MCFixup> &Fixups,
                                          const MCSubtargetInfo &STI) const {
  uint64_t Bits = getBinaryCodeForInstr(MI, Fixups, STI);
  assert(MCII.get(MI.getOpcode()).getSize() == 4 && "Unexpected instr length");

  // Emit bytes in big-endian
  for (int I = (4 - 1) * 8; I >= 0; I -= 8)
    OS << static_cast<uint8_t>((Bits >> I) & 0xff);
}

unsigned
M88kMCCodeEmitter::getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                                     SmallVectorImpl<MCFixup> &Fixups,
                                     const MCSubtargetInfo &STI) const {
  if (MO.isReg())
    return MRI.getEncodingValue(MO.getReg());
  if (MO.isImm())
    return static_cast<uint64_t>(MO.getImm());
  llvm_unreachable("Unexpected operand type!");
}

unsigned M88kMCCodeEmitter::getPC26Encoding(const MCInst &MI, unsigned OpNo,
                                            SmallVectorImpl<MCFixup> &Fixups,
                                            const MCSubtargetInfo &STI) const {
  // TODO Implement.
  return 0;
}

//#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "M88kGenMCCodeEmitter.inc"

MCCodeEmitter *llvm::createM88kMCCodeEmitter(const MCInstrInfo &MCII,
                                             const MCRegisterInfo &MRI,
                                             MCContext &Ctx) {
  return new M88kMCCodeEmitter(MCII, MRI, Ctx);
}