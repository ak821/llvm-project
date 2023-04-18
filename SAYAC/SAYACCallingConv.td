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
 // CCIfSRet<CCIfType<[i16], CCAssignToReg<[R12]>>>,

  // Registers R7 to 11 are used for passing parameters.
  CCIfType<[i16], CCAssignToReg<[R7, R8, R9, R10, R11]>>,

  // Other arguments are passed on the stack, at least 4-byte-aligned.
  CCAssignToStack<2, 2>
]>;

// SAYAC 16-bit ELF C return-value convention.
def RetCC_SAYAC : CallingConv<[
  // 16-bit values are returned in R7, 32-bit values in pair R7/R8.
  CCIfType<[i16,f16], CCAssignToReg<[R7]>>,
  CCIfType<[i32,f32], CCAssignToReg<[R7, R8]>>
]>;

// SAYAC 16-bit ELF C callee saved registers.
def CSR_SAYAC : CalleeSavedRegs<(add R1, (sequence "R%d", 3, 6) )>;