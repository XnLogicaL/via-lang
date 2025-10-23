/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <cstddef>
#include <type_traits>

namespace via {

template <typename Wider, typename High, typename Low>
    requires(
        std::is_unsigned_v<Wider> && std::is_unsigned_v<High> &&
        std::is_unsigned_v<Low> && (sizeof(Wider) >= sizeof(High) + sizeof(Low))
    )
constexpr Wider pack_halves(High high, Low low) noexcept
{
    constexpr size_t low_bits = sizeof(Low) * 8;

    return static_cast<Wider>(
        (static_cast<Wider>(high) << low_bits) | static_cast<Wider>(low)
    );
}

template <typename Big, typename Small>
    requires(
        std::is_unsigned_v<Big> && std::is_unsigned_v<Small> &&
        sizeof(Big) == sizeof(Small) * 2
    )
constexpr void unpack_halves(Big value, Small& high, Small& low) noexcept
{
    constexpr size_t low_bits = sizeof(Small) * 8;
    constexpr Big mask = (static_cast<Big>(1) << low_bits) - 1;

    low = static_cast<Small>(value & mask);
    high = static_cast<Small>(value >> low_bits);
}

} // namespace via
