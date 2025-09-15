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
#include <bitset>
#include "debug.h"

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
  inline u16 alloc() noexcept
  {
    for (usize i = 0; i < config::kRegisterCount; ++i) {
      if (!mBuffer.test(i)) {  // free register
        mBuffer.set(i);        // mark as occupied
        return static_cast<u16>(i);
      }
    }
    debug::bug("semantic register allocation failure");
  }

  inline void free(u16 reg) noexcept
  {
    debug::require(reg <= config::kRegisterCount,
                   "invalid semantic register to free");
    mBuffer.reset(reg);  // mark as free
  }

 private:
  std::bitset<config::kRegisterCount> mBuffer;
};

}  // namespace sema

}  // namespace via
