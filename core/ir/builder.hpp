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
#include <string>
#include <unordered_set>
#include <via/config.hpp>
#include "ast/ast.hpp"
#include "ir.hpp"
#include "module/manager.hpp"
#include "module/module.hpp"
#include "module/symbol.hpp"
#include "sema/local_ir.hpp"
#include "sema/types.hpp"
#include "support/traits.hpp"

namespace via {

class IRBuilder;

namespace detail {

template <derived_from<ast::Expr, ast::Type> Type>
QualType ast_type_of(IRBuilder&, const Type*) noexcept
{
    debug::todo(std::format("ast_type_of<{}>()", VIA_TYPENAME(Type)));
}

template <derived_from<ast::Expr> Expr>
const ir::Expr* ast_lower_expr(IRBuilder&, const Expr*) noexcept
{
    debug::todo(std::format("ast_lower_expr<{}>()", VIA_TYPENAME(Expr)));
}

template <derived_from<ast::Stmt> Stmt>
const ir::Stmt* ast_lower_stmt(IRBuilder&, const Stmt*) noexcept
{
    debug::todo(std::format("ast_lower_stmt<{}>()", VIA_TYPENAME(Stmt)));
}

} // namespace detail

class Module;
class IRBuilder final
{
  public:
    template <derived_from<ast::Expr, ast::Type> Type>
    friend QualType detail::ast_type_of(IRBuilder&, const Type*) noexcept;

    template <derived_from<ast::Expr> Expr>
    friend const ir::Expr* detail::ast_lower_expr(IRBuilder&, const Expr*) noexcept;

    template <derived_from<ast::Stmt> Stmt>
    friend const ir::Stmt* detail::ast_lower_stmt(IRBuilder&, const Stmt*) noexcept;

    friend class Module;

  public:
    IRBuilder(via::Module* module, const SyntaxTree& ast, DiagContext& diags)
        : m_module(module),
          m_ast(ast),
          m_alloc(module->allocator()),
          m_diags(diags),
          m_type_ctx(module->manager().type_context()),
          m_symbol_table(module->manager().symbol_table())
    {}

  public:
    IRTree build();

  protected:
    // clang-format off
    void poison_symbol(SymbolId symbol) noexcept { m_poisoned_ids.insert(symbol); }
    void poison_symbol(QualName name) noexcept { m_poisoned_ids.insert(intern_symbol(name)); }
    void poison_symbol(std::string symbol) noexcept { m_poisoned_ids.insert(intern_symbol(symbol)); }

    bool is_poisoned(SymbolId symbol) noexcept { return m_poisoned_ids.contains(symbol); }
    bool is_poisoned(QualName name) noexcept { return m_poisoned_ids.contains(intern_symbol(name)); }
    bool is_poisoned(std::string symbol) noexcept { return m_poisoned_ids.contains(intern_symbol(symbol)); }
    // clang-format on

  private:
    QualType type_of(const ast::Expr* expr) noexcept;
    QualType type_of(const ast::Type* type) noexcept;
    const ir::Expr* lower_expr(const ast::Expr* expr);
    const ir::Stmt* lower_stmt(const ast::Stmt* stmt);
    ir::StmtBlock* end_block() noexcept;
    ir::StmtBlock* new_block(size_t id) noexcept;
    std::string dump_type(QualType type) noexcept;
    std::string dump_expr(const ast::Expr* expr) noexcept;

    // clang-format off
    SymbolId intern_symbol(std::string symbol) noexcept { return m_symbol_table.intern(symbol); }
    SymbolId intern_symbol(const QualName& name) noexcept { return m_symbol_table.intern(name); }
    SymbolId intern_symbol(const via::Token& token) noexcept { return m_symbol_table.intern(token.to_string()); }
    // clang-format on

  private:
    via::Module* m_module;
    const SyntaxTree& m_ast;
    ScopedAllocator& m_alloc;
    DiagContext& m_diags;
    StackState<IRLocal> m_stack;
    TypeContext& m_type_ctx;
    SymbolTable& m_symbol_table;
    bool m_should_push_block;
    uint32_t m_block_id = 0;
    ir::StmtBlock* m_current_block;
    std::unordered_set<SymbolId> m_poisoned_ids;
};

} // namespace via
