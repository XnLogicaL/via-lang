/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "instruction.h"

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

bool Operand::check_type(OperandType ty)
{
    return type == ty;
}

bool Operand::check_boolean()
{
    return check_type(OperandType::Bool);
}

bool Operand::check_number()
{
    return check_type(OperandType::Number);
}

bool Operand::check_string()
{
    return check_type(OperandType::String);
}

bool Operand::check_register()
{
    return check_type(OperandType::Register);
}

bool Operand::check_nil()
{
    return check_type(OperandType::Nil);
}

} // namespace via
