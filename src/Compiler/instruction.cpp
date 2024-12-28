/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "instruction.h"
#include "core.h"

namespace via::Compilation
{

Instruction cnewinstruction()
{
    Instruction instr;
    instr.op = OpCode::NOP;
    instr.chunk = nullptr;
    instr.operand1 = cnewoperand();
    instr.operand2 = cnewoperand();
    instr.operand3 = cnewoperand();

    return instr;
}

Instruction cnewinstruction(OpCode op, const std::vector<Operand> &operands)
{
    Instruction instr = cnewinstruction();
    instr.op = op;

    if (operands.size() > 0)
        instr.operand1 = operands.at(0);
    if (operands.size() > 1)
        instr.operand2 = operands.at(1);
    if (operands.size() > 2)
        instr.operand3 = operands.at(2);

    return instr;
}

// clang-format off
std::string ccompileinstruction(Instruction& instr) {
    const std::string operands_str = std::format(
        "{} {} {}", 
        ccompileoperand(instr.operand1), 
        ccompileoperand(instr.operand2), 
        ccompileoperand(instr.operand3)
    );

    return std::format(
        "0x{:02X} {:<8} {}", 
        static_cast<unsigned>(instr.op), // Hex opcode
        ENUM_NAME(instr.op),             // Opcode name
        operands_str                     // Operands
    );
}
// clang-format on

// clang-format off
std::string ccompileoperand(Operand& oper) {
    switch (oper.type) {
        case OperandType::Bool:       return oper.val_boolean ? "true" : "false";
        case OperandType::Identifier: return std::format("@{}", oper.val_identifier);
        case OperandType::Number:     return std::to_string(oper.val_number);
        case OperandType::String:     return std::format("\"{}\"", oper.val_string);
        case OperandType::GPRegister:   return std::format("R{}", oper.val_register);
        case OperandType::Nil:        return std::string();
        default:                      return std::string(ENUM_NAME(oper.type));
    }
}
// clang-format on

// Type-checking utility functions
bool ccheckregister(const Operand &oper)
{
    return oper.type == OperandType::GPRegister;
}
bool cchecknumber(const Operand &oper)
{
    return oper.type == OperandType::Number;
}
bool ccheckbool(const Operand &oper)
{
    return oper.type == OperandType::Bool;
}
bool ccheckstring(const Operand &oper)
{
    return oper.type == OperandType::String;
}
bool ccheckidentifier(const Operand &oper)
{
    return oper.type == OperandType::Identifier;
}

// Operand creation utility functions
Operand cnewoperand()
{
    Operand operand;
    operand.type = OperandType::Nil;

    return operand;
}

Operand cnewoperand(double number)
{
    return {.type = OperandType::Number, .val_number = number};
}

Operand cnewoperand(bool boolean)
{
    return {.type = OperandType::Bool, .val_boolean = boolean};
}

Operand cnewoperand(const char *value, bool is_identifier)
{
    // clang-format off
    return is_identifier 
        ? Operand{.type = OperandType::Identifier, .val_identifier = value}
        : Operand{.type = OperandType::String, .val_string = value};
    // clang-format on
}

Operand cnewoperand(GPRegister reg)
{
    return {.type = OperandType::GPRegister, .val_register = reg};
}

} // namespace via::Compilation
