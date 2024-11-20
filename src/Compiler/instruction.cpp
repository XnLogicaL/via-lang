/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "instruction.h"
#include "core.h"

namespace via::Compilation
{

viaInstruction::viaInstruction(const std::string &op_str, const std::vector<viaOperand> &operands)
{
    op = ENUM_CAST(VM::OpCode, op_str).value_or(VM::OpCode::NOP);
    operandc = operands.size();
    size_t i = 0;

    for (const viaOperand &operand : operands)
    {
        viaOperand _operand = operand;
        operandv[i++] = _operand;
    }
}

const std::string viaInstruction::compile() const noexcept
{
    std::string operands_str;

    for (size_t i = 0; i < operandc; i++)
    {
        viaOperand operand = operandv[i];
        operands_str += operand.compile() + " ";
    }

    return std::format("{} {};\n", ENUM_NAME(op), operands_str);
}

const std::string viaOperand::compile() const noexcept
{
    switch (type)
    {
    case OType::Bool:
        return boole ? "true" : "false";
    case OType::Identifier:
        return std::format("@{}", ident);
    case OType::viaNumber:
        return std::to_string(num);
    case OType::String:
        return str;
    case OType::Register:
        return std::format("{}{}", ENUM_NAME(reg.type), reg.offset);
    default:
        return "";
    }
}

} // namespace via::Compilation