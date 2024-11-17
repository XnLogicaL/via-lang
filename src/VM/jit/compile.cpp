/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "compile.h"

namespace via::VM
{

bool is_compilable(OpCode op)
{
    switch (op)
    {
    case OpCode::ADD:
    case OpCode::SUB:
    case OpCode::MUL:
    case OpCode::DIV:
    case OpCode::BAND:
    case OpCode::BOR:
    case OpCode::BNOT:
    case OpCode::BXOR:
    case OpCode::BSHL:
    case OpCode::BSHR:
    case OpCode::BROL:
    case OpCode::BROR:
    case OpCode::BSAR:
    case OpCode::EQ:
    case OpCode::NEQ:
    case OpCode::LT:
    case OpCode::GT:
    case OpCode::LE:
    case OpCode::GE:
        return true;
    
    default:
        return false;
    }
}

} // namespace via::VM
