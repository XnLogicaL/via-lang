// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_INSTRUCTION_H
#define VIA_HAS_HEADER_INSTRUCTION_H

#include "common.h"
#include "opcode.h"

#include <utility/bits.h>
#include <utility/color.h>

namespace via {

inline constexpr size_t OPERAND_INVALID = 0xFFFF;

using operand_t = uint16_t;
using signed_operand_t = int16_t;

struct InstructionData {
  std::string comment = "";
};

struct alignas(8) Instruction {
  Opcode op = Opcode::NOP;

  operand_t operand0 = OPERAND_INVALID;
  operand_t operand1 = OPERAND_INVALID;
  operand_t operand2 = OPERAND_INVALID;
};

struct Bytecode {
  Instruction instruct;
  InstructionData meta_data;
};

std::string to_string(const Bytecode&, bool);

} // namespace via

#endif
