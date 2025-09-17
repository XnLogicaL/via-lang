/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "instruction.h"
#include <iomanip>
#include <magic_enum/magic_enum.hpp>

using OpCode = via::OpCode;

std::string via::Instruction::get_dump() const
{
    std::ostringstream oss;
    oss << std::left << std::setw(16) << std::setfill(' ') << magic_enum::enum_name(op);

    oss << a << ", ";
    oss << b << ", ";
    oss << c;

    return oss.str();
}
