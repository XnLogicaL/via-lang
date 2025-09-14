/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "executable.h"
#include "ansi.h"
#include "module/module.h"

via::Executable* via::Executable::buildFromIR(Module* module,
                                              const IRTree& ir,
                                              u64 flags) noexcept
{
  auto& alloc = module->getAllocator();
  auto* exe = alloc.emplace<Executable>();

  for (const auto& stmt : ir) {
    exe->lowerStmt(stmt);
  }

  exe->lowerJumps();
  exe->pushInstr(OpCode::HALT);
  return exe;
}

void via::Executable::lowerJumps() noexcept
{
  for (Instruction& instr : mBytecode) {
    auto opid = static_cast<u16>(instr.op);
    if (opid >= static_cast<u16>(OpCode::JMP) &&
        opid <= static_cast<u16>(OpCode::JMPBACKIFX)) {
    }
  }
}

std::string via::Executable::dump() const
{
  std::ostringstream oss;
  oss << ansi::format("[section .text]\n", ansi::Foreground::Yellow,
                      ansi::Background::Black, ansi::Style::Underline);

  for (const Instruction& insn : mBytecode) {
    oss << "  " << insn.dump() << "\n";
  }

  oss << ansi::format("[section .data]\n", ansi::Foreground::Yellow,
                      ansi::Background::Black, ansi::Style::Underline);

  for (const sema::ConstValue& cv : mConstants) {
    oss << "  " << cv.dump() << "\n";
  }

  return oss.str();
}
