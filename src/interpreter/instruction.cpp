//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================
#include "instruction.h"
#include "bit-utility.h"
#include "bytecode.h"
#include "object.h"
#include "color.h"

namespace via {

using namespace utils;
using enum IOpCode;

std::string to_string(const Bytecode& bc, bool capitalize_opcodes) {
  const auto& instr = bc.instruct;
  const auto& meta = bc.meta_data;
  const IOpCode op = instr.op;

  constexpr int opcode_column_width = 12;
  constexpr int operand_column_width = 8;

  auto is_operand_valid = [](const operand_t& operand) { return operand != VIA_OPERAND_INVALID; };

  // Generate raw IOpCode string and transform case
  std::string raw_opcode_str = std::string(magic_enum::enum_name(op));
  std::transform(
    raw_opcode_str.begin(),
    raw_opcode_str.end(),
    raw_opcode_str.begin(),
    [capitalize_opcodes](unsigned char c) {
      return capitalize_opcodes ? std::toupper(c) : std::tolower(c);
    }
  );

  // Pad raw IOpCode string before applying color
  std::ostringstream opcode_buf;
  opcode_buf << std::left << std::setw(opcode_column_width) << raw_opcode_str;
  std::string colored_opcode_str =
    apply_color(opcode_buf.str(), fg_color::magenta, bg_color::black, style::bold);

  // Build operand string
  std::string operand_str;
  if (is_operand_valid(instr.operand0)) {
    if (op == PUSHI) {
      uint32_t result = reinterpret_u16_as_u32(instr.operand0, instr.operand1);
      operand_str += std::to_string(result);
    }
    else {
      operand_str += std::to_string(instr.operand0);

      if (op == ADDI || op == SUBI || op == MULI || op == DIVI || op == POWI || op == MODI
          || op == LOADI) {
        uint32_t result = reinterpret_u16_as_u32(instr.operand1, instr.operand2);
        operand_str += ", " + std::to_string(result);
      }
      else {
        if (is_operand_valid(instr.operand1)) {
          operand_str += ", " + std::to_string(instr.operand1);

          if (is_operand_valid(instr.operand2)) {
            operand_str += ", " + std::to_string(instr.operand2);
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

  if (!meta.comment.empty()) {
    std::string full_comment = std::format(" ; {}", meta.comment);
    oss << apply_color(full_comment, fg_color::green, bg_color::black, style::italic);
  }

  return oss.str();
}

} // namespace via
