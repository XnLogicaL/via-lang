// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "vminstr.h"

namespace via {

using enum Opcode;

std::string to_string(const Instruction& insn, const InstructionData& data, bool cap_opcodes) {
  const Opcode op = insn.op;

  constexpr int opcode_column_width = 12;
  constexpr int operand_column_width = 8;

  auto is_operand_valid = [](const uint16_t& operand) { return operand != OPERAND_INVALID; };

  // Generate raw Opcode String and transform case
  std::string raw_opcode_str = std::string(magic_enum::enum_name(op));
  std::transform(
    raw_opcode_str.begin(),
    raw_opcode_str.end(),
    raw_opcode_str.begin(),
    [cap_opcodes](unsigned char c) { return cap_opcodes ? std::toupper(c) : std::tolower(c); }
  );

  // Pad raw Opcode String before applying color
  std::ostringstream opcode_buf;
  opcode_buf << std::left << std::setw(opcode_column_width) << raw_opcode_str;
  std::string colored_opcode_str =
    apply_color(opcode_buf.str(), fg_color::magenta, bg_color::black, style::bold);

  // Build operand String
  std::string operand_str;
  if (is_operand_valid(insn.a)) {
    if (op == PUSHI) {
      uint32_t result = ubit_2u16tou32(insn.a, insn.b);
      operand_str += std::to_string(result);
    }
    else {
      operand_str += std::to_string(insn.a);

      if (op == ADDI || op == SUBI || op == MULI || op == DIVI || op == POWI || op == MODI || op == LOADI) {
        uint32_t result = ubit_2u16tou32(insn.b, insn.c);
        operand_str += ", " + std::to_string(result);
      }
      else {
        if (is_operand_valid(insn.b)) {
          operand_str += ", " + std::to_string(insn.b);

          if (is_operand_valid(insn.c)) {
            operand_str += ", " + std::to_string(insn.c);
          }
        }
      }
    }
  }

  // Pad operands
  std::ostringstream operand_buf;
  operand_buf << std::left << std::setw(operand_column_width) << operand_str;

  // Final formatting
  std::ostringstream oss;
  oss << colored_opcode_str << ' ' << operand_buf.str();

  if (!data.comment.empty()) {
    std::string full_comment = std::format(" ; {}", data.comment);
    oss << apply_color(full_comment, fg_color::green, bg_color::black, style::italic);
  }

  return oss.str();
}

} // namespace via
