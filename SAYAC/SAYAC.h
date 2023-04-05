//===-- SAYAC.h - Top-level interface for SAYAC representation ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in the LLVM
// SAYAC back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SAYAC_SAYAC_H
#define LLVM_LIB_TARGET_SAYAC_SAYAC_H

#include "MCTargetDesc/SAYACMCTargetDesc.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class SAYACTargetMachine;
class FunctionPass;

FunctionPass *createSAYACISelDag(SAYACTargetMachine &TM,
                                CodeGenOpt::Level OptLevel);
} // end namespace llvm
#endif