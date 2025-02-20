// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#include "instruction.h"
#include "bytecode.h"

namespace via {

Operand::Operand()
    : type(OperandType::Nil)
{
}

Operand::Operand(TNumber x)
    : type(OperandType::Number)
    , val_number(x)
{
}

Operand::Operand(TBool b)
    : type(OperandType::Bool)
    , val_boolean(b)
{
}

Operand::Operand(const char *str)
    : type(OperandType::String)
    , val_string(str)
{
}

Operand::Operand(U32 reg)
    : type(OperandType::Register)
    , val_register(reg)
{
}

Instruction::Instruction()
    : op(OpCode::NOP)
    , operand0()
    , operand1()
    , operand2()
    , chunk(nullptr)
    , pos(0)
{
}

Instruction::Instruction(OpCode op, std::vector<Operand> operands, Chunk *chunk, size_t pos)
    : op(op)
    // clang-format off
    , operand0(safe_call([&operands](){return operands.at(0);}, Operand()))
    , operand1(safe_call([&operands](){return operands.at(1);}, Operand()))
    , operand2(safe_call([&operands](){return operands.at(2);}, Operand()))
    // clang-format on
    , chunk(chunk)
    , pos(pos)
{
}

std::string to_string(Operand operand)
{
    switch (operand.type) {
    case OperandType::Bool:
        return std::string(operand.val_boolean ? "true" : "false");
    case OperandType::String:
        return std::format("'{}'", operand.val_string);
    case OperandType::Number:
        return std::format("{:03g}", operand.val_number);
    case OperandType::Register:
        return std::format("R{:02}", operand.val_register);
    default:
        return "";
    }
}

std::string to_string(ProgramData *prog, Instruction instruction)
{
    std::string comment("");
    auto it = prog->bytecode_info.find(instruction.pos);
    if (it != prog->bytecode_info.end())
        comment = std::format("; {}", it->second);

    return std::format(
        "{:04} {:<12} {:<3} {:<3} {:<3} {:<15} {}",
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
