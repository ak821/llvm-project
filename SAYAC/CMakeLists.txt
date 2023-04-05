add_llvm_component_group(SAYAC)

set(LLVM_TARGET_DEFINITIONS SAYAC.td)

tablegen(LLVM SAYACGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM SAYACGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM SAYACGenCallingConv.inc -gen-callingconv)
tablegen(LLVM SAYACGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM SAYACGenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM SAYACGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM SAYACGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM SAYACGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM SAYACGenSubtargetInfo.inc -gen-subtarget)

add_public_tablegen_target(SAYACCommonTableGen)

add_llvm_target(SAYACCodeGen
  SAYACAsmPrinter.cpp
  SAYACFrameLowering.cpp
  SAYACISelDAGToDAG.cpp
  SAYACISelLowering.cpp
  SAYACInstrInfo.cpp
#  SAYACMachineFunctionInfo.cpp
  SAYACMCInstLower.cpp
  SAYACRegisterInfo.cpp
  SAYACSubtarget.cpp
  SAYACTargetMachine.cpp
  )

add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)