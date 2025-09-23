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
#include "ast/ast.h"
#include "ir/ir.h"
#include "module/symbol.h"

namespace via {
namespace sema {

class IRLocal final
{
  public:
    enum Qual : u8
    {
        CONST = 1 << 0,
    };

    struct Ref
    {
        u16 id;
        IRLocal& local;
    };

  public:
    IRLocal() = default;
    IRLocal(
        SymbolId symbol,
        size_t version,
        const ast::Stmt* ast_decl,
        const ir::Stmt* ir_decl,
        u8 quals = 0ULL
    )
        : m_version(version),
          m_quals(quals),
          m_symbol(symbol),
          m_ast_decl(ast_decl),
          m_ir_decl(ir_decl)
    {}

  public:
    auto get_version() const { return m_version; }
    auto get_qualifiers() const { return m_quals; }
    auto get_symbol() const { return m_symbol; }
    const auto* get_ast_decl() const { return m_ast_decl; }
    const auto* get_ir_decl() const { return m_ir_decl; }

  protected:
    const size_t m_version = 0;
    const u8 m_quals = 0ULL;
    const SymbolId m_symbol = -1;
    const ast::Stmt* m_ast_decl = nullptr;
    const ir::Stmt* m_ir_decl = nullptr;
};

} // namespace sema
} // namespace via
