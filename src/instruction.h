// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

/**
 * @file instruction.h
 * @brief Defines the representation of bytecode instructions in the Via VM.
 *
 * This file provides the core structures and type definitions used to represent
 * low-level bytecode instructions that the virtual machine executes. It includes
 * operand types, the main `Instruction` structure, and associated metadata for debugging.
 */
#ifndef VIA_HAS_HEADER_INSTRUCTION_H
#define VIA_HAS_HEADER_INSTRUCTION_H

#include "common.h"
#include "opcode.h"

#include <bits.h>
#include <color.h>

/**
 * @namespace via
 * @ingroup via_namespace
 * @{
 */
namespace via {

/**
 * @brief Sentinel value used to represent an invalid or unused operand.
 */
inline constexpr size_t OPERAND_INVALID = 0xFFFF;

/**
 * @brief Unsigned 16-bit operand type used by the instruction format.
 */
using operand_t = uint16_t;

/**
 * @brief Signed version of `operand_t`, used for relative or signed operands.
 */
using signed_operand_t = int16_t;

/**
 * @struct InstructionData
 * @brief Optional debug metadata associated with a single instruction.
 *
 * Currently only includes a comment string, but may be extended in the future
 * to support source mapping, line numbers, or debugging flags.
 */
struct InstructionData {
  std::string comment = ""; ///< Human-readable comment or annotation.
};

/**
 * @struct Instruction
 * @brief Represents a single VM instruction in the Via bytecode format.
 *
 * Each instruction has:
 * - An opcode (`op`) that specifies the operation to perform.
 * - Up to three 16-bit operands (`a`, `b`, and `c`), whose semantics depend on the opcode.
 *
 * The structure is aligned to 8 bytes to optimize for memory layout and access efficiency.
 */
struct alignas(8) Instruction {
  Opcode op = Opcode::NOP;       ///< Operation code (e.g., ADD, LOAD, CALL).
  operand_t a = OPERAND_INVALID; ///< First operand (typically a register or constant index).
  operand_t b = OPERAND_INVALID; ///< Second operand.
  operand_t c = OPERAND_INVALID; ///< Third operand.
};

/**
 * @brief Converts a `Bytecode` instruction into a string representation.
 *
 * @param bytecode The bytecode instruction to stringify.
 * @param include_metadata If true, include metadata (such as comments) in the output.
 * @return A human-readable string representation of the instruction.
 */
std::string to_string(const Instruction& insn, const InstructionData& data, bool cap_opcodes);

} // namespace via

/** @} */

#endif
