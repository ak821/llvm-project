//===-- M88kInstrInfo.td - M88k Instructions ---------------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file describes the M88k instructions in TableGen format.
//
//===----------------------------------------------------------------------===//

// ---------------------------------------------------------------------------//
// Selection DAG Nodes.
// ---------------------------------------------------------------------------//

// Selection DAG types.

// These are target-independent nodes, but have target-specific formats.
def SDT_CallSeqStart : SDCallSeqStart<[SDTCisVT<0, i16>, SDTCisVT<1, i16>]>;
def SDT_CallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, i16>, SDTCisVT<1, i16>]>;
def SDT_Call         : SDTypeProfile<0, -1, [SDTCisPtrTy<0>]>;

class ImmediateAsmOperand<string name> : AsmOperandClass {
  let Name = name;
  let RenderMethod = "addImmOperands";
}

// Selection DAG nodes.

def call             : SDNode<"SAYACISD::CALL", SDT_Call,
                              [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue,
                              SDNPVariadic]>;

def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_CallSeqStart,
                           [SDNPHasChain, SDNPOutGlue]>;
def callseq_end   : SDNode<"ISD::CALLSEQ_END", SDT_CallSeqEnd,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

def retflag          : SDNode<"SAYACISD::RET_FLAG", SDTNone,
                              [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def load_sym : SDNode<"SAYACISD::LOAD_SYM", SDTIntUnaryOp>;

// Bits 0-15.
def LO16 : SDNodeXForm<imm, [{
  uint32_t Value = N->getZExtValue() & 0x000000000000FFFFULL;
  return CurDAG->getTargetConstant(Value, SDLoc(N), MVT::i32);
}]>;

// Bits 16-31 (counting from the lsb).
def HI16 : SDNodeXForm<imm, [{
  uint64_t Value = (N->getZExtValue() & 0x00000000FFFF0000ULL) >> 16;
  return CurDAG->getTargetConstant(Value, SDLoc(N), MVT::i32);
}]>;

def ADDrr  : F_LAR<0b1001,
                   (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
                   "adr $rd $rs1 $rs2",
                   [(set GPR:$rd, (add GPR:$rs1, GPR:$rs2))]>;

def SUBrr  : F_LAR<0b1010,
                   (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
                   "sur $rd $rs1 $rs2",
                   [(set GPR:$rd, (sub GPR:$rs1, GPR:$rs2))]>;

//TODO: result greater than 16bit
def MUL  : F_LAR<0b1101,
                   (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
                   "mul $rd $rs1 $rs2",
                   [(set GPR:$rd, (mul GPR:$rs1, GPR:$rs2))]>;

def DIV  : F_LAR<0b1110,
                   (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
                   "div $rd $rs1 $rs2",
                   [(set GPR:$rd, (sdiv GPR:$rs1, GPR:$rs2))]>;

def ANDrr  : F_LAR<0b0011,
                   (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
                   "anr $rd $rs1 $rs2",
                   [(set GPR:$rd, (and GPR:$rs1, GPR:$rs2))]>;

let isPseudo = 1 in {
def ORrr  : InstSAYAC<(outs GPR:$rd, GPR: $rd2), (ins GPR: $rs1, GPR: $rs2),
                            "ORrr $rd $rs1 $rs2", [] >{
                              let Constraints = "$rd2 = $rs1";
                            }

def XORrr  : InstSAYAC<(outs GPR:$rd, GPR: $rd2), (ins GPR: $rs1, GPR: $rs2),
                            "XORrr $rd $rs1 $rs2", [] >{
                              // let Constraints = "$rd != $rs1, $rd != $rs2";
                              let Constraints = "@earlyclobber $rd, $rd2 = $rs1";
                            }

}

def : Pat<(or GPR:$rs1, GPR:$rs2), (ORrr $rs1, $rs2)>;
def : Pat<(xor GPR:$rs1, GPR:$rs2), (XORrr $rs1, $rs2)>;


def U8Imm : ImmediateAsmOperand<"U8Imm">;
def U5Imm : ImmediateAsmOperand<"U5Imm">;


class ImmediateOp<ValueType vt, string asmop> : Operand<vt> {
  let PrintMethod = "print"#asmop#"Operand";
  let DecoderMethod = "decode"#asmop#"Operand";
  let ParserMatchClass = !cast<AsmOperandClass>(asmop);
  let OperandType = "OPERAND_IMMEDIATE";
}

class ImmOpWithPattern<ValueType vt, string asmop, code pred, SDNodeXForm xform,
      SDNode ImmNode = imm> :
  ImmediateOp<vt, asmop>, PatLeaf<(vt ImmNode), pred, xform>;

multiclass Immediate<ValueType vt, code pred, SDNodeXForm xform, string asmop> {
  // def "" : ImmediateOp<vt, asmop>,
  //          PatLeaf<(vt imm), pred, xform>;
  def "" : ImmOpWithPattern<vt, asmop, pred, xform>;

//  def _timm : PatLeaf<(vt timm), pred, xform>;
  def _timm : ImmOpWithPattern<vt, asmop, pred, xform, timm>;
}


def NEG : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(-N->getSExtValue(), SDLoc(N), N->getValueType(0));
}]>;


defm imm16zx8 : Immediate<i16, [{
  return (N->getZExtValue() & ~0x00000000000000ffULL) == 0;
}], NOOP_SDNodeXForm, "U8Imm">;

defm imm16zx5 : Immediate<i16, [{
  return (N->getZExtValue() & ~0x000000000000001fULL) == 0;
}], NOOP_SDNodeXForm, "U5Imm">;

def simm16     : ImmLeaf<i16, [{return isInt<16>(Imm);}]>;
def simm12     : ImmLeaf<i16, [{return isInt<12>(Imm);}]>;
def simm8     : ImmLeaf<i16, [{return isInt<8>(Imm);}]>;
def simm5     : ImmLeaf<i16, [{return isInt<5>(Imm);}]>;


// Extract least significant 8 bits from an immediate value and sign extend
// them.
def LO8Sext : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(SignExtend32<8>(N->getZExtValue()),
                                   SDLoc(N), N->getValueType(0));
}]>;

def LO5Sext : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(SignExtend32<5>(N->getZExtValue()),
                                   SDLoc(N), N->getValueType(0));
}]>;

def Lo5SextNEG : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(-SignExtend32<5>(N->getZExtValue()),
                                   SDLoc(N), N->getValueType(0));
}]>;

// Extract the most significant 8 bits from an immediate value. Add 1 if bit
// 7 is 1, to compensate for the low 8 bits in the matching immediate addi
// or ld/st being negative.
def HI8 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(((N->getZExtValue()+0x80) >> 8) & 0xff,
                                   SDLoc(N), N->getValueType(0));
}]>;

class ImmAsmOperand<string prefix, int width, string suffix="">: AsmOperandClass {
  let Name = prefix # "ImmXLen" # suffix;
  let RenderMethod = "addImmOperands";
  // let DiagnosticType = !strconcat("Invalid", Name);
}

class UImmAsmOperand<int width, string suffix=""> : 
                        ImmAsmOperand<"U", width, suffix> {
}

def uimm8_m: Operand<i16> {
  let EncoderMethod = "getImmOpValue";
  let ParserMatchClass = UImmAsmOperand<8, "m">;
}


class PCRelAsmOperand<string size> : ImmediateAsmOperand<"PCRel"#size> {
  let PredicateMethod = "isImm";
  let ParserMethod = "parsePCRel"#size;
}

class PCRelOperand<ValueType vt, AsmOperandClass asmop> : Operand<vt> {
  let PrintMethod = "printPCRelOperand";
  let ParserMatchClass = asmop;
  let OperandType = "OPERAND_PCREL";
}

def PCRel10 : PCRelAsmOperand<"10">;

// Replace 10 with 8 here and in functions as well...
def brtarget8 : PCRelOperand<OtherVT, PCRel10> {
  let EncoderMethod = "getPC10Encoding";
  let DecoderMethod = "decodePC10BranchOperand";
}


def ImmShiftAsmOperand : AsmOperandClass { let Name = "ImmShift"; }
def immShift : Operand<i16>, PatLeaf<(imm), [{
    int Imm = N->getSExtValue();
    return Imm >= -15 && Imm <= 15;}]> {
  let ParserMatchClass = ImmShiftAsmOperand;
}



let hasSideEffects = 0, mayLoad = 0, mayStore = 0,
isReMaterializable = 1, isAsCheapAsAMove = 1 in {
def MSI : F_MOV<0b0101, (outs GPROpnd:$rd), (ins uimm8_m:$imm8),
                  "msi $rd $imm8", []>;

def MHI : F_MOV<0b0110, (outs GPROpnd: $rd), (ins uimm8_m:$imm8),
                "mhi $rd $imm8", []>;

def MSym : F_MOV_Symbol<0b0101, (outs GPR: $rd), (ins brtarget8: $d8),
                    "msym $rd $d8", []>;
}

// ADDri isn't always rematerializable, but isReMaterializable will be used as
// a hint which is verified in isReallyTriviallyReMaterializable.
let isReMaterializable = 1, isAsCheapAsAMove = 1 in
def ADDri : F_LAI<0b1011,
                 (outs GPR:$rd), (ins GPR: $rs, imm16zx8:$imm8),
                 "adi $rs $imm8",
                 [(set GPR:$rd, (add GPR:$rs, imm16zx8:$imm8))]
                 > {
                  let Constraints = "$rd = $rs";
                 }

def SUBri : F_LAI<0b1100,
                 (outs GPR:$rd), (ins GPR: $rs, imm16zx8:$imm8),
                 "sui $rs $imm8",
                 [(set GPR:$rd, (sub GPR:$rs, imm16zx8:$imm8))]
                 > {
                  let Constraints = "$rd = $rs";
                 }

def ANDri : F_LAI<0b0100,
                 (outs GPR:$rd), (ins GPR: $rs, imm16zx8:$imm8),
                 "ani $rs $imm8",
                 [(set GPR:$rd, (and GPR:$rs, imm16zx8:$imm8))]
                 > {
                  let Constraints = "$rd = $rs";
                 }


// def MOVrr : InstAlias<"mv  $rs",   (ADDri GPR:$rs,  0)>;

// def : Pattern<(simm16:$imm), 
//                 [(MSI GPROpnd: $rd, (LO8Sext imm:$imm)), (MHI (HI8 $imm))] >;

def : Pat<(simm8:$imm), (MSI (LO8Sext $imm))>;
// FIXME: Add MHI Instruction as well.


def : Pat<(simm16: $imm), (ADDri (MHI (HI8 $imm)), (LO8Sext $imm))>;

def addr : ComplexPattern<iPTR, 1, "SelectAddr", [frameindex], []>;

def IsOrAdd: PatFrag<(ops node:$A, node:$B), (or node:$A, node:$B), [{
  return isOrEquivalentToAdd(N);
}]>;

// FrameIndex
let isPseudo = 1 in 
def FI : InstSAYAC<(outs GPR:$rd), (ins GPR: $rs, imm16zx8:$imm8),
                 "fi $rs $imm8",
                 []
                 >;

/// FrameIndex calculations


def : Pat<(add (i16 addr:$Rs), simm8:$imm8),
          (FI (i16 addr:$Rs), simm8:$imm8)>;
def : Pat<(IsOrAdd (i16 addr:$Rs), simm8:$imm8),
          (FI (i16 addr:$Rs), simm8:$imm8)>;


let mayStore = 1 in 
def STR : F_Store<0b00, (outs), (ins GPR: $rd, GPR: $rs1),
                    "str $rd $rs1",
                    []
                    >;

let mayLoad = 1 in 
def LDR: F_Load<0b00, (outs GPR:$rd), (ins GPR:$rs1),
                  "ldr $rd $rs1",
                  []
                  >;


multiclass StPat<PatFrag StoreOp, InstSAYAC Inst> {
  def : Pat<(StoreOp GPR:$rs1, GPR:$rs2), (Inst GPR:$rs2, GPR:$rs1)>;
  def : Pat<(StoreOp GPR:$rs1, addr:$rs2), (Inst GPR:$rs2, addr:$rs1)>;
  // def : Pat<(StoreOp GPROpnd:$rs2, (add GPROpnd:$rs1, simm12:$imm12)),
  //           (Inst GPROpnd:$rs2, GPROpnd:$rs1, simm12:$imm12)>;
  // def : Pat<(StoreOp GPROpnd:$rs2, (add AddrFI:$rs1, simm12:$imm12)),
  //           (Inst GPROpnd:$rs2, AddrFI:$rs1, simm12:$imm12)>;
  // def : Pat<(StoreOp GPROpnd:$rs2, (IsOrAdd AddrFI:$rs1, simm12:$imm12)),
  //           (Inst GPROpnd:$rs2, AddrFI:$rs1, simm12:$imm12)>;
}

defm : StPat<truncstorei16, STR>;
defm : StPat<truncstorei8, STR>;
defm : StPat<store, STR>;


multiclass LdPat<PatFrag LoadOp, InstSAYAC Inst> {
  def : Pat<(LoadOp GPR:$rs1), (Inst GPR:$rs1)>;  
  def : Pat<(LoadOp addr:$rs1), (Inst addr:$rs1)>;
  // def : Pat<(LoadOp (add GPROpnd:$rs1, simm16:$imm16)),
  //           (Inst GPROpnd:$rs1)>;
  // def : Pat<(LoadOp (add AddrFI:$rs1, simm12:$imm12)),
  //           (Inst AddrFI:$rs1, simm12:$imm12)>;
  // def : Pat<(LoadOp (IsOrAdd AddrFI:$rs1, simm12:$imm12)),
  //           (Inst AddrFI:$rs1, simm12:$imm12)>;
}

defm : LdPat<sextloadi16, LDR>;
defm : LdPat<extloadi16, LDR>;
defm : LdPat<load, LDR>;

// def MoveImm16Ty : SDTypeProfile<1, 1, [
//   SDTCisSameAs<0, 1>, SDTCisInt<0>
// ]>;

// def movei16 : SDNode<"SAYACISD::MOVEi16", MoveImm16Ty>;

// def MOVi16 : InstSAYAC<(outs GPR:$rd), (ins i16imm:$src), "",
//                      []> {
//   let isPseudo = 1;
// }

def MOVi16sym : InstSAYAC<(outs GPR:$rd), (ins i16imm:$src), "Mi16sym $src",
                     []> {
  let isPseudo = 1;
}


let mayStore = 1 in 
def PSTR : F_Store<0b00, (outs), (ins GPR: $rd, GPR: $rs1),
                    "str $rd $rs1",
                    []
                    >;

let mayLoad = 1 in 
def PLDR: F_Load<0b00, (outs GPR:$rd), (ins GPR:$rs1),
                  "ldr $rd $rs1",
                  []
                  >;


// def memsrc : Operand<i16> {
//   let MIOperandInfo = (ops GPROpnd, i16imm);
//   let PrintMethod = "printAddrModeMemSrc";
//   let EncoderMethod = "getMemSrcValue";
// }

// let mayLoad = 1 in 
// def LDR: F_Load<0b00, (outs GPROpnd:$rd), (ins memsrc:$addr),
//                   "ldr $rd $addr",
//                   [(set i16: $rd, (load addr:$addr))]
//                   >;

// let mayStore = 1 in
// def STR: F_Store<0b00, (outs GPROpnd:$rd), (ins memsrc:$addr),
//                   "str $rd $addr",
//                   [(store i16:$rd, addr:$addr)]
//                   >;


// let mayStore = 1 in
// def STR: F_Store<0b00, (outs), (ins GPROpnd:$rs1, memsrc:$rd),
//                   "str $rd $rs1",
//                   [(store i16:$rs1, addr:$rd)]
//                   >;

// let mayLoad = 1 in 
// def LDR: F_Load<0b00, (outs GPROpnd:$rd), (ins memsrc:$rs1),
//                   "ldr $rd $rs1",
//                   [(set GPROpnd: $rd, (load addr:$rs1))]
//                   >;


def : Pat<(i16 (load_sym tglobaladdr:$addr)), (MOVi16sym $addr)>;

// def : Pattern<(store i16imm:$rd, addr:$addr), [(STR (MSI (LO8Sext i16imm:$rd)), $addr)]>;

// def : Pattern<(store i16:$rd, addr:$addr), [(STR (MSI (LO8Sext i16imm:$rd)), $addr)]>;


// Logical Shift
def SLR  : F_LAR<0b0111,
                   (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
                   "slr $rd, $rs1, $rs2",
                   []>;

// Arithmetic Shift
def SAR  : F_LAR<0b1000,
                   (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
                   "sar $rd, $rs1, $rs2",
                   []>;

// Logical Shift Immediate
def SHIl : F_SHI<0b0, 
                   (outs GPR:$rd), (ins GPR:$rs, immShift:$imm),
                   "shil $imm, $rs",
                   []>{
                      let Constraints = "$rd = $rs";
                    }

// Arithmetic Shift Immediate
def SHIa : F_SHI<0b1, 
                   (outs GPR:$rd), (ins GPR:$rs, immShift:$imm),
                   "shia $imm, $rs",
                   []>{
                      let Constraints = "$rd = $rs";
                    }

def : Pat<(shl GPR:$rs1, immShift: $imm), (SHIl GPR: $rs1, (NEG $imm ))>;
def : Pat<(srl GPR:$rs1, immShift:$imm), (SHIl GPR:$rs1,  $imm  ) >;
def : Pat<(sra GPR:$rs1, immShift:$imm), (SHIa GPR:$rs1,  $imm  ) >;

def : Pat<(shl GPR:$rs1, immShift: $imm), (SLR GPR: $rs1, (MSI (Lo5SextNEG $imm )))>;
def : Pat<(srl GPR:$rs1, immShift:$imm), (SLR GPR:$rs1, (MSI (LO5Sext $imm )) ) >;
def : Pat<(sra GPR:$rs1, immShift:$imm), (SAR GPR:$rs1, (MSI (LO5Sext $imm )) ) >;


/// Generic pattern classes

// class PatGprGpr<SDPatternOperator OpNode, InstSAYAC Inst>
//     : Pat<(OpNode GPR:$rs1, GPR:$rs2), (Inst GPR:$rs1, GPR:$rs2)>;
// class PatGprSimm5<SDPatternOperator OpNode, InstSAYAC Inst>
//     : Pat<(OpNode GPR:$rs1, simm5:$imm12), (Inst GPR:$rs1, simm5:$imm12)>;

// Comapare Instructions
let isCompare = 1 in {
  def CMR : F_CMR<0b0, (outs), (ins GPR:$rs,  GPR:$rd),
                     "cmr $rs $rd", []>;
  // def CMI : F_CMI<0b1, (outs), (ins GPR:$rd, imm16zx5: $imm),
  //                    "cmi $imm $rd", []> ;
  // def CMRS : F_CMR<0b0, (outs), (ins GPR:$rs,  GPR:$rd),
  //                    "cmrs $rs $rd", []> ;
  def CMI : F_CMI<0b1, (outs), (ins GPR:$rd, imm16zx5: $imm),
                     "cmi $imm $rd", []> ;
  // def CMIri : F_COMPARE<0b1, (outs), (ins GPROpnd:$rd, imm16zx8:$imm5),
  //                    "cmi $imm5 $rd", []> ;
}

let isPseudo = 1 in {
  def SEQ : InstSAYAC<(outs GPR:$rd), (ins GPR: $rs1, GPR: $rs2),
                            "SEQ $rd $rs1 $rs2", [] >;
  def SNE : InstSAYAC<(outs GPR:$rd), (ins GPR: $rs1, GPR: $rs2),
                            "SEQ $rd $rs1 $rs2", [] >;
  def SUGT : InstSAYAC<(outs GPR:$rd), (ins GPR: $rs1, GPR: $rs2),
                            "SUGT $rd $rs1 $rs2", [] >;
  def SUGE : InstSAYAC<(outs GPR:$rd), (ins GPR: $rs1, GPR: $rs2),
                            "SUGE $rd $rs1 $rs2", [] >;
  def SULT : InstSAYAC<(outs GPR:$rd), (ins GPR: $rs1, GPR: $rs2),
                            "SULT $rd $rs1 $rs2", [] >;
  def SULE : InstSAYAC<(outs GPR:$rd), (ins GPR: $rs1, GPR: $rs2),
                            "SULE $rd $rs1 $rs2", [] >;
  def SGT : InstSAYAC<(outs GPR:$rd), (ins GPR: $rs1, GPR: $rs2),
                            "SGT $rd $rs1 $rs2", [] >;
  def SGE : InstSAYAC<(outs GPR:$rd), (ins GPR: $rs1, GPR: $rs2),
                            "SGE $rd $rs1 $rs2", [] >;
  def SLT : InstSAYAC<(outs GPR:$rd), (ins GPR: $rs1, GPR: $rs2),
                            "SLT $rd $rs1 $rs2", [] >;
  def SLE : InstSAYAC<(outs GPR:$rd), (ins GPR: $rs1, GPR: $rs2),
                            "SLE $rd $rs1 $rs2", [] >;
}


def : Pat<(seteq GPR:$rs1, GPR:$rs2), (SEQ $rs1, $rs2)>;
def : Pat<(setne GPR:$rs1, GPR:$rs2), (SEQ $rs1, $rs2)>;
def : Pat<(setugt GPR:$rs1, GPR:$rs2), (SUGT $rs1, $rs2)>;
def : Pat<(setuge GPR:$rs1, GPR:$rs2), (SUGE $rs1, $rs2)>;
def : Pat<(setult GPR:$rs1, GPR:$rs2), (SULT $rs1, $rs2)>;
def : Pat<(setule GPR:$rs1, GPR:$rs2), (SULE $rs1, $rs2)>;

def : Pat<(setgt  GPR:$rs1, GPR:$rs2), (SGT $rs1, $rs2)>;
def : Pat<(setge  GPR:$rs1, GPR:$rs2), (SGT $rs1, $rs2)>;
def : Pat<(setlt  GPR:$rs1, GPR:$rs2), (SGT $rs1, $rs2)>;
def : Pat<(setle  GPR:$rs1, GPR:$rs2), (SGT $rs1, $rs2)>;


// def : PatGprGpr<seteq, SEQ>;
// def : PatGprSimm12<setlt, SLTI>;
// def : PatGprGpr<setult, SLTU>;
// def : PatGprSimm12<setult, SLTIU>;

//def: Pattern<(seteq GPR:$rs, GPR:$rd), [(CMR GPR:$rs, GPR:$rd), (SHI (AND SAYAC::R15, (MSI 16) ), 4  )]>;
//def: Pattern<(setne GPR:$rs, GPR:$rd), [(CMR GPR:$rs, GPR:$rd), (SHI (AND SAYAC::R15, (MSI 16) ), 4  )]>;



let isPseudo = 1 in {
  def PJMR : InstSAYAC<(outs GPR: $rd), (ins brtarget8: $d8), "pjmr $d8",
                    [(set GPR:$rd, (br bb:$d8))]>;
  def pcall : InstSAYAC<(outs GPR: $rd), (ins GPR: $addr), "pcall $addr", []>;
  def PJMRS : InstSAYAC<(outs), (ins GPR: $addr), "pjmrs $addr", []>;
}


let isTerminator = 1 in {
def JMR : F_CH<0b0010100, (outs), (ins GPR: $rd), "jmr $rd", []>;
}

let isTerminator = 1, isCall = 1, Defs = [R1] in {
  def JMRS : F_CH<0b0010101, (outs GPR: $rd), (ins GPR: $rs), "jmrs $rd $rs", []>;
}


def : Pat<(call i16: $addr), (PJMRS i16: $addr)>;

let isReturn = 1, isTerminator = 1, isBarrier = 1 in {
  def RET : Pseudo<(outs), (ins), [(retflag)]>;
}

let hasSideEffects = 0, mayLoad = 0, mayStore = 0, isBranch = 1, isTerminator = 1 in {
  def BRCeq : F_BRANCH<0b0, 0b000, (outs), (ins GPROpnd:$rd),
                     "brc 0x0 $rd", []>;
  def BRCne : F_BRANCH<0b0, 0b101, (outs), (ins GPROpnd:$rd),
                     "brc 0x5 $rd", []>;
  def BRCgt : F_BRANCH<0b0, 0b010, (outs), (ins GPROpnd:$rd),
                     "brc 0x2 $rd", []>;
  def BRCge : F_BRANCH<0b0, 0b011, (outs), (ins GPROpnd:$rd),
                     "brc 0x3 $rd", []>;
  def BRClt : F_BRANCH<0b0, 0b001, (outs), (ins GPROpnd:$rd),
                     "brc 0x1 $rd", []>;
  def BRCle : F_BRANCH<0b0, 0b100, (outs), (ins GPROpnd:$rd),
                     "brc 0x4 $rd", []>;
}

def CCSIOp : PatLeaf<(cond), [{
  switch (N->get()) {
  default:          return true;
  case ISD::SETULT:
  case ISD::SETULE:
  case ISD::SETUGT:
  case ISD::SETUGE: return false;
  }
}]>;

let isPseudo = 1 in {
  def CBeq : InstSAYAC<(outs GPR:$rd), (ins GPR: $rs1, GPR: $rs2, brtarget8:$imm8),
                            "cmpbeq $rs1 $rs2 $imm8", [] >;
  def CBne : InstSAYAC<(outs GPR:$rd), (ins GPR: $rs1, GPR: $rs2, brtarget8:$imm8),
                            "cmpbne $rs1 $rs2 $imm8", [] >;
  def CBugt : InstSAYAC<(outs GPR:$rd), (ins GPR: $rs1, GPR: $rs2, brtarget8:$imm8),
                            "cmpbugt $rs1 $rs2 $imm8", [] >;
  def CBuge : InstSAYAC<(outs GPR:$rd), (ins GPR: $rs1, GPR: $rs2, brtarget8:$imm8),
                            "cmpbuge $rs1 $rs2 $imm8", [] >;
  def CBult : InstSAYAC<(outs GPR:$rd), (ins GPR: $rs1, GPR: $rs2, brtarget8:$imm8),
                            "cmpbult $rs1 $rs2 $imm8", [] >;
  def CBule : InstSAYAC<(outs GPR:$rd), (ins GPR: $rs1, GPR: $rs2, brtarget8:$imm8),
                            "cmpbule $rs1 $rs2 $imm8", [] >;
  def CBgt : InstSAYAC<(outs GPR:$rd), (ins GPR: $rs1, GPR: $rs2, brtarget8:$imm8),
                            "cmpbgt $rs1 $rs2 $imm8", [] >;
  def CBge : InstSAYAC<(outs GPR:$rd), (ins GPR: $rs1, GPR: $rs2, brtarget8:$imm8),
                            "cmpbge $rs1 $rs2 $imm8", [] >;
  def CBlt : InstSAYAC<(outs GPR:$rd), (ins GPR: $rs1, GPR: $rs2, brtarget8:$imm8),
                            "cmpblt $rs1 $rs2 $imm8", [] >;
  def CBle : InstSAYAC<(outs GPR:$rd), (ins GPR: $rs1, GPR: $rs2, brtarget8:$imm8),
                            "cmpble $rs1 $rs2 $imm8", [] >;
}


class BccPat<PatFrag CondOp, InstSAYAC Inst>
    : Pat<(brcond (XLenVT (CondOp GPR:$rs1, GPR:$rs2)), bb:$imm8),
          (Inst GPR:$rs1, GPR:$rs2, brtarget8:$imm8)>;

// def : BccPat<seteq, BRRne>;
// def : BccPat<setne, BRRne>;


def : BccPat<seteq,   CBeq>;
def : BccPat<setne,   CBne>;
def : BccPat<setugt,  CBugt>;
def : BccPat<setuge,  CBuge>;
def : BccPat<setult,  CBult>;
def : BccPat<setule,  CBule>;
def : BccPat<setgt,   CBgt>;
def : BccPat<setge,   CBge>;
def : BccPat<setlt,   CBlt>;
def : BccPat<setle,   CBle>;


// def : BccPat<setlt, BRRlt>;
// def : BccPat<setge, BRRge>;
// def : BccPat<setult, BLTU>;
// def : BccPat<setuge, BGEU>;

// class BccSwapPat<PatFrag CondOp, InstSAYAC Inst>
//     : Pattern<(brcond (CondOp GPR:$rs1, GPR:$rs2), bb:$imm12),
//           [(CMRS GPR:$rs2, GPR:$rs1), (Inst (simm16:$imm12))]>;
//           // (Inst GPR:$rs2, GPR:$rs1, bb:$imm12)>;

// // Condition codes that don't have matching RISC-V branch instructions, but
// // are trivially supported by swapping the two input operands
// def : BccSwapPat<setgt, BRRlt>;
// def : BccSwapPat<setle, BRRge>;
// def : BccSwapPat<setugt, BLTU>;
// def : BccSwapPat<setule, BGEU>;

// An extra pattern is needed for a brcond without a setcc (i.e. where the
// condition was calculated elsewhere).
// def : Pattern<(brcond GPR:$cond, bb:$imm12), [(CMRS GPR:$cond, R0), (BRRne (bb:$imm12))]>;
// def : Pat<(brcond GPR:$cond, bb:$imm12), (CmpBranch $cond, R0, $imm12)>;



// Logical NOT
def NTR2c : F_NTR<0b1, 
                 (outs GPR:$rd), (ins GPR: $rs),
                 "ntr2 $rd $rs",
                 [(set GPR:$rd, (not GPR:$rs))]
                 > ;

def NTD2c : F_NTD<0b1, 
                 (outs GPR:$rd), (ins GPR: $rs),
                 "ntd2 $rs",
                 [(set GPR:$rd, (not GPR:$rs))]
                 > 
                 {
                  let Constraints = "$rd = $rs";
                 }

//TODO: NTR1c and NTD1c (see SAYAC ISR)



let Defs = [R2], Uses = [R2], isPseudo = 1 in {
def ADJCALLSTACKDOWN : InstSAYAC<(outs), (ins i16imm:$amt1, i16imm:$amt2),
                                    "# ADJCALLSTACKDOWN $amt1",
                                    [(callseq_start timm:$amt1, timm:$amt2)]>;
def ADJCALLSTACKUP   : InstSAYAC<(outs), (ins i16imm:$amt1, i16imm:$amt2),
                                    "# ADJCALLSTACKUP $amt1",
                                    [(callseq_end timm:$amt1, timm:$amt2)]>;
}


// def m88k_clr : SDNode<"M88kISD::CLR", SDTIntBinOp>;
// def m88k_set : SDNode<"M88kISD::SET", SDTIntBinOp>;
// def m88k_ext : SDNode<"M88kISD::EXT", SDTIntBinOp>;
// def m88k_extu : SDNode<"M88kISD::EXTU", SDTIntBinOp>;
// def m88k_mak : SDNode<"M88kISD::MAK", SDTIntBinOp>;
// def m88k_rot : SDNode<"M88kISD::ROT", SDTIntBinOp>;
// def m88k_ff1 : SDNode<"M88kISD::FF1", SDTIntBitCountUnaryOp>;
// def m88k_ff0 : SDNode<"M88kISD::FF0", SDTIntBitCountUnaryOp>;

// def m88k_bb0 : SDNode<"M88kISD::BB0", SDTIntBinOp>;
// def m88k_bb1 : SDNode<"M88kISD::BB0", SDTIntBinOp>;
// def m88k_bcond : SDNode<"M88kISD::BB0", SDTIntBinOp>;


// // Hi16 and Lo16 nodes are used to handle global addresses.
// // TODO Name very similar to LO16/HI16
// def Hi16 : SDNode<"M88kISD::Hi16", SDTIntUnaryOp>;
// def Lo16 : SDNode<"M88kISD::Lo16", SDTIntUnaryOp>;

// // ---------------------------------------------------------------------------//
// // Operands.
// // ---------------------------------------------------------------------------//

// class ImmediateOp<ValueType vt, string asmop> : Operand<vt> {
//   let PrintMethod = "print"#asmop#"Operand";
//   let DecoderMethod = "decode"#asmop#"Operand";
//   let ParserMatchClass = !cast<AsmOperandClass>(asmop);
//   let OperandType = "OPERAND_IMMEDIATE";
// }

// class ImmOpWithPattern<ValueType vt, string asmop, code pred, SDNodeXForm xform,
//       SDNode ImmNode = imm> :
//   ImmediateOp<vt, asmop>, PatLeaf<(vt ImmNode), pred, xform>;

// multiclass Immediate<ValueType vt, code pred, SDNodeXForm xform, string asmop> {
//   // def "" : ImmediateOp<vt, asmop>,
//   //          PatLeaf<(vt imm), pred, xform>;
//   def "" : ImmOpWithPattern<vt, asmop, pred, xform>;

// //  def _timm : PatLeaf<(vt timm), pred, xform>;
//   def _timm : ImmOpWithPattern<vt, asmop, pred, xform, timm>;
// }

// // Constructs an asm operand for a PC-relative address.  SIZE says how
// // many bits there are.
// class PCRelAsmOperand<string size> : ImmediateAsmOperand<"PCRel"#size> {
//   let PredicateMethod = "isImm";
//   let ParserMethod = "parsePCRel"#size;
// }

// // Constructs an operand for a PC-relative address with address type VT.
// // ASMOP is the associated asm operand.
// class PCRelOperand<ValueType vt, AsmOperandClass asmop> : Operand<vt> {
//   let PrintMethod = "printPCRelOperand";
//   let ParserMatchClass = asmop;
//   let OperandType = "OPERAND_PCREL";
// }


// // Signed and unsigned operands.
// def U5Imm : ImmediateAsmOperand<"U5Imm">;
// def U5ImmO : ImmediateAsmOperand<"U5ImmO">  {
//   let ParserMethod = "parseImmWO";
// }
// def U10ImmWO : ImmediateAsmOperand<"U10ImmWO"> {
//   let ParserMethod = "parseImmWO";
// }
// def S16Imm : ImmediateAsmOperand<"S16Imm">;
// def U16Imm : ImmediateAsmOperand<"U16Imm">;

// // PC-relative asm operands.
// def PCRel16 : PCRelAsmOperand<"16">;
// def PCRel26 : PCRelAsmOperand<"26">;

// // PC-relative offsets of a basic block.  The offset is sign-extended
// // and shifted left by 2 bits.
// def brtarget16 : PCRelOperand<OtherVT, PCRel16> {
//   let EncoderMethod = "getPC16Encoding";
//   let DecoderMethod = "decodePC16BranchOperand";
// }

// def brtarget26 : PCRelOperand<OtherVT, PCRel26> {
//   let EncoderMethod = "getPC26Encoding";
//   let DecoderMethod = "decodePC26BranchOperand";
// }


// // Extracting immediate operands from nodes.



// // Immediates for the lower and upper 16 bits of an i32, with the other
// // bits of the i32 being zero.
// defm imm32lo16 : Immediate<i32, [{
//   return (N->getZExtValue() & ~0x000000000000ffffULL) == 0;
// }], LO16, "U16Imm">;

// defm imm32hi16 : Immediate<i32, [{
//   return (N->getZExtValue() & ~0x00000000ffff0000ULL) == 0;
// }], HI16, "U16Imm">;

// // Immediates for the lower and upper 16 bits of an i32, with the other
// // bits of the i32 being one.
// defm imm32lo16c : Immediate<i32, [{
//   return (uint32_t(~N->getZExtValue()) & ~0x000000000000ffffULL) == 0;
// }], LO16, "U16Imm">;

// defm imm32hi16c : Immediate<i32, [{
//   return (uint32_t(~N->getZExtValue()) & ~0x00000000ffff0000ULL) == 0;
// }], HI16, "U16Imm">;

// defm imm32zx5 : Immediate<i32, [{
//   return (N->getZExtValue() & ~0x000000000000001fULL) == 0;
// }], NOOP_SDNodeXForm, "U5Imm">;

// defm imm32zx16 : Immediate<i32, [{
//   return (N->getZExtValue() & ~0x00000000000000ffULL) == 0;
// }], NOOP_SDNodeXForm, "U16Imm">;

// defm imm32zx5O : Immediate<i32, [{
//   return (N->getZExtValue() & ~0x000000000000001fULL) == 0;
// }], NOOP_SDNodeXForm, "U5ImmO">;

// defm imm32zx10WO : Immediate<i32, [{
//   return (N->getZExtValue() & ~0x00000000000003ffULL) == 0;
// }], NOOP_SDNodeXForm, "U10ImmWO">;

// // Predicate: Arbitrary 32 bit value.
// def uimm32 : PatLeaf<(imm), [{
//   uint64_t Val = N->getZExtValue();
//   return isUInt<32>(Val) && (Val & 0xffff);
// }]>;


// // Multiclass for logical instructions with immediates.
// // The pattern for "and" is slightly different.
// multiclass LogicImm<bits<2> FuncI, string OpcStr, SDNode OpNode,
//                   InstrItinClass itin = NoItinerary> {
//   def ri  : F_LI<FuncI, 0b0,
//                  (outs GPROpnd:$rd),
//                  !if(!eq(OpcStr, "and"),
//                    (ins GPROpnd:$rs1, imm32lo16c:$imm16),
//                    (ins GPROpnd:$rs1, imm32lo16:$imm16)
//                  ),
//                  !strconcat(OpcStr, " $rd, $rs1, $imm16"),
//                  !if(!eq(OpcStr, "and"),
//                    [(set i32:$rd, (OpNode GPROpnd:$rs1, imm32lo16c:$imm16))],
//                    [(set i32:$rd, (OpNode GPROpnd:$rs1, imm32lo16:$imm16))]
//                  ),
//                  itin>;
//   def riu  : F_LI<FuncI, 0b1,
//                  (outs GPROpnd:$rd),
//                  !if(!eq(OpcStr, "and"),
//                    (ins GPROpnd:$rs1, imm32hi16c:$imm16),
//                    (ins GPROpnd:$rs1, imm32hi16:$imm16)
//                  ),
//                  !strconcat(OpcStr, ".u $rd, $rs1, $imm16"),
//                  !if(!eq(OpcStr, "and"),
//                    [(set i32:$rd, (OpNode GPROpnd:$rs1, imm32hi16c:$imm16))],
//                    [(set i32:$rd, (OpNode GPROpnd:$rs1, imm32hi16:$imm16))]
//                  ),
//                  itin>;
// }

// // Multiclass for logical instructions with triadic registers or immediates.
// multiclass Logic<bits<5> FuncR, bits<2> FuncI, string OpcStr, SDNode OpNode,
//                  InstrItinClass itin = NoItinerary>
//                  : LogicImm<FuncI, OpcStr, OpNode, itin> {
//   let isCommutable = 1 in
//     def rr  : F_LR<FuncR, 0b0,
//                    (outs GPROpnd:$rd), (ins GPROpnd:$rs1, GPROpnd:$rs2),
//                    !strconcat(OpcStr, " $rd, $rs1, $rs2"),
//                    [(set i32:$rd, (OpNode GPROpnd:$rs1, GPROpnd:$rs2))],
//                    itin>;
//   def rrc : F_LR<FuncR, 0b1,
//                  (outs GPROpnd:$rd), (ins GPROpnd:$rs1, GPROpnd:$rs2),
//                  !strconcat(OpcStr, ".c $rd, $rs1, $rs2"),
//                  [(set i32:$rd, (OpNode GPROpnd:$rs1, (not GPROpnd:$rs2)))],
//                  itin>;
// }

// defm MASK : LogicImm<0b01, "mask", and>;
// defm AND : Logic<0b01000, 0b00, "and", and>;
// defm XOR : Logic<0b01010, 0b10, "xor", xor>;
// defm OR  : Logic<0b01011, 0b11, "or", or>;

// // Pattern for 32 bit constants.
// def : Pat<(and GPR:$rs1, uimm32:$imm),
//           (ANDri (ANDriu GPR:$rs1, (HI16 i32:$imm)), (LO16 i32:$imm))>;
// def : Pat<(or GPR:$rs1, uimm32:(HI16 i32:$imm)), (LO16 i32:$imm))>;
// def : Pat<(xor GPR:$rs1, uimm32:$$imm),
//           (ORri (ORriu GPR:$rs1, imm),
//           (XORri (XORriu GPR:$rs1, (HI16 i32:$imm)), (LO16 i32:$imm))>;


// // Multiclass for bit-field instructions with triadic registers or immediates.
// multiclass Bitfield<bits<6> Func, string OpcStr, SDNode OpNode, PatLeaf ImmOp,
//                  InstrItinClass itin = NoItinerary> {
//   def rr  : F_BR<Func,
//                  (outs GPROpnd:$rd), (ins GPROpnd:$rs1, GPROpnd:$rs2),
//                  !strconcat(OpcStr, " $rd, $rs1, $rs2"),
//                  [(set GPROpnd:$rd, (OpNode GPROpnd:$rs1, GPROpnd:$rs2))],
//                  itin>;
//   def rwo : F_BI<Func,
//                  (outs GPROpnd:$rd), (ins GPROpnd:$rs1, ImmOp:$w5o5),
//                  !strconcat(OpcStr, " $rd, $rs1, $w5o5"),
//                  [(set GPROpnd:$rd, (OpNode GPROpnd:$rs1, ImmOp:$w5o5))],
//                  itin>;
// }

// defm CLR  : Bitfield<0b100000, "clr", m88k_clr, imm32zx10WO>;
// defm SET  : Bitfield<0b100010, "set", m88k_set, imm32zx10WO>;
// defm EXT  : Bitfield<0b100100, "ext", m88k_ext, imm32zx10WO>;
// defm EXTU : Bitfield<0b100110, "extu", m88k_extu, imm32zx10WO>;
// defm MAK  : Bitfield<0b101000, "mak", m88k_mak, imm32zx10WO>;
// defm ROT  : Bitfield<0b101010, "rot", m88k_rot, imm32zx5O>;

// // Pattern for shifts
// def : Pat<(sra GPR:$rs1, GPR:$rs2), (EXTrr GPR:$rs1, GPR:$rs2)>;
// def : Pat<(srl GPR:$rs1, GPR:$rs2), (EXTUrr GPR:$rs1, GPR:$rs2)>;
// def : Pat<(shl GPR:$rs1, GPR:$rs2), (MAKrr GPR:$rs1, GPR:$rs2)>;
// def : Pat<(rotr GPR:$rs1, GPR:$rs2), (ROTrr GPR:$rs1, GPR:$rs2)>;
// def : Pat<(sra GPR:$rs1, imm32zx5O:$o5), (EXTrwo GPR:$rs1, imm32zx5O:$o5)>;
// def : Pat<(srl GPR:$rs1, imm32zx5O:$o5), (EXTUrwo GPR:$rs1, imm32zx5O:$o5)>;
// def : Pat<(shl GPR:$rs1, imm32zx5O:$o5), (MAKrwo GPR:$rs1, imm32zx5O:$o5)>;
// def : Pat<(rotr GPR:$rs1, imm32zx5O:$o5), (ROTrwo GPR:$rs1, imm32zx5O:$o5)>;

// let rs1 = 0 in
// class FindBF<bits<6> Func, string OpcStr, SDNode OpNode,
//             InstrItinClass itin = NoItinerary> :
//   F_BR<Func, (outs GPROpnd:$rd), (ins GPROpnd:$rs2),
//              !strconcat(OpcStr, " $rd, $rs2"),
//              [(set GPROpnd:$rd, (OpNode GPROpnd:$rs2))],
//              itin>;

// def FF1rr : FindBF<0b111010, "ff1", m88k_ff1>;
// def FF0rr : FindBF<0b111011, "ff0", m88k_ff0>;

// // ctlz = 32 - ff1
// //def : Pat<(ctlz GPR:$rs1, GPR:$rs2), (SUBri (i32 32), (FF1rr GPR:$rs1, GPR:$rs2))>;

// // Multiclass for arithmetic instructions with triadic registers or immediates.
// multiclass Arith<bits<6> Func, string OpcStr, SDNode OpNode,
//                  InstrItinClass itin = NoItinerary> {
//   def rr  : F_IRC<Func, 0b0, 0b0,
//                  (outs GPROpnd:$rd), (ins GPROpnd:$rs1, GPROpnd:$rs2),
//                  !strconcat(OpcStr, " $rd, $rs1, $rs2"),
//                  [(set GPROpnd:$rd, (OpNode GPROpnd:$rs1, GPROpnd:$rs2))],
//                  itin>;
//   def rrci  : F_IRC<Func, 0b1, 0b0,
//                  (outs GPROpnd:$rd), (ins GPROpnd:$rs1, GPROpnd:$rs2),
//                  !strconcat(OpcStr, ".ci $rd, $rs1, $rs2"),
//                  [(set GPROpnd:$rd, (OpNode GPROpnd:$rs1, GPROpnd:$rs2))],
//                  itin>;
//   def rrco  : F_IRC<Func, 0b0, 0b1,
//                  (outs GPROpnd:$rd), (ins GPROpnd:$rs1, GPROpnd:$rs2),
//                  !strconcat(OpcStr, ".co $rd, $rs1, $rs2"),
//                  [(set GPROpnd:$rd, (OpNode GPROpnd:$rs1, GPROpnd:$rs2))],
//                  itin>;
//   def rrcio : F_IRC<Func, 0b1, 0b1,
//                  (outs GPROpnd:$rd), (ins GPROpnd:$rs1, GPROpnd:$rs2),
//                  !strconcat(OpcStr, ".cio $rd, $rs1, $rs2"),
//                  [(set GPROpnd:$rd, (OpNode GPROpnd:$rs1, GPROpnd:$rs2))],
//                  itin>;
//   def ri  : F_II<Func,
//                  (outs GPROpnd:$rd), (ins GPROpnd:$rs1, imm32zx16:$imm16),
//                  !strconcat(OpcStr, " $rd, $rs1, $imm16"),
//                  [(set GPROpnd:$rd, (OpNode GPROpnd:$rs1, imm32zx16:$imm16))],
//                  itin>;
// }

// let isCommutable = 1 in
// defm ADDU : Arith<0b011000, "addu", add>;
// defm SUBU : Arith<0b011001, "subu", sub>;

// def CMPrr : F_IRC<0b011111, 0b0, 0b0,
//                  (outs GPROpnd:$rd), (ins GPROpnd:$rs1, GPROpnd:$rs2),
//                  "cmp $rd, $rs1, $rs2",
//                  [], //[(set GPROpnd:$rd, (OpNode GPROpnd:$rs1, GPROpnd:$rs2))],
//                  NoItinerary>;

// // Multiclass for load and store instructions.
// multiclass LoadStore<bits<4> Func, string OpcStr, SDNode OpNode,
//                  InstrItinClass itin = NoItinerary> {
//   def riw  : F_LS<Func, 0b01,
//                   (outs GPROpnd:$rd), (ins GPROpnd:$rs1, imm32zx16:$si16),
//                   !strconcat(OpcStr, " $rd, $rs1, $si16"),
//                   [],
//                   itin>;
//   def rib  : F_LS<Func, 0b11,
//                   (outs GPROpnd:$rd), (ins GPROpnd:$rs1, imm32zx16:$si16),
//                   !strconcat(OpcStr, ".b $rd, $rs1, $si16"),
//                   [],
//                   itin>;
//   def rih  : F_LS<Func, 0b10,
//                   (outs GPROpnd:$rd), (ins GPROpnd:$rs1, imm32zx16:$si16),
//                   !strconcat(OpcStr, ".h $rd, $rs1, $si16"),
//                   [],
//                   itin>;
//   def rid  : F_LS<Func, 0b00,
//                   (outs GPROpnd:$rd), (ins GPROpnd:$rs1, imm32zx16:$si16),
//                   !strconcat(OpcStr, ".d $rd, $rs1, $si16"),
//                   [],
//                   itin>;
// }

// class LoadUnsigned<bits<1> b, string OpcStr,
//                  InstrItinClass itin = NoItinerary> :
//   F_LU<b, (outs GPROpnd:$rd), (ins GPROpnd:$rs1, imm32zx16:$si16),
//        !strconcat(OpcStr, " $rd, $rs1, $si16"),
//        [],
//        itin>;

// defm LD : LoadStore<0b0001, "ld", load>;
// defm ST : LoadStore<0b0010, "st", null_frag>;
// def LDurih : LoadUnsigned<0b0, "ld.hu">;
// def LDurib : LoadUnsigned<0b1, "ld.bu">;

// let isTerminator = 1, isBarrier = 1 in {
//   def JMP : F_JMP<0b11000, 0, (outs), (ins GPROpnd:$rs2),
//                   "jmp $rs2", [(brind GPROpnd:$rs2)]>;
//   let hasDelaySlot = 1 in
//     def JMPn : F_JMP<0b11000, 1, (outs), (ins GPROpnd:$rs2),
//                      "jmp.n $rs2", [(brind GPROpnd:$rs2)]>;
// }

// let isCall = 1, isTerminator = 1, isBarrier = 1 in {
//   def JSR : F_JMP<0b11001, 0, (outs), (ins GPROpnd:$rs2),
//                   "jsr $rs2", []>;
//   let hasDelaySlot = 1 in
//     def JSRn : F_JMP<0b11001, 1, (outs), (ins GPROpnd:$rs2),
//                      "jsr.n $rs2", []>;
// }

// let isReturn = 1, isTerminator = 1, isBarrier = 1, Uses = [R1] in {
//   def RET : Pseudo<(outs), (ins), [(retflag)]>;
// }

// let isBranch = 1, isTerminator = 1, isBarrier = 1 in {
//   def BR : F_BRANCH<0b11000, 0, (outs), (ins brtarget26:$d26), "br $d26",
//                     [(br bb:$d26)]>;

//   let hasDelaySlot = 1 in
//     def BRn : F_BRANCH<0b11000, 1, (outs), (ins brtarget26:$d26), "br.n $d26",
//                        [(br bb:$d26)]>;
// }

// //
// def : InstAlias<"nop", (ORrr R0, R0, R0)>;