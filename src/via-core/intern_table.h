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
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <via/config.h>

namespace via {

template <typename T>
struct view
{
    using type = std::reference_wrapper<T>;
};

template <>
struct view<std::string>
{
    using type = std::string_view;
};

template <typename T, typename Id = uint64_t>
class InternTable
{
  public:
    using view_type = typename view<T>::type;

  public:
    Id intern(const T& val)
    {
        auto [it, inserted] = m_map.try_emplace(val, m_next_id);
        if (inserted) {
            m_reverse[m_next_id] = val;
            m_next_id++;
        }
        return it->second;
    }

    std::optional<view_type> lookup(Id id) const
    {
        if (auto it = m_reverse.find(id); it != m_reverse.end()) {
            return view_type(it->second);
        }
        return std::nullopt;
    }

  protected:
    size_t m_next_id = 0;
    std::unordered_map<T, Id> m_map;
    std::unordered_map<Id, T> m_reverse;
};

} // namespace via
