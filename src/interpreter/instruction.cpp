// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "instruction.h"
#include "bit-utility.h"
#include "bytecode.h"
#include "object.h"

namespace via {

using enum opcode;

std::string to_string(const bytecode& bytecode, bool capitalize_opcodes) {
  // The main instruction is printed with fixed widths:
  // - Opcode string: 12 characters, colored in magenta.
  // - Three operands: each 3 characters wide.
  static constexpr const char* main_format = "\033[0;35m{:<12}\033[0;37m {:<3} {:<3} {:<3}\033[0m";
  // The comment field is formatted as a left-aligned, fixed width of 15 characters in green.
  static constexpr const char* comment_format = "\033[3;32m{:<15}\033[0m";

  const instruction& instruction = bytecode.instruction;
  const instruction_data& data = bytecode.meta_data;

  auto get_opcode_string = [&]() -> std::string {
    std::string str(magic_enum::enum_name(instruction.op));
    std::transform(str.begin(), str.end(), str.begin(), capitalize_opcodes ? ::toupper : ::tolower);
    return str;
  };

  std::string arg0, arg1, arg2;
  opcode opcode = instruction.op;
  uint32_t opcode_id = static_cast<uint32_t>(opcode);

  if (opcode_id >= static_cast<uint32_t>(JUMP) &&
      opcode_id <= static_cast<uint32_t>(JUMPIFGREATEROREQUAL)) {
    arg0 = std::format("{}", static_cast<signed_operand_t>(instruction.operand0));
    arg1 = std::format("{}", static_cast<signed_operand_t>(instruction.operand1));
    arg2 = std::format("{}", static_cast<signed_operand_t>(instruction.operand2));
  }
  else if (opcode == LOADINT || opcode == ADDINT || opcode == SUBINT || opcode == MULINT ||
           opcode == DIVINT || opcode == POWINT || opcode == MODINT) {
    arg0 = std::format("{}", static_cast<operand_t>(instruction.operand0));
    arg1 = std::format(
      "{}",
      static_cast<TInteger>(reinterpret_u16_as_i32(instruction.operand1, instruction.operand2))
    );
    arg2 = "";
  }
  else if (opcode == LOADFLOAT || opcode == ADDFLOAT || opcode == SUBFLOAT || opcode == MULFLOAT ||
           opcode == DIVFLOAT || opcode == POWFLOAT || opcode == MODFLOAT) {
    arg0 = std::format("{}", static_cast<operand_t>(instruction.operand0));
    arg1 = std::format(
      "{}", static_cast<TFloat>(reinterpret_u16_as_f32(instruction.operand1, instruction.operand2))
    );
    arg2 = "";
  }
  else if (opcode == GETGLOBAL || opcode == SETGLOBAL) {
    arg0 = std::format("{}", instruction.operand0);
    arg1 = std::format("{}", reinterpret_u16_as_u32(instruction.operand1, instruction.operand2));
    arg2 = "";
  }
  else {
    arg0 = std::format("{}", instruction.operand0);
    arg1 = std::format("{}", instruction.operand1);
    arg2 = std::format("{}", instruction.operand2);
  }

  std::string main_line = std::format(main_format, get_opcode_string(), arg0, arg1, arg2);
  std::string comment_body = data.comment.empty() ? "" : std::format("; {}", data.comment);
  std::string comment_str = std::format(comment_format, comment_body);

  return main_line + " " + comment_str;
}

} // namespace via
