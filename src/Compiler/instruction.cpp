/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "instruction.h"
#include "core.h"

namespace via::Compilation
{

Instruction viaC_newinstruction()
{
    Instruction instr;
    instr.op = OpCode::NOP;
    instr.chunk = nullptr;

    return instr;
}

Instruction viaC_newinstruction(OpCode op, const std::vector<Operand> &operands)
{
    Instruction instr;
    instr.op = op;
    instr.chunk = nullptr;

    if (operands.size() >= 1)
        instr.operand1 = operands.at(0);

    if (operands.size() >= 2)
        instr.operand2 = operands.at(1);

    if (operands.size() >= 3)
        instr.operand3 = operands.at(2);

    return instr;
}

std::string viaC_compileinstruction(Instruction &instr)
{
    std::string operands_str =
        std::format("{} {} {}", viaC_compileoperand(instr.operand1), viaC_compileoperand(instr.operand2), viaC_compileoperand(instr.operand3));

    // Format the instruction with schema: [HEX OPCODE] [OPCODE NAME] [OPERANDS]
    return std::format(
        "0x{:02X} {:<8} {}",
        static_cast<unsigned>(instr.op), // Hex opcode, padded to 4 chars
        ENUM_NAME(instr.op),             // Opcode name, padded to 4 chars
        operands_str                     // Operand list
    );
}

std::string viaC_compileoperand(Operand &oper)
{
    switch (oper.type)
    {
    case OperandType::Bool:
        return std::string(oper.val_boolean ? "true" : "false");
    case OperandType::Identifier:
        return std::format("@{}", oper.val_identifier);
    case OperandType::Number:
        return std::to_string(oper.val_number);
    case OperandType::String:
        return std::format("\"{}\"", oper.val_string);
    case OperandType::Register:
        return std::format("R{}", oper.val_register);
    case OperandType::Nil:
        return std::string("nil");
    default:
        return std::string(ENUM_NAME(oper.type));
    }
}

bool viaC_checkregister(const Operand &oper)
{
    return oper.type == OperandType::Register;
}

bool viaC_checknumber(const Operand &oper)
{
    return oper.type == OperandType::Number;
}

bool viaC_checkbool(const Operand &oper)
{
    return oper.type == OperandType::Bool;
}

bool viaC_checkstring(const Operand &oper)
{
    return oper.type == OperandType::String;
}

bool viaC_checkidentifier(const Operand &oper)
{
    return oper.type == OperandType::Identifier;
}

Operand viaC_newoperand()
{
    Operand operand;
    operand.type = OperandType::Nil;

    return operand;
}

Operand viaC_newoperand(double x)
{
    return {.type = OperandType::Number, .val_number = x};
}

Operand viaC_newoperand(bool b)
{
    return {.type = OperandType::Bool, .val_boolean = b};
}

Operand viaC_newoperand(const char *s, bool id)
{
    if (id)
        return {.type = OperandType::Identifier, .val_identifier = s};
    return {.type = OperandType::String, .val_string = s};
}

Operand viaC_newoperand(Register reg)
{
    return {.type = OperandType::Register, .val_register = reg};
}

} // namespace via::Compilation