//===-- SAYACISelDAGToDAG.cpp - A dag to dag inst selector for SAYAC --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines an instruction selector for the SAYAC target.
//
//===----------------------------------------------------------------------===//

#include "SAYACTargetMachine.h"
//#include "SAYACISelLowering.h"
#include "SAYAC.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/KnownBits.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "SAYAC-isel"

namespace {

class SAYACDAGToDAGISel : public SelectionDAGISel {
  const SAYACSubtarget *Subtarget;

  const SAYACTargetMachine &getTargetMachine() const {
    return static_cast<const SAYACTargetMachine &>(TM);
  }

  //  const SAYACInstrInfo *getInstrInfo() const {
  //    return Subtarget->getInstrInfo();
  //  }

public:
  SAYACDAGToDAGISel(SAYACTargetMachine &TM, CodeGenOpt::Level OptLevel)
      : SelectionDAGISel(TM, OptLevel) {}

/*
  bool runOnMachineFunction(MachineFunction &MF) override {
    Subtarget = &MF.getSubtarget<SAYACSubtarget>();
    return SelectionDAGISel::runOnMachineFunction(MF);
  }
*/
  // Override MachineFunctionPass.
  StringRef getPassName() const override {
    return "SAYAC DAG->DAG Pattern Instruction Selection";
  }

  // Override SelectionDAGISel.
  void Select(SDNode *Node) override;
/*
bool SelectInlineAsmMemoryOperand(const SDValue &Op, unsigned ConstraintID,
                                  std::vector<SDValue> &OutOps) override;
bool IsProfitableToFold(SDValue N, SDNode *U, SDNode *Root) const override;
void PreprocessISelDAG() override;
*/

// Include the pieces autogenerated from the target description.
#include "SAYACGenDAGISel.inc"
};
} // end anonymous namespace

FunctionPass *llvm::createSAYACISelDag(SAYACTargetMachine &TM,
                                      CodeGenOpt::Level OptLevel) {
  return new SAYACDAGToDAGISel(TM, OptLevel);
}

void SAYACDAGToDAGISel::Select(SDNode *Node) {
  // Instruction Selection not handled by the auto-generated tablegen selection
  // should be handled here.

  // Select the default instruction.
  SelectCode(Node);
}