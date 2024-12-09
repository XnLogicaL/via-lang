/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "instruction.h"
#include "core.h"

namespace via::Compilation
{

viaInstruction viaC_newinstruction()
{
    viaInstruction instr;
    instr.op = OpCode::NOP;
    instr.operandc = 0;

    return instr;
}

viaInstruction viaC_newinstruction(const std::string &op_str, const std::vector<viaOperand> &operands)
{
    viaInstruction instr;

    instr.op = ENUM_CAST(OpCode, op_str).value_or(OpCode::NOP);
    instr.operandc = operands.size();

    size_t i = 0;
    for (const viaOperand &operand : operands)
    {
        viaOperand _operand = operand;
        instr.operandv[i++] = _operand;
    }

    return instr;
}

std::string viaC_compileinstruction(viaInstruction &instr)
{
    std::string operands_str;

    for (size_t i = 0; i < instr.operandc; i++)
    {
        viaOperand operand = instr.operandv[i];
        operands_str += viaC_compileoperand(operand) + " ";
    }

    return std::format("{} {};\n", ENUM_NAME(instr.op), operands_str);
}

std::string viaC_compileoperand(viaOperand &oper)
{
    switch (oper.type)
    {
    case viaOperandType_t::Bool:
        return oper.val_boolean ? "true" : "false";
    case viaOperandType_t::Identifier:
        return std::format("@{}", oper.val_identifier);
    case viaOperandType_t::Number:
        return std::to_string(oper.val_number);
    case viaOperandType_t::String:
        return oper.val_string;
    case viaOperandType_t::Register:
        return std::format("R{}", oper.val_register);
    default:
        return "";
    }
}

bool viaC_checkregister(const viaOperand &oper)
{
    return oper.type == viaOperandType_t::Register;
}

bool viaC_checknumber(const viaOperand &oper)
{
    return oper.type == viaOperandType_t::Number;
}

bool viaC_checkbool(const viaOperand &oper)
{
    return oper.type == viaOperandType_t::Bool;
}

bool viaC_checkstring(const viaOperand &oper)
{
    return oper.type == viaOperandType_t::String;
}

bool viaC_checkidentifier(const viaOperand &oper)
{
    return oper.type == viaOperandType_t::Identifier;
}

} // namespace via::Compilation