// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef vl_has_header_instruction_h
#define vl_has_header_instruction_h

#include "common.h"
#include "opcode.h"

#define via_operand_invalid std::numeric_limits<operand_t>::max()

namespace via {

using operand_t = uint16_t;
using signed_operand_t = int16_t;

struct instruction_data {
  std::string comment = "";
};

struct vl_align(8) instruction {
  opcode op = opcode::NOP;

  operand_t operand0 = 0;
  operand_t operand1 = 0;
  operand_t operand2 = 0;
};

struct bytecode {
  instruction instruction;
  instruction_data meta_data;
};

std::string to_string(const bytecode&, bool);

} // namespace via

#endif
