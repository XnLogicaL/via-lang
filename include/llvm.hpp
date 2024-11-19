/* This file is a shortcut for the LLVM library/framework/whatever */

#pragma once

// LLVM Core and Support
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/FileSystem.h>

// LLVM IR and Module
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>

// LLVM Target and Machine Setup
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Target/TargetRegistry.h>
#include <llvm/Target/Target.h>
#include <llvm/Target/TargetSubtargetInfo.h>
#include <llvm/Target/TargetInstrInfo.h>

// LLVM JIT Compilation
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeContext.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/IndirectStubsManager.h>
#include <llvm/ExecutionEngine/Orc/OrcJIT.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/ExecutionEngine/Orc/RTDyldMemoryManager.h>

// LLVM Pass Management
#include <llvm/PassManager.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Utils.h>

// LLVM Codegen
#include <llvm/CodeGen/Passes.h>
#include <llvm/CodeGen/MachineFunction.h>
#include <llvm/CodeGen/SelectionDAG.h>
#include <llvm/CodeGen/TargetLowering.h>

// LLVM JIT Symbol Resolution
#include <llvm/ExecutionEngine/Orc/JITDylib.h>
#include <llvm/ExecutionEngine/Orc/MaterializationUnit.h>
#include <llvm/ExecutionEngine/Orc/BasicJITLayer.h>

// LLVM Linker
#include <llvm/Linker/Linker.h>
