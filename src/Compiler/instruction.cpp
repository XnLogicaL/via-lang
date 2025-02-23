// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GPL v3.           |
// =========================================================================================== |

#include "instruction.h"
#include "bytecode.h"

namespace via {

std::string to_string(ProgramData *prog, Instruction instruction)
{
    std::string comment("");
    auto it = prog->bytecode_info.find(instruction.pos);
    if (it != prog->bytecode_info.end())
        comment = std::format("; {}", it->second);

    return std::format(
        "{:04} {:<12} {:<2} {:<2} {:<2} {:<15} {}",
        instruction.pos,
        magic_enum::enum_name(instruction.op),
        instruction.operand0,
        instruction.operand1,
        instruction.operand2,
        " ",
        comment
    );
}

} // namespace via
