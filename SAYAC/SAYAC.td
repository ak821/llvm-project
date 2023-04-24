//===-- SAYAC.td - Describe the M88000 Target Machine -------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Target-independent interfaces which we are implementing
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// SAYAC Subtarget features.
//

def SAYACMode : HwMode<"">;

//===----------------------------------------------------------------------===//
// Register File, Calling Conv, Instruction Descriptions
//===----------------------------------------------------------------------===//

include "SAYACRegisterInfo.td"
include "SAYACCallingConv.td"
include "SAYACSchedule.td"
include "SAYACInstrFormats.td"
include "SAYACInstrInfo.td"

//===----------------------------------------------------------------------===//
// SAYAC processors supported.
//===----------------------------------------------------------------------===//

def : ProcessorModel<"SAYACProc", SAYACSchedModel, []>;
// def : ProcessorModel<"mc88110", SAYACSchedModel, []>;

//===----------------------------------------------------------------------===//
// Declare the target which we are implementing
//===----------------------------------------------------------------------===//

def SAYACInstrInfo : InstrInfo;
def SAYACAsmParser : AsmParser;
def SAYACAsmParserVariant : AsmParserVariant {
  let RegisterPrefix = "%";
}

def SAYACAsmWriter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
  int PassSubtarget = 1;
  int Variant = 0;
}

def SAYAC : Target {
  let InstructionSet = SAYACInstrInfo;
  let AssemblyParsers  = [SAYACAsmParser];
  let AssemblyParserVariants = [SAYACAsmParserVariant];
  let AssemblyWriters = [SAYACAsmWriter];
  let AllowRegisterRenaming = 1;
}