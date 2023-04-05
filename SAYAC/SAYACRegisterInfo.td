//===-- SAYACRegisterInfo.td - SAYAC Register definitions ----*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

class SAYACReg<bits<4> Enc, string n> : Register<n> {
  let HWEncoding{15-4} = 0;
  let HWEncoding{3-0} = Enc;
  let Namespace = "SAYAC";
}

// General register
class GRi<bits<4> Enc, string n> : SAYACReg<Enc, n>;

// General-purpose registers
foreach I = 0-15 in {
  def R#I : GRi<I, "r"#I>;
}

def GPR : RegisterClass<"SAYAC", [i16], 16,
                            (add (sequence "R%u", 0, 15))>;

def GPROpnd : RegisterOperand<GPR>;
