/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <cstdint>
#include <string>
#include <via/config.hpp>
#include "support/utility.hpp"

namespace via {

// xmacros are goated
#define FOR_EACH_OPCODE(X)                                                               \
    X(NOP)                                                                               \
    X(HALT)                                                                              \
    X(EXTRAARG1)                                                                         \
    X(EXTRAARG2)                                                                         \
    X(EXTRAARG3)                                                                         \
    X(MOVE)                                                                              \
    X(FREE1)                                                                             \
    X(FREE2)                                                                             \
    X(FREE3)                                                                             \
    X(XCHG)                                                                              \
    X(COPY)                                                                              \
    X(COPYREF)                                                                           \
    X(LOADK)                                                                             \
    X(LOADTRUE)                                                                          \
    X(LOADFALSE)                                                                         \
    X(NEWSTR)                                                                            \
    X(NEWARR)                                                                            \
    X(NEWDICT)                                                                           \
    X(NEWTUPLE)                                                                          \
    X(NEWCLOSURE)                                                                        \
    X(IADD)                                                                              \
    X(IADDK)                                                                             \
    X(FADD)                                                                              \
    X(FADDK)                                                                             \
    X(ISUB)                                                                              \
    X(ISUBK)                                                                             \
    X(FSUB)                                                                              \
    X(FSUBK)                                                                             \
    X(IMUL)                                                                              \
    X(IMULK)                                                                             \
    X(FMUL)                                                                              \
    X(FMULK)                                                                             \
    X(IDIV)                                                                              \
    X(IDIVK)                                                                             \
    X(FDIV)                                                                              \
    X(FDIVK)                                                                             \
    X(INEG)                                                                              \
    X(INEGK)                                                                             \
    X(FNEG)                                                                              \
    X(FNEGK)                                                                             \
    X(BAND)                                                                              \
    X(BANDK)                                                                             \
    X(BOR)                                                                               \
    X(BORK)                                                                              \
    X(BXOR)                                                                              \
    X(BXORK)                                                                             \
    X(BSHL)                                                                              \
    X(BSHLK)                                                                             \
    X(BSHR)                                                                              \
    X(BSHRK)                                                                             \
    X(BNOT)                                                                              \
    X(BNOTK)                                                                             \
    X(AND)                                                                               \
    X(ANDK)                                                                              \
    X(OR)                                                                                \
    X(ORK)                                                                               \
    X(IEQ)                                                                               \
    X(IEQK)                                                                              \
    X(FEQ)                                                                               \
    X(FEQK)                                                                              \
    X(BEQ)                                                                               \
    X(BEQK)                                                                              \
    X(SEQ)                                                                               \
    X(SEQK)                                                                              \
    X(INEQ)                                                                              \
    X(INEQK)                                                                             \
    X(FNEQ)                                                                              \
    X(FNEQK)                                                                             \
    X(BNEQ)                                                                              \
    X(BNEQK)                                                                             \
    X(SNEQ)                                                                              \
    X(SNEQK)                                                                             \
    X(IS)                                                                                \
    X(ILT)                                                                               \
    X(ILTK)                                                                              \
    X(FLT)                                                                               \
    X(FLTK)                                                                              \
    X(IGT)                                                                               \
    X(IGTK)                                                                              \
    X(FGT)                                                                               \
    X(FGTK)                                                                              \
    X(ILTEQ)                                                                             \
    X(ILTEQK)                                                                            \
    X(FLTEQ)                                                                             \
    X(FLTEQK)                                                                            \
    X(IGTEQ)                                                                             \
    X(IGTEQK)                                                                            \
    X(FGTEQ)                                                                             \
    X(FGTEQK)                                                                            \
    X(NOT)                                                                               \
    X(JMP)                                                                               \
    X(JMPIF)                                                                             \
    X(JMPIFX)                                                                            \
    X(JMPBACK)                                                                           \
    X(JMPBACKIF)                                                                         \
    X(JMPBACKIFX)                                                                        \
    X(SAVE)                                                                              \
    X(RESTORE)                                                                           \
    X(PUSH)                                                                              \
    X(PUSHK)                                                                             \
    X(GETTOP)                                                                            \
    X(GETARG)                                                                            \
    X(GETARGREF)                                                                         \
    X(SETARG)                                                                            \
    X(GETLOCAL)                                                                          \
    X(GETLOCALREF)                                                                       \
    X(SETLOCAL)                                                                          \
    X(CALL)                                                                              \
    X(PCALL)                                                                             \
    X(RET)                                                                               \
    X(RETNIL)                                                                            \
    X(RETTRUE)                                                                           \
    X(RETFALSE)                                                                          \
    X(RETK)                                                                              \
    X(TOINT)                                                                             \
    X(TOFLOAT)                                                                           \
    X(TOBOOL)                                                                            \
    X(TOSTRING)                                                                          \
    X(GETIMPORT)

enum class OpCode : uint16_t
{
    FOR_EACH_OPCODE(DEFINE_ENUM)
};

DEFINE_TO_STRING(OpCode, FOR_EACH_OPCODE(DEFINE_CASE_TO_STRING));

struct Instruction
{
    OpCode op = OpCode::NOP;
    uint16_t a, b, c;

    std::string to_string(bool use_color = false, size_t pc = 0x0) const;
};

} // namespace via
