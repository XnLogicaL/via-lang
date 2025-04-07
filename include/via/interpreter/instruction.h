//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================

#ifndef VIA_HAS_HEADER_INSTRUCTION_H
#define VIA_HAS_HEADER_INSTRUCTION_H

#include "common.h"
#include "opcode.h"

#define VIA_OPERAND_INVALID std::numeric_limits<operand_t>::max()

namespace via {

using operand_t = uint16_t;
using signed_operand_t = int16_t;

struct instruction_data {
  std::string comment = "";
};

struct alignas(8) instruction {
  opcode op = opcode::NOP;

  operand_t operand0 = VIA_OPERAND_INVALID;
  operand_t operand1 = VIA_OPERAND_INVALID;
  operand_t operand2 = VIA_OPERAND_INVALID;
};

struct bytecode {
  instruction instruct;
  instruction_data meta_data;
};

std::string to_string(const bytecode&, bool);

} // namespace via

#endif
