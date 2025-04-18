// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_INSTRUCTION_H
#define VIA_HAS_HEADER_INSTRUCTION_H

#include "common.h"
#include "opcode.h"

#define VIA_OPERAND_INVALID std::numeric_limits<operand_t>::max()

namespace via {

using operand_t = uint16_t;
using signed_operand_t = int16_t;

struct InstructionData {
  std::string comment = "";
};

struct alignas(8) Instruction {
  IOpCode op = IOpCode::NOP;

  operand_t operand0 = VIA_OPERAND_INVALID;
  operand_t operand1 = VIA_OPERAND_INVALID;
  operand_t operand2 = VIA_OPERAND_INVALID;
};

struct Bytecode {
  Instruction instruct;
  InstructionData meta_data;
};

std::string to_string(const Bytecode&, bool);

} // namespace via

#endif
