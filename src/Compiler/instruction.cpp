/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "instruction.h"
#include "bytecode.h"

namespace via
{

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

Operand::Operand(RegId reg)
    : type(OperandType::Register)
    , val_register(reg)
{
}

Instruction::Instruction()
    : op(OpCode::NOP)
    , operand1()
    , operand2()
    , operand3()
    , chunk(nullptr)
    , pos(0)
{
}

Instruction::Instruction(OpCode op, std::vector<Operand> operands, Chunk *chunk, size_t pos)
    : op(op)
    // clang-format off
    , operand1(safe_call([&operands](){return operands.at(0);}, Operand()))
    , operand2(safe_call([&operands](){return operands.at(1);}, Operand()))
    , operand3(safe_call([&operands](){return operands.at(2);}, Operand()))
    // clang-format on
    , chunk(chunk)
    , pos(pos)
{
}

std::string to_string(Operand operand)
{
    switch (operand.type)
    {
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

std::string to_string(ProgramData &prog, Instruction instruction)
{
    std::string comment("");
    auto it = prog.bytecode_info.find(instruction.pos);
    if (it != prog.bytecode_info.end())
        comment = std::format("; {}", it->second);

    return std::format(
        "{:04} {:<12} {:<3} {:<3} {:<3} {:<15} {}",
        instruction.pos,
        ENUM_NAME(instruction.op),
        to_string(instruction.operand1),
        to_string(instruction.operand2),
        to_string(instruction.operand3),
        " ",
        comment
    );
}

} // namespace via
