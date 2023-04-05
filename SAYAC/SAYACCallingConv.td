//===-- SAYACCallingConv.td - SAYAC Calling Conventions ------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This describes the calling conventions for the SAYAC architectures.
// Only the OpenBSD variant is supported.
//
//===----------------------------------------------------------------------===//

// SAYAC 16-bit ELF C Calling convention.
def CC_SAYAC : CallingConv<[
  // Promote i1,i8 args to i16.
  CCIfType<[i1, i8], CCPromoteToType<i16>>,

  // Register R12 is used to pass structure return pointer.
  CCIfSRet<CCIfType<[i16], CCAssignToReg<[R12]>>>,

  // Registers R2 to R6 are used for passing parameters.
  CCIfType<[i16], CCAssignToReg<[R2, R3, R4, R5, R6]>>,

  // Other arguments are passed on the stack, at least 4-byte-aligned.
  CCAssignToStack<2, 2>
]>;

// SAYAC 16-bit ELF C return-value convention.
def RetCC_SAYAC : CallingConv<[
  // 16-bit values are returned in R2, 32-bit values in pair R2/R3.
  CCIfType<[i16,f16], CCAssignToReg<[R2]>>,
  CCIfType<[i32,f32], CCAssignToReg<[R2, R3]>>
]>;

// SAYAC 16-bit ELF C callee saved registers.
def CSR_SAYAC : CalleeSavedRegs<(add (sequence "R%d", 10, 12), R13)>;