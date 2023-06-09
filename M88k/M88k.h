//===-- M88k.h - Top-level interface for M88k representation ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in the LLVM
// M88k back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_M88K_M88K_H
#define LLVM_LIB_TARGET_M88K_M88K_H

#include "MCTargetDesc/M88kMCTargetDesc.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class M88kTargetMachine;
class FunctionPass;

FunctionPass *createM88kISelDag(M88kTargetMachine &TM,
                                CodeGenOpt::Level OptLevel);
} // end namespace llvm
#endif