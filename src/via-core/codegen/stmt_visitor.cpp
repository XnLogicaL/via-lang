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
  sema::Context& sema_ctx = ctx.get_sema_context();
  sema::Frame& frame = sema_ctx.stack.top();

  u16 dst = sema::alloc_register(sema_ctx);

  switch (svar.lval->kind) {
    case LValue::Kind::Symbol: {
      String symbol = svar.lval->sym->tok->to_string();
      frame.set_local(symbol, svar.lval, svar.rval, NULL);

      if (sema::is_constexpr(sema_ctx, svar.rval)) {
        u16 kp;

        auto cvr = sema::to_constexpr(sema_ctx, svar.rval);
        if (cvr.has_value()) {
          ctx.emit_constant(std::move(*cvr), &kp);
          ctx.emit_instruction(Opcode::PUSHK, {dst, kp});
        } else {
          ctx.get_diagnostics().diagnose<Diagnosis::Kind::Error>(svar.loc,
                                                                 cvr.error());
        }
      } else {
        svar.rval->accept(expr_vis, dst);
        ctx.emit_instruction(Opcode::PUSH, {dst});
      }

      sema::free_register(sema_ctx, dst);
      break;
    }
    default:
      VIA_UNIMPLEMENTED();
  }
}

void StmtVisitor::visit(const NodeStmtScope& sscp) {
  sema::Context& sema_ctx = ctx.get_sema_context();
  sema::Frame& frame = sema_ctx.stack.top();

  frame.save();
  ctx.emit_instruction(Opcode::SAVESP);

  for (const ast::StmtNode* stmt : sscp.stmts)
    stmt->accept(*this);

  frame.restore();
  ctx.emit_instruction(Opcode::RESTSP);
}

void StmtVisitor::visit(const NodeStmtIf& sif) {}
void StmtVisitor::visit(const NodeStmtFor& sfor) {}
void StmtVisitor::visit(const NodeStmtForEach& sfeach) {}
void StmtVisitor::visit(const NodeStmtWhile& swhl) {}
void StmtVisitor::visit(const NodeStmtAssign& sasgn) {}
void StmtVisitor::visit(const NodeStmtEmpty& semt) {}
void StmtVisitor::visit(const NodeStmtExpr& sexpr) {}

}  // namespace gen

}  // namespace via
