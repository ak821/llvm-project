//===-- SAYACAsmPrinter.cpp - SAYAC LLVM assembly writer ----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to GAS-format SAYAC assembly language.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/SAYACInstPrinter.h"
//#include "MCTargetDesc/SAYACMCExpr.h"
//#include "MCTargetDesc/SAYACTargetStreamer.h"
#include "SAYAC.h"
#include "SAYACInstrInfo.h"
#include "SAYACMCInstLower.h"
#include "SAYACTargetMachine.h"
#include "TargetInfo/SAYACTargetInfo.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineModuleInfoImpls.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/IR/Mangler.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "asm-printer"

// TODO:
// %hi16() and %lo16() for addresses

namespace {
class SAYACAsmPrinter : public AsmPrinter {
#if 0
    SAYACTargetStreamer &getTargetStreamer() {
      return static_cast<SAYACTargetStreamer &>(
          *OutStreamer->getTargetStreamer());
    }
#endif
public:
  explicit SAYACAsmPrinter(TargetMachine &TM,
                          std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)) {}

  StringRef getPassName() const override { return "SAYAC Assembly Printer"; }

  bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                       const char *ExtraCode, raw_ostream &OS) override;
  void emitInstruction(const MachineInstr *MI) override;
};
} // end of anonymous namespace

bool SAYACAsmPrinter::PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                                     const char *ExtraCode, raw_ostream &OS) {
  if (ExtraCode)
    return AsmPrinter::PrintAsmOperand(MI, OpNo, ExtraCode, OS);
  SAYACMCInstLower Lower(MF->getContext(), *this);
  MCOperand MO(Lower.lowerOperand(MI->getOperand(OpNo)));
  SAYACInstPrinter::printOperand(MO, MAI, OS);
  return false;
}

void SAYACAsmPrinter::emitInstruction(const MachineInstr *MI) {
  MCInst LoweredMI;
  MCInst SeteqflagMI ;
  switch (MI->getOpcode()) {
  case SAYAC::RET:
    SeteqflagMI = MCInstBuilder(SAYAC::CMI)
                            .addReg(SAYAC::R0).addImm(0);
    EmitToStreamer(*OutStreamer, SeteqflagMI);
    LoweredMI = MCInstBuilder(SAYAC::BRCeq).addReg(SAYAC::R1);
    break;
  case SAYAC::JMRS:
  {
    LoweredMI = MCInstBuilder(SAYAC::JMRS)
    .addReg(MI->getOperand(0).getReg())
    .addReg(MI->getOperand(1).getReg());
    break;
  }

  default:
    SAYACMCInstLower Lower(MF->getContext(), *this);
    Lower.lower(MI, LoweredMI);
    // doLowerInstr(MI, LoweredMI);
    break;
  }
  EmitToStreamer(*OutStreamer, LoweredMI);
}

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSAYACAsmPrinter() {
  RegisterAsmPrinter<SAYACAsmPrinter> X(getTheSAYACTarget());
}