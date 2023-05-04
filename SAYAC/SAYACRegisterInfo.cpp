//===-- SAYACRegisterInfo.cpp - SAYAC Register Information ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the SAYAC implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "SAYACRegisterInfo.h"
#include "SAYAC.h"
//#include "SAYACMachineFunctionInfo.h"
#include "SAYACSubtarget.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h" 

using namespace llvm;

#define GET_REGINFO_TARGET_DESC
#include "SAYACGenRegisterInfo.inc"

SAYACRegisterInfo::SAYACRegisterInfo() : SAYACGenRegisterInfo(SAYAC::R1) {}

const MCPhysReg *
SAYACRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_SAYAC_SaveList;
}

BitVector SAYACRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());

  // R2 is the stack pointer.
  markSuperRegs(Reserved, SAYAC::R2);

  // R0 is wired to zero
  markSuperRegs(Reserved, SAYAC::R0);

  // R15 stores flag bits
  markSuperRegs(Reserved, SAYAC::R15);

  // R1 stores return address
  markSuperRegs(Reserved, SAYAC::R1);

  return Reserved;
}

bool SAYACRegisterInfo::isConstantPhysReg(MCRegister PhysReg) const {
  return PhysReg == SAYAC::R0;
}

void SAYACRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                           int SPAdj, unsigned FIOperandNum,
                                           RegScavenger *RS) const {
  MachineInstr &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();
  const MachineFrameInfo MFI = MF.getFrameInfo();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  const SAYACInstrInfo *TII = MF.getSubtarget<SAYACSubtarget>().getInstrInfo();
  MachineOperand &FIOp = MI.getOperand(FIOperandNum);

  unsigned FI = FIOp.getIndex();

  MachineBasicBlock &MBB = *MI.getParent();
  DebugLoc DL = MI.getDebugLoc();

  Register BaseReg = getFrameRegister(MF);

  // FIXME: check the size of offset.
  int Offset = MFI.getObjectOffset(FI) ;


  // Determine if we can eliminate the index from this kind of
  // instruction
  switch (MI.getOpcode()) {
  default:
    // Not yet supported
    return;
  case SAYAC::LDR:
  case SAYAC::STR:
    break;
  case SAYAC::FI:
  {
    int localOffset = MI.getOperand(2).getImm();
    Offset += localOffset; // This gives offset starting from fp.

    Register destReg = MI.getOperand(0).getReg();
    TII->movImm(MBB, II, DL, destReg, Offset);
    BuildMI(MBB, II, DL, TII->get(SAYAC::ADDrr), destReg)
        .addReg(BaseReg)
        .addReg(destReg);

    MBB.erase(MI);
    return;
  }
  case SAYAC::PSTR:
  case SAYAC::PLDR:
  {
    Offset = Offset + MFI.getStackSize();
    BaseReg = SAYAC::R2;
    break;
  }
  }

  bool BaseRegIsKill = false;

  

  assert(isInt<16>(Offset) && "Int16 expected");
  // Use a scratch register
  // Modify Offset and BaseReg appropriately
  Register ScratchReg = MRI.createVirtualRegister(&SAYAC::GPRRegClass);
  TII->movImm(MBB, II, DL, ScratchReg, Offset);
  BuildMI(MBB, II, DL, TII->get(SAYAC::ADDrr), ScratchReg)
      .addReg(BaseReg)
      .addReg(ScratchReg, RegState::Kill);

  BaseReg = ScratchReg;
  BaseRegIsKill = true;

  FIOp.ChangeToRegister(BaseReg, false, false, BaseRegIsKill);
}

Register SAYACRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return SAYAC::R3;
}

const uint32_t *SAYACRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                                      CallingConv::ID) const {
  return CSR_SAYAC_RegMask;
}