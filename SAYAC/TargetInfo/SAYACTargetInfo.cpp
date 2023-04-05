//===-- SAYACTargetInfo.cpp - SAYAC target implementation -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/SAYACTargetInfo.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

Target &llvm::getTheSAYACTarget() {
  static Target TheSAYACTarget;
  return TheSAYACTarget;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeSAYACTargetInfo() {
  RegisterTarget<Triple::sayac, /*HasJIT=*/false> X(getTheSAYACTarget(), "sayac",
                                                   "SAYAC", "SAYAC");
}
