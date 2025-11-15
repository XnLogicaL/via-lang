/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <functional>

#define defer ::via::detail::DeferImpl _ = [&]

namespace via {
namespace detail {

using DeferCallback = std::function<void()>;

class DeferImpl final
{
  public:
    ~DeferImpl() { m_callback(); }
    DeferImpl(DeferCallback callback)
        : m_callback(std::move(callback))
    {}

  private:
    DeferCallback m_callback;
};

} // namespace detail
} // namespace via
