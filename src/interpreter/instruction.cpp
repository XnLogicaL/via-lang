// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "instruction.h"
#include "bit-utility.h"
#include "bytecode.h"
#include "object.h"
#include "color.h"

namespace via {

using namespace utils;
using enum opcode;

std::string to_string(const bytecode& bc, bool capitalize_opcodes) {
  const auto& instr = bc.instruct;
  const auto& meta = bc.meta_data;
  const opcode op = instr.op;
  const uint32_t op_id = static_cast<uint32_t>(op);

  // Helper to get a properly formatted opcode string
  auto get_opcode_str = [&]() -> std::string {
    std::string str(magic_enum::enum_name(op));
    std::transform(str.begin(), str.end(), str.begin(), [capitalize_opcodes](unsigned char c) {
      return capitalize_opcodes ? std::toupper(c) : std::tolower(c);
    });
    return str;
  };

  const std::string opcode_string = apply_color(
    std::format("{:<12}", get_opcode_str()), fg_color::magenta, bg_color::black, style::bold
  );

  // Format comment only if it's non-empty
  const std::string comment_string = meta.comment.empty()
    ? ""
    : apply_color(
        std::format("; {:<15}", meta.comment), fg_color::green, bg_color::black, style::italic
      );

  std::string arg0, arg1, arg2;

  // Function to apply "dim" style to zero operands
  auto dim_operand = [](const std::string& operand) -> std::string {
    return operand == "0" || operand == "0,"
      ? apply_color(operand, fg_color::white, bg_color::black, style::faint)
      : operand;
  };

  // Handle specific opcode groups with custom formatting
  if (op_id >= static_cast<uint32_t>(JUMP)
      && op_id <= static_cast<uint32_t>(JUMPIFGREATEROREQUAL)) {
    arg0 = dim_operand(std::format("{},", static_cast<signed_operand_t>(instr.operand0)));
    arg1 = dim_operand(std::format("{},", static_cast<signed_operand_t>(instr.operand1)));
    arg2 = dim_operand(std::format("{}", static_cast<signed_operand_t>(instr.operand2)));
  }
  else if (op == LOADINT || op == ADDINT || op == SUBINT || op == MULINT || op == DIVINT
           || op == POWINT || op == MODINT) {
    arg0 = dim_operand(std::format("{},", static_cast<operand_t>(instr.operand0)));
    arg1 = dim_operand(std::format(
      "{}", static_cast<TInteger>(reinterpret_u16_as_i32(instr.operand1, instr.operand2))
    ));
    arg2 = "";
  }
  else if (op == LOADFLOAT || op == ADDFLOAT || op == SUBFLOAT || op == MULFLOAT || op == DIVFLOAT
           || op == POWFLOAT || op == MODFLOAT) {
    arg0 = dim_operand(std::format("{},", static_cast<operand_t>(instr.operand0)));
    arg1 = dim_operand(
      std::format("{}", static_cast<TFloat>(reinterpret_u16_as_f32(instr.operand1, instr.operand2)))
    );
    arg2 = "";
  }
  else {
    arg0 = dim_operand(std::format("{},", instr.operand0));
    arg1 = dim_operand(std::format("{},", instr.operand1));
    arg2 = dim_operand(std::format("{}", instr.operand2));
  }

  std::string main_line = opcode_string + std::format("{} {} {}", arg0, arg1, arg2);

  // Append comment if it exists
  return comment_string.empty() ? main_line
                                : main_line + std::format("{:>5}", ' ') + comment_string;
}

} // namespace via
