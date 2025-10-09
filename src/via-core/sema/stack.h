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
#include <optional>
#include <stack>
#include <vector>
#include <via/config.h>
#include "module/symbol.h"

namespace via {
namespace sema {

class Module;

template <typename Local>
class Frame final
{
  public:
    using Ref = struct Local::Ref;

  public:
    Local& top() { return m_locals.back(); }
    std::optional<Ref> get_local(SymbolId symbol)
    {
        for (int64_t i = m_locals.size() - 1; i >= 0; --i) {
            Local& local = m_locals[i];
            if (local.get_symbol() == symbol) {
                return Ref(static_cast<uint16_t>(i), &local);
            }
        }

        return std::nullopt;
    }

    template <typename... Args>
        requires(std::is_constructible_v<Local, SymbolId, size_t, Args...>)
    void set_local(SymbolId symbol, Args&&... args)
    {
        size_t version = 0;
        if (auto lref = get_local(symbol)) {
            version = lref->local->get_version() + 1;
        }
        m_locals.emplace_back(symbol, version, args...);
    }

    void save() { m_stack_ptr = m_locals.size(); }
    void restore() { m_locals.resize(m_stack_ptr); }
    auto& get() { return m_locals; }

  private:
    Module* m_module;
    size_t m_stack_ptr;
    std::vector<Local> m_locals;
};

template <typename Local>
using StackState = std::stack<Frame<Local>>;

} // namespace sema
} // namespace via
