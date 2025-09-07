/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <via/config.h>
#include <via/types.h>
#include "instruction.h"

namespace via
{

namespace sema
{

class ConstValue;

}

struct Executable
{
  static constexpr u32 kmagic = 0x2E766961;  // .via

  u32 magic;
  u64 flags;
  Vec<sema::ConstValue> consts;
  Vec<Instruction> bytecode;

  std::string dump() const;
};

}  // namespace via
