//===-- M88kInstrFormats.td - M88k Instruction Formats -----*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

class InstM88k<dag outs, dag ins, string asmstr, list<dag> pattern,
             InstrItinClass itin = NoItinerary>
   : Instruction {
  field bits<32> Inst;

  field bits<32> SoftFail = 0; // Needed by disassembler.

  let Namespace = "M88k";
  let Size = 4;

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString   = asmstr;
  let Pattern = pattern;

  let DecoderNamespace = "M88k";

  let Itinerary = itin;
}

class Pseudo<dag outs, dag ins, list<dag> pattern>
  : InstM88k<outs, ins, "", pattern> {
  let isCodeGenOnly       = 1;
  let isPseudo            = 1;
  let hasNoSchedulingInfo = 1;
}

// The formats follow the categories specified in the User Manual.

// Category: Logical.
class F_L<dag outs, dag ins, string asmstr, list<dag> pattern,
          InstrItinClass itin = NoItinerary>
   : InstM88k<outs, ins, asmstr, pattern, itin> {
  bits<5>  rd;
  bits<5>  rs1;
  let Inst{25-21} = rd;
  let Inst{20-16} = rs1;
}

// Format: Logical with 16bit immediate.
class F_LI<bits<2> func, bits<1> upper, dag outs, dag ins, string asmstr, list<dag> pattern,
         InstrItinClass itin = NoItinerary>
   : F_L<outs, ins, asmstr, pattern, itin> {
  bits<16> imm16;
  let Inst{31-29} = 0b010;
  let Inst{28-27} = func;
  let Inst{26}    = upper;
  let Inst{15-0}  = imm16;
}

// Format: Logical with triadic register.
class F_LR<bits<5> func, bits<1> comp, dag outs, dag ins, string asmstr, list<dag> pattern,
         InstrItinClass itin = NoItinerary>
   : F_L<outs, ins, asmstr, pattern, itin> {
  bits<5>  rs2;
  let Inst{31-26} = 0b111101;
  let Inst{15-11} = func;
  let Inst{10}    = comp;
  let Inst{9-5}   = 0b00000;
  let Inst{4-0}   = rs2;
}

// Category: Bit-fields.
class F_B<bits<6> func, dag outs, dag ins, string asmstr, list<dag> pattern,
         InstrItinClass itin = NoItinerary>
   : F_L<outs, ins, asmstr, pattern, itin> {
  let Inst{31-27} = 0b11110;
  let Inst{15-10} = func;
}

// Format: Bit-field with triadic register.
class F_BR<bits<6> func, dag outs, dag ins, string asmstr, list<dag> pattern,
         InstrItinClass itin = NoItinerary>
   : F_B<func, outs, ins, asmstr, pattern, itin> {
  bits<5> rs2;
  let Inst{26}  = 0b1;
  let Inst{9-5} = 0b00000;
  let Inst{4-0} = rs2;
}

// Format: Bit-field with 10-bit immediate.
class F_BI<bits<6> func, dag outs, dag ins, string asmstr, list<dag> pattern,
         InstrItinClass itin = NoItinerary>
   : F_B<func, outs, ins, asmstr, pattern, itin> {
  bits<10> w5o5;
  let Inst{26}  = 0b0;
  let Inst{9-0} = w5o5;
}

// Category: Integer.
class F_I<dag outs, dag ins, string asmstr, list<dag> pattern,
          InstrItinClass itin = NoItinerary>
   : InstM88k<outs, ins, asmstr, pattern, itin> {
  bits<5>  rd;
  bits<5>  rs1;
  let Inst{25-21} = rd;
  let Inst{20-16} = rs1;
}

// Format: Integer with 16bit immediate.
class F_II<bits<6> func, dag outs, dag ins, string asmstr, list<dag> pattern,
           InstrItinClass itin = NoItinerary>
   : F_I<outs, ins, asmstr, pattern, itin> {
  bits<16> imm16;

  let Inst{31-26} = func;
  let Inst{15-0}  = imm16;
}

// Format: Integer with triadic register and carry.
class F_IRC<bits<6> func, bits<1> carryin, bits<1> carryout, dag outs, dag ins,
            string asmstr, list<dag> pattern, InstrItinClass itin = NoItinerary>
   : F_I<outs, ins, asmstr, pattern, itin> {
  bits<5> rs2;

  let Inst{31-26} = 0b111101;
  let Inst{15-10} = func;
  let Inst{9}     = carryin;
  let Inst{8}     = carryout;
  let Inst{7-5}   = 0b000;
  let Inst{4-0}   = rs2;
}

// Category: Load/Store/Exchange

// Format: Load/Store with word index
class F_LSINDEX<dag outs, dag ins, string asmstr, list<dag> pattern,
                InstrItinClass itin = NoItinerary>
   : InstM88k<outs, ins, asmstr, pattern, itin> {
  bits<5>  rd;
  bits<5>  rs1;
  bits<16> si16;

  let Inst{25-21} = rd;
  let Inst{20-16} = rs1;
  let Inst{15-0}  = si16;
}

// Format: Load/Store
class F_LS<bits<4> func, bits<2> ty,
               dag outs, dag ins, string asmstr, list<dag> pattern,
               InstrItinClass itin = NoItinerary>
   : F_LSINDEX<outs, ins, asmstr, pattern, itin> {
  let Inst{31-28} = func;
  let Inst{27-26} = ty;
}

// Format: Load unsigned
class F_LU<bits<1> b,
               dag outs, dag ins, string asmstr, list<dag> pattern,
               InstrItinClass itin = NoItinerary>
   : F_LSINDEX<outs, ins, asmstr, pattern, itin> {
  let Inst{31-27} = 0b00001;
  let Inst{26}    = b;
}


// Category: Flow control.

// Format: JMP/JSR
class F_JMP<bits<5> func, bits<1> next,
            dag outs, dag ins, string asmstr, list<dag> pattern,
            InstrItinClass itin = NoItinerary>
   : InstM88k<outs, ins, asmstr, pattern, itin> {
  bits<5> rs2;

  let Inst{31-26} = 0b111101;
  let Inst{25-16} = 0b0000000000;
  let Inst{15-11} = func;
  let Inst{10}    = next;
  let Inst{9-5}   = 0b00000;
  let Inst{4-0}   = rs2;
}

// Format: BR/BSR
class F_BRANCH<bits<5> func, bits<1> next,
               dag outs, dag ins, string asmstr, list<dag> pattern,
               InstrItinClass itin = NoItinerary>
   : InstM88k<outs, ins, asmstr, pattern, itin> {
  bits<26> d26;

  let Inst{31-27} = func;
  let Inst{26}    = next;
  let Inst{25-0}  = d26;
}

// Format: BB0/BB1/BCOND
class F_BCOND<bits<5> func, bits<1> next,
               dag outs, dag ins, string asmstr, list<dag> pattern,
               InstrItinClass itin = NoItinerary>
   : InstM88k<outs, ins, asmstr, pattern, itin> {
  bits<5>  m5;
  bits<5>  rs1;
  bits<16> d16;

  let Inst{31-27} = func;
  let Inst{26}    = next;
  let Inst{25-21} = m5;
  let Inst{20-16} = rs1;
  let Inst{15-0}  = d16;
}