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
#include <cstddef>
#include <limits>
#include <via/config.hpp>
#include "debug.hpp"
#include "diagnostics.hpp"

namespace via {
namespace config {

VIA_CONSTANT size_t REGISTER_COUNT = UINT16_MAX;

}

namespace sema {

class RegisterState
{
  public:
    RegisterState(DiagContext& diags)
        : m_diags(diags)
    {}

  public:
    inline uint16_t alloc() noexcept
    {
        for (size_t i = 0; i < config::REGISTER_COUNT; ++i) {
            if (!m_buffer.test(i)) { // free register
                m_buffer.set(i);     // mark as occupied
                return static_cast<uint16_t>(i);
            }
        }

        m_diags.report<Level::ERROR>(
            {0, std::numeric_limits<size_t>::max()},
            "Program complexity exceeds language limits (out of register space)"
        );
        return 0;
    }

    inline void free(uint16_t reg) noexcept
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
    DiagContext& m_diags;
    std::bitset<config::REGISTER_COUNT> m_buffer;
};

} // namespace sema
} // namespace via
