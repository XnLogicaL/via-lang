// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_AST_OPTIMIZER_H_
#define VIA_CORE_AST_OPTIMIZER_H_

#include <via/config.h>
#include <via/types.h>
#include "pass.h"

namespace via {

namespace ast {

class Optimizer final {
 public:
  Optimizer(Allocator& alloc, SyntaxTree& ast, sema::Context& ctx)
      : m_alloc(alloc), m_ast(ast), m_ctx(ctx) {}

 public:
  template <types::pass Pass>
  void apply_pass() {
    Pass pass(m_alloc, m_ast, m_ctx);

    for (auto it = m_ast.begin(); it < m_ast.end(); it++) {
      ast::StmtNode* substitute = pass.apply(*it);
      if (substitute != NULL)
        *it = substitute;
    }
  }

  template <types::pass... Passes>
  void apply_all() {
    (apply_pass<Passes>(), ...);
  }

 private:
  Allocator& m_alloc;
  SyntaxTree& m_ast;
  sema::Context& m_ctx;
};

}  // namespace ast

}  // namespace via

#endif
