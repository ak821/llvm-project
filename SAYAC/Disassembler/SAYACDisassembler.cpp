//===-- SAYACDisassembler.cpp - Disassembler for SAYAC ------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SAYAC.h"
#include "MCTargetDesc/SAYACMCTargetDesc.h"
#include "TargetInfo/SAYACTargetInfo.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCFixedLenDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/TargetRegistry.h"
#include <cassert>
#include <cstdint>

using namespace llvm;

#define DEBUG_TYPE "SAYAC-disassembler"

using DecodeStatus = MCDisassembler::DecodeStatus;

namespace {

class SAYACDisassembler : public MCDisassembler {
public:
  SAYACDisassembler(const MCSubtargetInfo &STI, MCContext &Ctx)
      : MCDisassembler(STI, Ctx) {}
  ~SAYACDisassembler() override = default;

  DecodeStatus getInstruction(MCInst &instr, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &CStream) const override;
};

} // end anonymous namespace

static MCDisassembler *createSAYACDisassembler(const Target &T,
                                              const MCSubtargetInfo &STI,
                                              MCContext &Ctx) {
  return new SAYACDisassembler(STI, Ctx);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSAYACDisassembler() {
  // Register the disassembler.
  TargetRegistry::RegisterMCDisassembler(getTheSAYACTarget(),
                                         createSAYACDisassembler);
}

static const uint16_t GPRDecoderTable[] = {
    SAYAC::R0,  SAYAC::R1,  SAYAC::R2,  SAYAC::R3,  SAYAC::R4,  SAYAC::R5,
    SAYAC::R6,  SAYAC::R7,  SAYAC::R8,  SAYAC::R9,  SAYAC::R10, SAYAC::R11,
    SAYAC::R12, SAYAC::R13, SAYAC::R14, SAYAC::R15
};

static DecodeStatus DecodeGPRRegisterClass(MCInst &Inst, uint64_t RegNo,
                                           uint64_t Address,
                                           const void *Decoder) {
  if (RegNo > 15)
    return MCDisassembler::Fail;

  unsigned Register = GPRDecoderTable[RegNo];
  Inst.addOperand(MCOperand::createReg(Register));
  return MCDisassembler::Success;
}

template <unsigned N>
static DecodeStatus decodeUImmOperand(MCInst &Inst, uint64_t Imm) {
  if (!isUInt<N>(Imm))
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createImm(Imm));
  return MCDisassembler::Success;
}

static DecodeStatus decodeU5ImmOOperand(MCInst &Inst, uint64_t Imm,
                                        uint64_t Address, const void *Decoder) {
  return decodeUImmOperand<5>(Inst, Imm);
}

static DecodeStatus decodeU5ImmOperand(MCInst &Inst, uint64_t Imm,
                                        uint64_t Address, const void *Decoder) {
  return decodeUImmOperand<5>(Inst, Imm);
}

static DecodeStatus decodeU8ImmOperand(MCInst &Inst, uint64_t Imm,
                                        uint64_t Address, const void *Decoder) {
  return decodeUImmOperand<8>(Inst, Imm);
}


static DecodeStatus decodeU10ImmWOOperand(MCInst &Inst, uint64_t Imm,
                                          uint64_t Address,
                                          const void *Decoder) {
  return decodeUImmOperand<10>(Inst, Imm);
}

static DecodeStatus decodeU16ImmOperand(MCInst &Inst, uint64_t Imm,
                                        uint64_t Address, const void *Decoder) {
  return decodeUImmOperand<16>(Inst, Imm);
}

static DecodeStatus decodePC10BranchOperand(MCInst &Inst, uint64_t Imm,
                                            uint64_t Address,
                                            const void *Decoder) {
  if (!isUInt<10>(Imm))
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createImm(SignExtend64<10>(Imm)));
  return MCDisassembler::Success;
}

#include "SAYACGenDisassemblerTables.inc"

DecodeStatus SAYACDisassembler::getInstruction(MCInst &MI, uint64_t &Size,
                                              ArrayRef<uint8_t> Bytes,
                                              uint64_t Address,
                                              raw_ostream &CS) const {
  // Instruction size is always 32 bit.
  if (Bytes.size() < 4) {
    Size = 0;
    return MCDisassembler::Fail;
  }
  Size = 4;

  // Construct the instruction.
  uint32_t Inst = 0;
  for (uint32_t I = 0; I < Size; ++I)
    Inst = (Inst << 8) | Bytes[I];

  return decodeInstruction(DecoderTableSAYAC16, MI, Inst, Address, this, STI);
}