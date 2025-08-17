// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "stmt_visitor.h"
#include "generator.h"
#include "sema/constexpr.h"
#include "sema/register.h"

namespace via {

namespace gen {

using namespace ast;

void StmtVisitor::visit(const NodeStmtVar& svar) {
  u16 dst = sema::alloc_register(ctx.sema);

  switch (svar.lval->kind) {
    case LValue::Kind::Symbol:
      if (sema::is_constexpr(ctx.sema, svar.lval->sym)) {
        u16 kp;

        auto cv = sema::to_constexpr(ctx.sema, svar.lval->sym);
        ctx.emit_constant(std::move(cv), &kp);
        ctx.emit_instruction(Opcode::PUSHK, {dst, kp});
      } else {
        ctx.emit_instruction(Opcode::PUSH, {dst});
      }

      sema::free_register(ctx.sema, dst);
      break;
    default:
      VIA_UNIMPLEMENTED();
  }
}

void StmtVisitor::visit(const NodeStmtScope& sscp) {}
void StmtVisitor::visit(const NodeStmtIf& sif) {}
void StmtVisitor::visit(const NodeStmtFor& sfor) {}
void StmtVisitor::visit(const NodeStmtForEach& sfeach) {}
void StmtVisitor::visit(const NodeStmtWhile& swhl) {}
void StmtVisitor::visit(const NodeStmtAssign& sasgn) {}
void StmtVisitor::visit(const NodeStmtEmpty& semt) {}
void StmtVisitor::visit(const NodeStmtExpr& sexpr) {}

}  // namespace gen

}  // namespace via
