// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#include "instruction.h"
#include "bytecode.h"

namespace via {

Instruction::Instruction(OpCode op, std::vector<U32> operands, Chunk *chunk, size_t pos)
    : op(op)
    , operand0(safe_call<U32>([&operands]() { return operands.at(0); }, 0))
    , operand1(safe_call<U32>([&operands]() { return operands.at(1); }, 0))
    , operand2(safe_call<U32>([&operands]() { return operands.at(2); }, 0))
    , chunk(chunk)
    , pos(pos)
{
}

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
