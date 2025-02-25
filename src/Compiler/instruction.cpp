// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "instruction.h"
#include "bytecode.h"

namespace via {

std::string to_string(U32 operand)
{
    return operand == VIA_OPERAND_INVALID ? "" : std::format("{:02}", operand);
}

std::string to_string(ProgramData *prog, Instruction instruction)
{
    std::string comment("");
    auto it = prog->bytecode_info.find(instruction.pos);
    if (it != prog->bytecode_info.end()) {
        comment = std::format("; {}", it->second);
    }

    return std::format(
        "{:04} {:<12} {:<2} {:<2} {:<2} {:<15} {}",
        instruction.pos,
        magic_enum::enum_name(instruction.op),
        to_string(instruction.operand0),
        to_string(instruction.operand1),
        to_string(instruction.operand2),
        " ",
        comment
    );
}

} // namespace via
