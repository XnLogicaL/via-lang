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
  Generator(const AstBuf& ast, Diagnostics& diags) : ast(ast), diags(diags) {}

 public:
  Header generate();

 protected:
  void emit_instruction(Opcode op, Array<u16, 3> ops = {});
  void emit_constant(sema::ConstValue&& cv, u16* kp);

 protected:
  const AstBuf& ast;
  Diagnostics& diags;

  Header header;
  sema::Stack stack;
  sema::Context sema;
};

}  // namespace via

#endif
