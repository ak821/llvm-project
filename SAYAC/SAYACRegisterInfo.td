//===-- SAYACRegisterInfo.td - SAYAC Register definitions ----*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

let Namespace = "SAYAC" in {
class SAYACReg<bits<4> Enc, string n, list<string> alt = []> : Register<n> {
  let HWEncoding{15-4} = 0;
  let HWEncoding{3-0} = Enc;
  let AltNames = alt;
}

def ABIRegAltName: RegAltNameIndex ;
}

// General-purpose registers

let RegAltNameIndices = [ABIRegAltName] in {
def R0  : SAYACReg<0,  "r0",   ["zero"]>;
def R1  : SAYACReg<1,  "r1",   ["ra"]>;
def R2  : SAYACReg<2,  "r2",   ["sp"]>;
def R3  : SAYACReg<3,  "r3",   ["fp", "s0"]>;
def R4  : SAYACReg<4,  "r4",   ["s1"]>;
def R5  : SAYACReg<5,  "r5",   ["s2"]>;
def R6  : SAYACReg<6,  "r6",   ["s3"]>;
def R7  : SAYACReg<7,  "r7",   ["a0"]>;
def R8  : SAYACReg<8,  "r8",   ["a1"]>;
def R9  : SAYACReg<9,  "r9",   ["a2"]>;
def R10 : SAYACReg<10, "r10",  ["a3"]>;
def R11 : SAYACReg<11, "r11",  ["a4"]>;
def R12 : SAYACReg<12, "r12",  ["t0"]>;
def R13 : SAYACReg<13, "r13",  ["t1"]>;
def R14 : SAYACReg<14, "r14",  ["t2"]>;
def R15 : SAYACReg<15, "r15",  ["flag"]>;
}

// The order of registers represents the preferred allocation sequence.
// Registers are listed in the order caller-save, callee-save, specials.
def GPR : RegisterClass<"SAYAC", [i16], 16,
                            (add 
                            (sequence "R%u", 7, 11),
                            (sequence "R%u", 12, 14),
                            (sequence "R%u", 3, 6),
                            (sequence "R%u", 0, 2),
                            R15
                            )>;

def GPROpnd : RegisterOperand<GPR>;

def XLenVT : ValueTypeByHwMode<[SAYACMode],
                               [i16]>;