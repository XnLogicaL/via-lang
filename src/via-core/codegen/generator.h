// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_CODEGEN_GENERATOR_H_
#define VIA_CORE_CODEGEN_GENERATOR_H_

#include <via/config.h>
#include <via/types.h>
#include "diagnostics.h"
#include "expr_visitor.h"
#include "parser/parser.h"
#include "sema/const_value.h"
#include "sema/context.h"
#include "sema/stack.h"
#include "stmt_visitor.h"
#include "vm/header.h"
#include "vm/instruction.h"

namespace via {

class Generator final {
 public:
  friend class gen::ExprVisitor;
  friend class gen::StmtVisitor;

 public:
  Generator(const Vec<ast::StmtNode*>& ast, Diagnostics& diags)
      : m_ast(ast), m_diags(diags) {}

 public:
  Header generate();

  Diagnostics& get_diagnostics() { return m_diags; }
  sema::Context& get_sema_context() { return m_sema; }
  sema::Stack& get_sema_stack() { return m_stack; }

 protected:
  void emit_instruction(Opcode op, Array<u16, 3> ops = {});
  void emit_constant(sema::ConstValue&& cv, u16* kp);

 protected:
  const Vec<ast::StmtNode*>& m_ast;
  Diagnostics& m_diags;
  Header m_header;
  sema::Stack m_stack;
  sema::Context m_sema;
};

}  // namespace via

#endif
