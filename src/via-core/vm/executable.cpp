/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "executable.h"
#include "color.h"
#include "sema/const_value.h"

std::string via::Executable::dump() const
{
  std::ostringstream oss;
  oss << ansiFormat("[section .text]\n", Fg::Yellow, Bg::Black,
                    Style::Underline);

  for (const Instruction& insn : bytecode) {
    oss << "  " << insn.dump() << "\n";
  }

  oss << ansiFormat("[section .data]\n", Fg::Yellow, Bg::Black,
                    Style::Underline);

  for (const sema::ConstValue& cv : consts) {
    oss << "  " << cv.dump() << "\n";
  }

  return oss.str();
}
