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
#include "ir.h"
#include "module/manager.h"
#include "module/module.h"
#include "sema/ir_local.h"
#include "sema/stack.h"
#include "sema/type_context.h"
#include "support/type_traits.h" // derived_from

namespace via {

class IRBuilder;

namespace detail {

template <derived_from<ast::Expr, ast::Type> Type>
const sema::Type* ast_type_of(IRBuilder&, const Type*) noexcept
{
    debug::todo(std::format("ast_type_of<{}>()", TYPENAME(Type)));
}

template <derived_from<ast::Expr> Expr>
const ir::Expr* ast_lower_expr(IRBuilder&, const Expr*) noexcept
{
    debug::todo(std::format("ast_lower_expr<{}>()", TYPENAME(Expr)));
}

template <derived_from<ast::Stmt> Stmt>
const ir::Stmt* ast_lower_stmt(IRBuilder&, const Stmt*) noexcept
{
    debug::todo(std::format("ast_lower_stmt<{}>()", TYPENAME(Stmt)));
}

} // namespace detail

class Module;
class IRBuilder final
{
  public:
    template <derived_from<ast::Expr, ast::Type> Type>
    friend const sema::Type* detail::ast_type_of(IRBuilder&, const Type*) noexcept;

    template <derived_from<ast::Expr> Expr>
    friend const ir::Expr* detail::ast_lower_expr(IRBuilder&, const Expr*) noexcept;

    template <derived_from<ast::Stmt> Stmt>
    friend const ir::Stmt* detail::ast_lower_stmt(IRBuilder&, const Stmt*) noexcept;

  public:
    IRBuilder(via::Module* module, const SyntaxTree& ast, DiagContext& diags)
        : m_module(module),
          m_ast(ast),
          m_alloc(module->get_allocator()),
          m_type_ctx(module->get_manager()->get_type_context()),
          m_symbol_table(module->get_manager()->get_symbol_table()),
          m_diags(diags)
    {}

  public:
    IRTree build();

  private:
    const sema::Type* type_of(const ast::Expr* expr) noexcept;
    const sema::Type* type_of(const ast::Type* type) noexcept;

    inline ir::StmtBlock* end_block() noexcept
    {
        m_should_push_block = true;
        return m_current_block;
    }

    inline ir::StmtBlock* new_block(size_t id) noexcept
    {
        ir::StmtBlock* block = m_current_block;
        m_should_push_block = false;
        m_current_block = m_alloc.emplace<ir::StmtBlock>();
        m_current_block->id = id;
        return block;
    }

    auto intern_symbol(std::string symbol) noexcept
    {
        return m_symbol_table.intern(symbol);
    }
    auto intern_symbol(const via::Token& symbol) noexcept
    {
        return m_symbol_table.intern(symbol.to_string());
    }

    const ir::Expr* lower_expr(const ast::Expr* expr);
    const ir::Stmt* lower_stmt(const ast::Stmt* stmt);

  private:
    via::Module* m_module;
    const SyntaxTree& m_ast;
    ScopedAllocator& m_alloc;
    DiagContext& m_diags;
    sema::StackState<sema::IRLocal> m_stack;
    sema::TypeContext& m_type_ctx;
    SymbolTable& m_symbol_table;
    bool m_should_push_block;
    ir::StmtBlock* m_current_block;
};

} // namespace via
