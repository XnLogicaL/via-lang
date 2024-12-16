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

viaInstruction viaC_newinstruction(OpCode op, const std::vector<viaOperand> &operands)
{
    viaInstruction instr;

    instr.op = op;
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

    // Append operands with proper formatting
    for (size_t i = 0; i < instr.operandc; i++)
    {
        viaOperand operand = instr.operandv[i];
        operands_str += viaC_compileoperand(operand) + ", ";
    }

    // Remove the trailing ", " if any operands were added
    if (!operands_str.empty())
    {
        operands_str.pop_back(); // Remove last space
        operands_str.pop_back(); // Remove last comma
    }

    // Format the instruction with schema: [HEX OPCODE] [OPCODE NAME] [OPERANDS]
    return std::format(
        "0x{:02X} {:<8} {}",
        static_cast<unsigned>(instr.op), // Hex opcode, padded to 4 chars
        ENUM_NAME(instr.op),             // Opcode name, padded to 4 chars
        operands_str                     // Operand list
    );
}

std::string viaC_compileoperand(viaOperand &oper)
{
    switch (oper.type)
    {
    case viaOperandType_t::Bool:
        return std::string(oper.val_boolean ? "true" : "false");
    case viaOperandType_t::Identifier:
        return std::format("@{}", oper.val_identifier);
    case viaOperandType_t::Number:
        return std::to_string(oper.val_number);
    case viaOperandType_t::String:
        return std::format("\"{}\"", oper.val_string);
    case viaOperandType_t::Register:
        return std::format("R{}", oper.val_register);
    case viaOperandType_t::Nil:
        return std::string("nil");
    default:
        return std::string(ENUM_NAME(oper.type));
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

viaOperand viaC_newoperand()
{
    return {.type = viaOperandType_t::Nil};
}

viaOperand viaC_newoperand(double x)
{
    return {.type = viaOperandType_t::Number, .val_number = x};
}

viaOperand viaC_newoperand(bool b)
{
    return {.type = viaOperandType_t::Bool, .val_boolean = b};
}

viaOperand viaC_newoperand(const char *s, bool id)
{
    if (id)
        return {.type = viaOperandType_t::Identifier, .val_identifier = s};
    return {.type = viaOperandType_t::String, .val_string = s};
}

viaOperand viaC_newoperand(viaRegister reg)
{
    return {.type = viaOperandType_t::Register, .val_register = reg};
}

} // namespace via::Compilation