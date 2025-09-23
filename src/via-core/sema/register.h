/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <bitset>
#include <via/config.h>
#include <via/types.h>
#include "debug.h"

namespace via {
namespace config {

CONSTANT size_t REGISTER_COUNT = UINT16_MAX;

}

namespace sema {

class RegisterState
{
  public:
    inline u16 alloc() noexcept
    {
        for (size_t i = 0; i < config::REGISTER_COUNT; ++i) {
            if (!m_buffer.test(i)) { // free register
                m_buffer.set(i);     // mark as occupied
                return static_cast<u16>(i);
            }
        }
        debug::bug("semantic register allocation failure");
    }

    inline void free(u16 reg) noexcept
    {
        debug::require(
            reg <= config::REGISTER_COUNT,
            "invalid semantic register to free"
        );
        m_buffer.reset(reg); // mark as free
    }

    template <typename... Regs>
    inline void free_all(Regs... regs) noexcept
    {
        (free(regs), ...);
    }

  private:
    std::bitset<config::REGISTER_COUNT> m_buffer;
};

} // namespace sema
} // namespace via
