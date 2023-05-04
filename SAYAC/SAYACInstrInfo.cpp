//===-- SAYACInstrInfo.cpp - SAYAC instruction information ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the SAYAC implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "SAYACInstrInfo.h"
#include "SAYAC.h"
#include "MCTargetDesc/SAYACMCTargetDesc.h"
//#include "SAYACInstrBuilder.h"
#include "SAYACSubtarget.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/LiveInterval.h"
#include "llvm/CodeGen/LiveIntervals.h"
#include "llvm/CodeGen/LiveVariables.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SlotIndexes.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/Support/BranchProbability.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Target/TargetMachine.h"
#include <cassert>
#include <cstdint>
#include <iterator>

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#define GET_INSTRMAP_INFO
#include "SAYACGenInstrInfo.inc"

#define DEBUG_TYPE "SAYAC-ii"

// Pin the vtable to this file.
void SAYACInstrInfo::anchor() {}

SAYACInstrInfo::SAYACInstrInfo(SAYACSubtarget &STI)
    : SAYACGenInstrInfo(SAYAC::ADJCALLSTACKDOWN, SAYAC::ADJCALLSTACKUP), RI(), STI(STI) {}


void SAYACInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator I,
                                 const DebugLoc &DL, MCRegister DestReg,
                                 MCRegister SrcReg, bool KillSrc) const {
  BuildMI(MBB, I, I->getDebugLoc(), get(SAYAC::ADDrr), DestReg)
      .addReg(SrcReg, getKillRegState(KillSrc))
      .addReg(SAYAC::R0);
}

void SAYACInstrInfo::movImm(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MBBI,
                            const DebugLoc &DL, Register DstReg, uint16_t Val,
                            MachineInstr::MIFlag Flag) const {

  int16_t Hi8 = ((Val + 0x80) >> 8) & 0xFF;
  int16_t Lo8 = SignExtend64<8>(Val);

  if(!Hi8) {
    BuildMI(MBB, MBBI, DL, get(SAYAC::MSI), DstReg)
        .addImm(Lo8)
        .setMIFlag(Flag);
  } else {
    BuildMI(MBB, MBBI, DL, get(SAYAC::MSI), DstReg)
        .addImm(Lo8)
        .setMIFlag(Flag);
    BuildMI(MBB, MBBI, DL, get(SAYAC::MHI), DstReg)
        .addImm(Hi8)
        .setMIFlag(Flag);
  }
}

void SAYACInstrInfo::expandBranch(MachineInstr &MI, unsigned BranchInstr,
                                  bool isUnsignedCmp) const {
  DebugLoc DL = MI.getDebugLoc(); MachineBasicBlock &MBB = *MI.getParent();

    MachineFunction &MF = *MI.getParent()->getParent();
    const MachineFrameInfo MFI = MF.getFrameInfo();

    const unsigned destReg = MI.getOperand(0).getReg();
    const unsigned src1 = MI.getOperand(1).getReg();
    const unsigned src2 = MI.getOperand(2).getReg();


    if(isUnsignedCmp){
      // unset SHADOW BIT R15[10]
      BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(-1);
      BuildMI(MBB, MI, DL, get(SAYAC::MHI), destReg).addImm(251);
      BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), SAYAC::R15)
          .addReg(SAYAC::R15)
          .addReg(destReg);
    }
    // else{
    //   // set SHADOW BIT R15[10]
    //   BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(0);
    //   BuildMI(MBB, MI, DL, get(SAYAC::MHI), destReg).addImm(4);
    //   BuildMI(MBB, MI, DL, get(SAYAC::ORrr), SAYAC::R15)
    //       .addReg(SAYAC::R15)
    //       .addReg(destReg);
    // }

    BuildMI(MBB, MI, DL, get(SAYAC::CMR)).addReg(src1).addReg(src2);

    if(isUnsignedCmp){
      // set SHADOW BIT R15[10]
      // BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(0);
      // BuildMI(MBB, MI, DL, get(SAYAC::MHI), destReg).addImm(4);
      // BuildMI(MBB, MI, DL, get(SAYAC::ORrr), SAYAC::R15)
      //     .addReg(SAYAC::R15)
      //     .addReg(destReg);
      BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(-1);
      BuildMI(MBB, MI, DL, get(SAYAC::MHI), destReg).addImm(251);
      BuildMI(MBB, MI, DL, get(SAYAC::NTD2c), SAYAC::R15).addReg(SAYAC::R15);
      // BuildMI(MBB, MI, DL, get(SAYAC::NTD2c), destReg).addReg(destReg);
      BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), SAYAC::R15)
          .addReg(SAYAC::R15)
          .addReg(destReg);
      BuildMI(MBB, MI, DL, get(SAYAC::NTD2c), SAYAC::R15).addReg(SAYAC::R15);
    }

    // Note the usage of src1 here means the value stored at src1 is erased
    // It is assumed that llvm knows this through the BuildMI function call.

    MachineOperand &MO = MI.getOperand(3);

    BuildMI(MBB, MI, DL, get(SAYAC::MSym), src1)->addOperand(MF, MO);

    BuildMI(MBB, MI, DL, get(BranchInstr)).addReg(src1);

    MBB.erase(MI);
}

bool SAYACInstrInfo::expandPostRAPseudo(MachineInstr &MI) const {
  // dbgs() << "IN expandPostRAPPseudo: " << MI.getOpcode() << '\n';
  switch (MI.getOpcode()) {
  default:
    return false;
  case SAYAC::PJMRS:
  {
    DebugLoc DL = MI.getDebugLoc();
    MachineBasicBlock &MBB = *MI.getParent();

    const unsigned src = MI.getOperand(0).getReg();

    BuildMI(MBB, MI, DL, get(SAYAC::JMRS), SAYAC::R1)
    .addReg(src);

    MBB.erase(MI);
    return true;
  }
  case SAYAC::MOVi16sym:
  {
    DebugLoc DL = MI.getDebugLoc();
    MachineBasicBlock &MBB = *MI.getParent();
    MachineFunction &MF = *MI.getParent()->getParent();

    Register destReg = MI.getOperand(0).getReg();
    MachineOperand MO = MI.getOperand(1);

    BuildMI(MBB, MI, DL, get(SAYAC::MSym), destReg)->addOperand(MF, MO);
    MBB.erase(MI);

    return true; 
  }
  case SAYAC::ORrr:
  {
    /*NOTE:
    if registers other than dest Registers are modified in an instruction, that leads to error
    in some instructions, destReg = srcReg may lead to error
    */
    DebugLoc DL = MI.getDebugLoc();
    MachineBasicBlock &MBB = *MI.getParent();

    const unsigned destReg = MI.getOperand(0).getReg();
    const unsigned destReg2 = MI.getOperand(1).getReg();
    const unsigned src1 = MI.getOperand(2).getReg();
    const unsigned src2 = MI.getOperand(3).getReg();

    BuildMI(MBB, MI, DL, get(SAYAC::NTD2c), src1).addReg(src1); // src1 = destReg2 , hence we can modify
    BuildMI(MBB, MI, DL, get(SAYAC::NTR2c), destReg).addReg(src2);
    BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), destReg)
        .addReg(src1)
        .addReg(destReg);
    BuildMI(MBB, MI, DL, get(SAYAC::NTD2c), destReg).addReg(destReg);

    MBB.erase(MI);

    return true;
  }
  case SAYAC::XORrr:
  {
    DebugLoc DL = MI.getDebugLoc();
    MachineBasicBlock &MBB = *MI.getParent();

    unsigned destReg = MI.getOperand(0).getReg();
    unsigned destReg2 = MI.getOperand(1).getReg();
    const unsigned src1 = MI.getOperand(2).getReg();
    const unsigned src2 = MI.getOperand(3).getReg();

    // addition overflow can lead to wrong result
    BuildMI(MBB, MI, DL, get(SAYAC::ADDrr), destReg)
        .addReg(src1)
        .addReg(src2);
    BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), src1)
        .addReg(src1)
        .addReg(src2);
    BuildMI(MBB, MI, DL, get(SAYAC::SHIl), src1)
        .addReg(src1)
        .addImm(-1);
    BuildMI(MBB, MI, DL, get(SAYAC::SUBrr), destReg)
        .addReg(destReg)
        .addReg(src1);

    MBB.erase(MI);

    return true;
  }
  case SAYAC::PJMR:
  {
    DebugLoc DL = MI.getDebugLoc();
    MachineBasicBlock &MBB = *MI.getParent();

    MachineFunction &MF = *MI.getParent()->getParent();

    unsigned rd = MI.getOperand(0).getReg();
    
    // This operand contains the target location to jump to
    MachineOperand &MO = MI.getOperand(1);

    BuildMI(MBB, MI, DL, get(SAYAC::MSym), rd)->addOperand(MF, MO);
    BuildMI(MBB, MI, DL, get(SAYAC::JMR)).addReg(rd);

    MBB.erase(MI);
    return true;
  }
  case SAYAC::SEQ:
  {
    DebugLoc DL = MI.getDebugLoc();
    MachineBasicBlock &MBB = *MI.getParent();

    const unsigned destReg = MI.getOperand(0).getReg();
    const unsigned src1 = MI.getOperand(1).getReg();
    const unsigned src2 = MI.getOperand(2).getReg();

    // unset SHADOW BIT R15[10]
    // BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(-1);
    // BuildMI(MBB, MI, DL, get(SAYAC::MHI), destReg).addImm(251);
    // BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), SAYAC::R15)
    //     .addReg(SAYAC::R15)
    //     .addReg(destReg);

    BuildMI(MBB, MI, DL, get(SAYAC::CMR)).addReg(src1).addReg(src2);

    BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(16);
    BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), destReg)
        .addReg(SAYAC::R15)
        .addReg(destReg);
    BuildMI(MBB, MI, DL, get(SAYAC::SHIl), destReg).addReg(destReg).addImm(4);

    MBB.erase(MI);

    return true;
  }
  case SAYAC::SNE:
  {
    DebugLoc DL = MI.getDebugLoc();
    MachineBasicBlock &MBB = *MI.getParent();

    const unsigned destReg = MI.getOperand(0).getReg();
    const unsigned src1 = MI.getOperand(1).getReg();
    const unsigned src2 = MI.getOperand(2).getReg();

    // unset SHADOW BIT R15[10]
    // BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(-1);
    // BuildMI(MBB, MI, DL, get(SAYAC::MHI), destReg).addImm(251);
    // BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), SAYAC::R15)
    //     .addReg(SAYAC::R15)
    //     .addReg(destReg);

    BuildMI(MBB, MI, DL, get(SAYAC::CMR)).addReg(src1).addReg(src2);

    BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(16);
    BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), destReg)
        .addReg(SAYAC::R15)
        .addReg(destReg);
    BuildMI(MBB, MI, DL, get(SAYAC::SHIl), destReg).addReg(destReg).addImm(4);

    // negate the boolean output ( 1 <-> 0)
    BuildMI(MBB, MI, DL, get(SAYAC::SUBrr), destReg)
        .addReg(SAYAC::R0)
        .addReg(destReg);
    BuildMI(MBB, MI, DL, get(SAYAC::ADDri), destReg)
        .addReg(destReg)
        .addImm(1);


    MBB.erase(MI);

    return true;
  }
  case SAYAC::SUGT:
  {
    DebugLoc DL = MI.getDebugLoc();
    MachineBasicBlock &MBB = *MI.getParent();

    const unsigned destReg = MI.getOperand(0).getReg();
    const unsigned src1 = MI.getOperand(1).getReg();
    const unsigned src2 = MI.getOperand(2).getReg();

    // unset SHADOW BIT R15[10]
    BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(-1);
    BuildMI(MBB, MI, DL, get(SAYAC::MHI), destReg).addImm(251);
    BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), SAYAC::R15)
        .addReg(SAYAC::R15)
        .addReg(destReg);

    BuildMI(MBB, MI, DL, get(SAYAC::CMR)).addReg(src1).addReg(src2);

    // set SHADOW BIT R15[10]
    // BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(0);
    // BuildMI(MBB, MI, DL, get(SAYAC::MHI), destReg).addImm(4);
    // BuildMI(MBB, MI, DL, get(SAYAC::ORrr), SAYAC::R15)
    //     .addReg(SAYAC::R15)
    //     .addReg(destReg);
    BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(-1);
    BuildMI(MBB, MI, DL, get(SAYAC::MHI), destReg).addImm(251);
    BuildMI(MBB, MI, DL, get(SAYAC::NTD2c), SAYAC::R15).addReg(SAYAC::R15);
    // BuildMI(MBB, MI, DL, get(SAYAC::NTD2c), destReg).addReg(destReg);
    BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), SAYAC::R15)
          .addReg(SAYAC::R15)
          .addReg(destReg);
    BuildMI(MBB, MI, DL, get(SAYAC::NTD2c), SAYAC::R15).addReg(SAYAC::R15);

    BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(32);
    BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), destReg)
        .addReg(SAYAC::R15)
        .addReg(destReg);
    BuildMI(MBB, MI, DL, get(SAYAC::SHIl), destReg).addReg(destReg).addImm(5);

    MBB.erase(MI);

    return true;
  }
   case SAYAC::SUGE:
  {
    DebugLoc DL = MI.getDebugLoc();
    MachineBasicBlock &MBB = *MI.getParent();

    const unsigned destReg = MI.getOperand(0).getReg();
    const unsigned src1 = MI.getOperand(1).getReg();
    const unsigned src2 = MI.getOperand(2).getReg();

    // unset SHADOW BIT R15[10]
    BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(-1);
    BuildMI(MBB, MI, DL, get(SAYAC::MHI), destReg).addImm(251);
    BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), SAYAC::R15)
        .addReg(SAYAC::R15)
        .addReg(destReg);

    BuildMI(MBB, MI, DL, get(SAYAC::CMR)).addReg(src1).addReg(src2);

    // set SHADOW BIT R15[10]
    // BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(0);
    // BuildMI(MBB, MI, DL, get(SAYAC::MHI), destReg).addImm(4);
    // BuildMI(MBB, MI, DL, get(SAYAC::ORrr), SAYAC::R15)
    //     .addReg(SAYAC::R15)
    //     .addReg(destReg);
    BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(-1);
    BuildMI(MBB, MI, DL, get(SAYAC::MHI), destReg).addImm(251);
    BuildMI(MBB, MI, DL, get(SAYAC::NTD2c), SAYAC::R15).addReg(SAYAC::R15);
    // BuildMI(MBB, MI, DL, get(SAYAC::NTD2c), destReg).addReg(destReg);
    BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), SAYAC::R15)
          .addReg(SAYAC::R15)
          .addReg(destReg);
    BuildMI(MBB, MI, DL, get(SAYAC::NTD2c), SAYAC::R15).addReg(SAYAC::R15);


    BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(48);
    BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), destReg)
        .addReg(SAYAC::R15)
        .addReg(destReg);
    
    BuildMI(MBB, MI, DL, get(SAYAC::CMR)).addReg(destReg).addReg(SAYAC::R0);

    BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(32);
    BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), destReg)
        .addReg(SAYAC::R15)
        .addReg(destReg);
    BuildMI(MBB, MI, DL, get(SAYAC::SHIl), destReg).addReg(destReg).addImm(5);

    MBB.erase(MI);

    return true;
  }
   case SAYAC::SULT:
  {
    DebugLoc DL = MI.getDebugLoc();
    MachineBasicBlock &MBB = *MI.getParent();

    const unsigned destReg = MI.getOperand(0).getReg();
    const unsigned src1 = MI.getOperand(1).getReg();
    const unsigned src2 = MI.getOperand(2).getReg();

    // unset SHADOW BIT R15[10]
    BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(-1);
    BuildMI(MBB, MI, DL, get(SAYAC::MHI), destReg).addImm(251);
    BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), SAYAC::R15)
        .addReg(SAYAC::R15)
        .addReg(destReg);

    BuildMI(MBB, MI, DL, get(SAYAC::CMR)).addReg(src1).addReg(src2);

    // set SHADOW BIT R15[10]
    // BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(0);
    // BuildMI(MBB, MI, DL, get(SAYAC::MHI), destReg).addImm(4);
    // BuildMI(MBB, MI, DL, get(SAYAC::ORrr), SAYAC::R15)
    //     .addReg(SAYAC::R15)
    //     .addReg(destReg);
    BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(-1);
    BuildMI(MBB, MI, DL, get(SAYAC::MHI), destReg).addImm(251);
    BuildMI(MBB, MI, DL, get(SAYAC::NTD2c), SAYAC::R15).addReg(SAYAC::R15);
    // BuildMI(MBB, MI, DL, get(SAYAC::NTD2c), destReg).addReg(destReg);
    BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), SAYAC::R15)
          .addReg(SAYAC::R15)
          .addReg(destReg);
    BuildMI(MBB, MI, DL, get(SAYAC::NTD2c), SAYAC::R15).addReg(SAYAC::R15);

    

    BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(48);
    BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), destReg)
        .addReg(SAYAC::R15)
        .addReg(destReg);
    
    BuildMI(MBB, MI, DL, get(SAYAC::CMR)).addReg(destReg).addReg(SAYAC::R0);

    BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(16);
    BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), destReg)
        .addReg(SAYAC::R15)
        .addReg(destReg);
    BuildMI(MBB, MI, DL, get(SAYAC::SHIl), destReg).addReg(destReg).addImm(4);

    MBB.erase(MI);

    return true;
  }
  case SAYAC::SULE:
  {
    DebugLoc DL = MI.getDebugLoc();
    MachineBasicBlock &MBB = *MI.getParent();

    const unsigned destReg = MI.getOperand(0).getReg();
    const unsigned src1 = MI.getOperand(1).getReg();
    const unsigned src2 = MI.getOperand(2).getReg();

    // unset SHADOW BIT R15[10]
    BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(-1);
    BuildMI(MBB, MI, DL, get(SAYAC::MHI), destReg).addImm(251);
    BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), SAYAC::R15)
        .addReg(SAYAC::R15)
        .addReg(destReg);

    BuildMI(MBB, MI, DL, get(SAYAC::CMR)).addReg(src1).addReg(src2);

    // set SHADOW BIT R15[10]
    // BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(0);
    // BuildMI(MBB, MI, DL, get(SAYAC::MHI), destReg).addImm(4);
    // BuildMI(MBB, MI, DL, get(SAYAC::ORrr), SAYAC::R15)
    //     .addReg(SAYAC::R15)
    //     .addReg(destReg);
    BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(-1);
    BuildMI(MBB, MI, DL, get(SAYAC::MHI), destReg).addImm(251);
    BuildMI(MBB, MI, DL, get(SAYAC::NTD2c), SAYAC::R15).addReg(SAYAC::R15);
    // BuildMI(MBB, MI, DL, get(SAYAC::NTD2c), destReg).addReg(destReg);
    BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), SAYAC::R15)
          .addReg(SAYAC::R15)
          .addReg(destReg);
    BuildMI(MBB, MI, DL, get(SAYAC::NTD2c), SAYAC::R15).addReg(SAYAC::R15);


    BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(32);
    BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), destReg)
        .addReg(SAYAC::R15)
        .addReg(destReg);
    BuildMI(MBB, MI, DL, get(SAYAC::SHIl), destReg).addReg(destReg).addImm(5);

    // negate the boolean output ( 1 <-> 0)
    BuildMI(MBB, MI, DL, get(SAYAC::SUBrr), destReg)
        .addReg(SAYAC::R0)
        .addReg(destReg);
    BuildMI(MBB, MI, DL, get(SAYAC::ADDri), destReg)
        .addReg(destReg)
        .addImm(1);

    MBB.erase(MI);

    return true;
  }
  case SAYAC::SGT:
  {
    DebugLoc DL = MI.getDebugLoc();
    MachineBasicBlock &MBB = *MI.getParent();

    const unsigned destReg = MI.getOperand(0).getReg();
    const unsigned src1 = MI.getOperand(1).getReg();
    const unsigned src2 = MI.getOperand(2).getReg();

    // Assuming SHADOW BIT for Compare - R15[10] is set initially
    // So when unsigned comparison needs to be used, this bit is unset before comparison
    // and then again set again after compare instruction

    // set SHADOW BIT R15[10]
    // BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(0);
    // BuildMI(MBB, MI, DL, get(SAYAC::MHI), destReg).addImm(4);
    // BuildMI(MBB, MI, DL, get(SAYAC::ORrr), SAYAC::R15)
    //     .addReg(SAYAC::R15)
    //     .addReg(destReg);


    BuildMI(MBB, MI, DL, get(SAYAC::CMR)).addReg(src1).addReg(src2);

    BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(32);
    BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), destReg)
        .addReg(SAYAC::R15)
        .addReg(destReg);
    BuildMI(MBB, MI, DL, get(SAYAC::SHIl), destReg).addReg(destReg).addImm(5);

    // unset SHADOW BIT R15[10]
    // BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(-1);
    // BuildMI(MBB, MI, DL, get(SAYAC::MHI), destReg).addImm(255);
    // BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), SAYAC::R15)
    //     .addReg(SAYAC::R15)
    //     .addReg(destReg);

    MBB.erase(MI);

    return true;
  }
   case SAYAC::SGE:
  {
    DebugLoc DL = MI.getDebugLoc();
    MachineBasicBlock &MBB = *MI.getParent();

    const unsigned destReg = MI.getOperand(0).getReg();
    const unsigned src1 = MI.getOperand(1).getReg();
    const unsigned src2 = MI.getOperand(2).getReg();

    // set SHADOW BIT R15[10]
    // BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(0);
    // BuildMI(MBB, MI, DL, get(SAYAC::MHI), destReg).addImm(4);
    // BuildMI(MBB, MI, DL, get(SAYAC::ORrr), SAYAC::R15)
    //     .addReg(SAYAC::R15)
    //     .addReg(destReg);

    BuildMI(MBB, MI, DL, get(SAYAC::CMR)).addReg(src1).addReg(src2);

    BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(48);
    BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), destReg)
        .addReg(SAYAC::R15)
        .addReg(destReg);
    
    BuildMI(MBB, MI, DL, get(SAYAC::CMR)).addReg(destReg).addReg(SAYAC::R0);

    BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(32);
    BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), destReg)
        .addReg(SAYAC::R15)
        .addReg(destReg);
    BuildMI(MBB, MI, DL, get(SAYAC::SHIl), destReg).addReg(destReg).addImm(5);

    // unset SHADOW BIT R15[10]
    // BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(-1);
    // BuildMI(MBB, MI, DL, get(SAYAC::MHI), destReg).addImm();
    // BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), SAYAC::R15)
    //     .addReg(SAYAC::R15)
    //     .addReg(destReg);

    MBB.erase(MI);

    return true;
  }
   case SAYAC::SLT:
  {
    DebugLoc DL = MI.getDebugLoc();
    MachineBasicBlock &MBB = *MI.getParent();

    const unsigned destReg = MI.getOperand(0).getReg();
    const unsigned src1 = MI.getOperand(1).getReg();
    const unsigned src2 = MI.getOperand(2).getReg();

    // set SHADOW BIT R15[10]
    // BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(0);
    // BuildMI(MBB, MI, DL, get(SAYAC::MHI), destReg).addImm(4);
    // BuildMI(MBB, MI, DL, get(SAYAC::ORrr), SAYAC::R15)
    //     .addReg(SAYAC::R15)
    //     .addReg(destReg);

    BuildMI(MBB, MI, DL, get(SAYAC::CMR)).addReg(src1).addReg(src2);

    BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(48);
    BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), destReg)
        .addReg(SAYAC::R15)
        .addReg(destReg);
    
    BuildMI(MBB, MI, DL, get(SAYAC::CMR)).addReg(destReg).addReg(SAYAC::R0);

    BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(16);
    BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), destReg)
        .addReg(SAYAC::R15)
        .addReg(destReg);
    BuildMI(MBB, MI, DL, get(SAYAC::SHIl), destReg).addReg(destReg).addImm(4);

    // unset SHADOW BIT R15[10]
    // BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(-1);
    // BuildMI(MBB, MI, DL, get(SAYAC::MHI), destReg).addImm(251);
    // BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), SAYAC::R15)
    //     .addReg(SAYAC::R15)
    //     .addReg(destReg);

    MBB.erase(MI);

    return true;
  }
  case SAYAC::SLE:
  {
    DebugLoc DL = MI.getDebugLoc();
    MachineBasicBlock &MBB = *MI.getParent();

    const unsigned destReg = MI.getOperand(0).getReg();
    const unsigned src1 = MI.getOperand(1).getReg();
    const unsigned src2 = MI.getOperand(2).getReg();

    // set SHADOW BIT R15[10]
    // BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(0);
    // BuildMI(MBB, MI, DL, get(SAYAC::MHI), destReg).addImm(4);
    // BuildMI(MBB, MI, DL, get(SAYAC::ORrr), SAYAC::R15)
    //     .addReg(SAYAC::R15)
    //     .addReg(destReg);

    BuildMI(MBB, MI, DL, get(SAYAC::CMR)).addReg(src1).addReg(src2);

    BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(32);
    BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), destReg)
        .addReg(SAYAC::R15)
        .addReg(destReg);
    BuildMI(MBB, MI, DL, get(SAYAC::SHIl), destReg).addReg(destReg).addImm(5);

    // negate the boolean output ( 1 <-> 0)
    BuildMI(MBB, MI, DL, get(SAYAC::SUBrr), destReg)
        .addReg(SAYAC::R0)
        .addReg(destReg);
    BuildMI(MBB, MI, DL, get(SAYAC::ADDri), destReg)
        .addReg(destReg)
        .addImm(1);

    // unset SHADOW BIT R15[10]
    // BuildMI(MBB, MI, DL, get(SAYAC::MSI), destReg).addImm(-1);
    // BuildMI(MBB, MI, DL, get(SAYAC::MHI), destReg).addImm(251);
    // BuildMI(MBB, MI, DL, get(SAYAC::ANDrr), SAYAC::R15)
    //     .addReg(SAYAC::R15)
    //     .addReg(destReg);

    MBB.erase(MI);

    return true;
  }
  case SAYAC::CBeq:{
    expandBranch(MI, SAYAC::BRCeq, false);
    return true;
  }
  case SAYAC::CBne:{
    expandBranch(MI, SAYAC::BRCne, false);
    return true;
  }
  case SAYAC::CBugt:{
    expandBranch(MI, SAYAC::BRCgt, true);
    return true;
  }
  case SAYAC::CBuge:{
    expandBranch(MI, SAYAC::BRCge, true);
    return true;
  }
  case SAYAC::CBult:{
    expandBranch(MI, SAYAC::BRClt, true);
    return true;
  }
  case SAYAC::CBule:{
    expandBranch(MI, SAYAC::BRCle, true);
    return true;
  }
  case SAYAC::CBgt:{
    expandBranch(MI, SAYAC::BRCgt, false);
    return true;
  }
  case SAYAC::CBge:{
    expandBranch(MI, SAYAC::BRCge, false);
    return true;
  }
  case SAYAC::CBlt:{
    expandBranch(MI, SAYAC::BRClt, false);
    return true;
  }
  case SAYAC::CBle:{
    expandBranch(MI, SAYAC::BRCle, false);
    return true;
  }

    //   case SAYAC::CmpBranch:
    //     DebugLoc DL = MI.getDebugLoc();
    //     MachineBasicBlock &MBB = *MI.getParent();

        // MachineFunction &MF = *MI.getParent()->getParent();
        // const MachineFrameInfo MFI = MF.getFrameInfo();
        // MachineRegisterInfo &MRI = MF.getRegInfo();
        // const SAYACInstrInfo *TII =
        //     MF.getSubtarget<SAYACSubtarget>().getInstrInfo();

        // const unsigned src1 = MI.getOperand(0).getReg();
        // const unsigned src2 = MI.getOperand(1).getReg();
        // const unsigned branch_offset = MI.getOperand(2).getImm();

        // BuildMI(MBB, MI, DL, get(SAYAC::CMR)).addReg(src1).addReg(src2);

        // Register ScratchReg = MRI.createVirtualRegister(&SAYAC::GPRRegClass);
        // TII->movImm(MBB, MI, DL, ScratchReg, branch_offset);

        // BuildMI(MBB, MI, DL, get(SAYAC::BRne)).addReg(ScratchReg);

        // MBB.erase(MI);
        // return true;
  }
}

void SAYACInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                       MachineBasicBlock::iterator I,
                                       Register SrcReg, bool isKill,
                                       int FrameIndex,
                                       const TargetRegisterClass *RC,
                                       const TargetRegisterInfo *TRI) const {
  BuildMI(MBB, I, I->getDebugLoc(), get(SAYAC::PSTR))
      .addFrameIndex(FrameIndex)
      .addReg(SrcReg, getKillRegState(isKill));
}

void SAYACInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                          MachineBasicBlock::iterator I,
                                          Register DestReg, int FrameIndex,
                                          const TargetRegisterClass *RC,
                                          const TargetRegisterInfo *TRI) const {
  BuildMI(MBB, I, I->getDebugLoc(), get(SAYAC::PLDR), DestReg)
      .addFrameIndex(FrameIndex);
}