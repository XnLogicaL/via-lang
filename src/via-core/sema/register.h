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

namespace via
{

namespace config
{

inline constexpr usize kRegisterCount = UINT16_MAX;

}

namespace sema
{

class RegisterState
{
 public:
  u16 alloc();
  void free(u16 reg);

 private:
  std::vector<u16> mBuffer{config::kRegisterCount / 8};
};

}  // namespace sema

}  // namespace via
