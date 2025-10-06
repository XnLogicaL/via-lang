/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "instruction.h"
#include <array>
#include <iomanip>
#include "support/ansi.h"

using OpCode = via::OpCode;

enum Operand
{
    UNUSED,
    LITERAL,
    REGISTER,
    CONSTANT,

};

struct OpInfo
{
    OpCode op;
    Operand a = UNUSED, b = UNUSED, c = UNUSED;
};

static OpInfo OPERAND_INFO_MAP[] = {
    {OpCode::NOP},
    {OpCode::HALT},
    {OpCode::EXTRAARG1, LITERAL},
    {OpCode::EXTRAARG2, LITERAL, LITERAL},
    {OpCode::EXTRAARG3, LITERAL, LITERAL, LITERAL},
    {OpCode::MOVE, REGISTER, REGISTER},
    {OpCode::FREE1, REGISTER},
    {OpCode::FREE2, REGISTER, REGISTER},
    {OpCode::FREE3, REGISTER, REGISTER, REGISTER},
    {OpCode::XCHG, REGISTER, REGISTER},
    {OpCode::COPY, REGISTER, REGISTER},
    {OpCode::COPYREF, REGISTER, REGISTER},
    {OpCode::LOADK, REGISTER, CONSTANT},
    {OpCode::LOADTRUE, REGISTER},
    {OpCode::LOADFALSE, REGISTER},
    {OpCode::NEWSTR},
    {OpCode::NEWARR},
    {OpCode::NEWDICT},
    {OpCode::NEWTUPLE},
    {OpCode::NEWCLOSURE, REGISTER, LITERAL, LITERAL},
    {OpCode::IADD, REGISTER, REGISTER, REGISTER},
    {OpCode::IADDK, REGISTER, REGISTER, CONSTANT},
    {OpCode::FADD, REGISTER, REGISTER, REGISTER},
    {OpCode::FADDK, REGISTER, REGISTER, CONSTANT},
    {OpCode::ISUB, REGISTER, REGISTER, REGISTER},
    {OpCode::ISUBK, REGISTER, REGISTER, CONSTANT},
    {OpCode::FSUB, REGISTER, REGISTER, REGISTER},
    {OpCode::FSUBK, REGISTER, REGISTER, CONSTANT},
    {OpCode::IMUL, REGISTER, REGISTER, REGISTER},
    {OpCode::IMULK, REGISTER, REGISTER, CONSTANT},
    {OpCode::FMUL, REGISTER, REGISTER, REGISTER},
    {OpCode::FMULK, REGISTER, REGISTER, CONSTANT},
    {OpCode::IDIV, REGISTER, REGISTER, REGISTER},
    {OpCode::IDIVK, REGISTER, REGISTER, CONSTANT},
    {OpCode::FDIV, REGISTER, REGISTER, REGISTER},
    {OpCode::FDIVK, REGISTER, REGISTER, CONSTANT},
    {OpCode::IADD, REGISTER, REGISTER, REGISTER},
    {OpCode::IADDK, REGISTER, REGISTER, CONSTANT},
    {OpCode::FADD, REGISTER, REGISTER, REGISTER},
    {OpCode::FADDK, REGISTER, REGISTER, CONSTANT},
    {OpCode::INEG, REGISTER, REGISTER},
    {OpCode::INEGK, REGISTER, CONSTANT},
    {OpCode::FNEG, REGISTER, REGISTER},
    {OpCode::FNEGK, REGISTER, CONSTANT},
    {OpCode::BAND, REGISTER, REGISTER, REGISTER},
    {OpCode::BANDK, REGISTER, REGISTER, CONSTANT},
    {OpCode::BOR, REGISTER, REGISTER, REGISTER},
    {OpCode::BORK, REGISTER, REGISTER, CONSTANT},
    {OpCode::BXOR, REGISTER, REGISTER, REGISTER},
    {OpCode::BXORK, REGISTER, REGISTER, CONSTANT},
    {OpCode::BSHL, REGISTER, REGISTER, REGISTER},
    {OpCode::BSHLK, REGISTER, REGISTER, CONSTANT},
    {OpCode::BSHR, REGISTER, REGISTER, REGISTER},
    {OpCode::BSHRK, REGISTER, REGISTER, CONSTANT},
    {OpCode::BNOT, REGISTER, REGISTER},
    {OpCode::BNOTK, REGISTER, CONSTANT},
    {OpCode::AND, REGISTER, REGISTER, REGISTER},
    {OpCode::ANDK, REGISTER, REGISTER, CONSTANT},
    {OpCode::OR, REGISTER, REGISTER, REGISTER},
    {OpCode::ORK, REGISTER, REGISTER, CONSTANT},
    {OpCode::IEQ, REGISTER, REGISTER, REGISTER},
    {OpCode::IEQK, REGISTER, REGISTER, CONSTANT},
    {OpCode::FEQ, REGISTER, REGISTER, REGISTER},
    {OpCode::FEQK, REGISTER, REGISTER, CONSTANT},
    {OpCode::BEQ, REGISTER, REGISTER, REGISTER},
    {OpCode::BEQK, REGISTER, REGISTER, CONSTANT},
    {OpCode::SEQ, REGISTER, REGISTER, REGISTER},
    {OpCode::SEQK, REGISTER, REGISTER, CONSTANT},
    {OpCode::INEQ, REGISTER, REGISTER, REGISTER},
    {OpCode::INEQK, REGISTER, REGISTER, CONSTANT},
    {OpCode::FNEQ, REGISTER, REGISTER, REGISTER},
    {OpCode::FNEQK, REGISTER, REGISTER, CONSTANT},
    {OpCode::BNEQ, REGISTER, REGISTER, REGISTER},
    {OpCode::BNEQK, REGISTER, REGISTER, CONSTANT},
    {OpCode::SNEQ, REGISTER, REGISTER, REGISTER},
    {OpCode::SNEQK, REGISTER, REGISTER, CONSTANT},
    {OpCode::IS, REGISTER, REGISTER, REGISTER},
    {OpCode::ILT, REGISTER, REGISTER, REGISTER},
    {OpCode::ILTK, REGISTER, REGISTER, CONSTANT},
    {OpCode::FLT, REGISTER, REGISTER, REGISTER},
    {OpCode::FLTK, REGISTER, REGISTER, CONSTANT},
    {OpCode::IGT, REGISTER, REGISTER, REGISTER},
    {OpCode::IGTK, REGISTER, REGISTER, CONSTANT},
    {OpCode::FGT, REGISTER, REGISTER, REGISTER},
    {OpCode::FGTK, REGISTER, REGISTER, CONSTANT},
    {OpCode::ILTEQ, REGISTER, REGISTER, REGISTER},
    {OpCode::ILTEQK, REGISTER, REGISTER, CONSTANT},
    {OpCode::FLTEQ, REGISTER, REGISTER, REGISTER},
    {OpCode::FLTEQK, REGISTER, REGISTER, CONSTANT},
    {OpCode::IGTEQ, REGISTER, REGISTER, REGISTER},
    {OpCode::IGTEQK, REGISTER, REGISTER, CONSTANT},
    {OpCode::FGTEQ, REGISTER, REGISTER, REGISTER},
    {OpCode::FGTEQK, REGISTER, REGISTER, CONSTANT},
    {OpCode::NOT, REGISTER, REGISTER},
    {OpCode::JMP, LITERAL},
    {OpCode::JMPIF, REGISTER, LITERAL},
    {OpCode::JMPIFX, REGISTER, LITERAL},
    {OpCode::JMPBACK, LITERAL},
    {OpCode::JMPBACKIF, REGISTER, LITERAL},
    {OpCode::JMPBACKIFX, REGISTER, LITERAL},
    {OpCode::SAVE},
    {OpCode::RESTORE},
    {OpCode::PUSH, REGISTER},
    {OpCode::PUSHK, CONSTANT},
    {OpCode::GETTOP, REGISTER},
    {OpCode::GETARG, REGISTER, LITERAL},
    {OpCode::GETARGREF, REGISTER, LITERAL},
    {OpCode::SETARG, REGISTER, LITERAL},
    {OpCode::GETLOCAL, REGISTER, LITERAL},
    {OpCode::GETLOCALREF, REGISTER, LITERAL},
    {OpCode::SETLOCAL, REGISTER, LITERAL},
    {OpCode::CALL, REGISTER},
    {OpCode::PCALL, REGISTER},
    {OpCode::RET, REGISTER},
    {OpCode::RETNIL},
    {OpCode::RETTRUE},
    {OpCode::RETFALSE},
    {OpCode::RETK, CONSTANT},
    {OpCode::TOINT, REGISTER, REGISTER},
    {OpCode::TOFLOAT, REGISTER, REGISTER},
    {OpCode::TOBOOL, REGISTER, REGISTER},
    {OpCode::TOSTRING, REGISTER, REGISTER},
    {OpCode::GETIMPORT, REGISTER, LITERAL, LITERAL},
};

std::string via::Instruction::to_string(bool use_color) const
{
    std::string opcode(via::to_string(op));
    std::array<int, 3> operands{a, b, c};

    std::ostringstream oss;
    oss << std::left << std::setw(use_color ? 24 : 16) << std::setfill(' ')
        << (use_color ? ansi::format(
                            opcode,
                            ansi::Foreground::MAGENTA,
                            ansi::Background::NONE,
                            ansi::Style::BOLD
                        )
                      : opcode);

    OpInfo info;
    bool found = false;

    for (const auto& pair: OPERAND_INFO_MAP) {
        if (pair.op == op) {
            info = pair;
            found = true;
            break;
        }
    }

    if (!found) {
        oss << std::setw(6) << a << std::setw(6) << b << std::setw(6) << c;
        oss << "(MISSING OPERAND INFO!)";
        return oss.str();
    }

    std::array<Operand, 3> operand_types = {info.a, info.b, info.c};

    for (int i = 0; i < 3; ++i) {
        auto type = operand_types[i];
        if (type == UNUSED) {
            break;
        }

        std::string formatted;
        switch (type) {
        case LITERAL:
            formatted = std::to_string(operands[i]);
            break;
        case REGISTER:
            formatted = "R" + std::to_string(operands[i]);
            break;
        case CONSTANT:
            formatted = "K" + std::to_string(operands[i]);
            break;
        default:
            break;
        }

        oss << std::right << std::setw(3) << formatted;

        if (i < 2 && operand_types[i + 1] != UNUSED) {
            oss << ", ";
        }
    }

    return oss.str();
}
