// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "stmt_visitor.h"
#include "debug.h"
#include "generator.h"
#include "sema/constexpr.h"
#include "sema/register.h"

namespace via {

namespace gen {

using namespace ast;
using enum Diagnosis::Kind;

void StmtVisitor::visit(const NodeStmtVar& svar, Box<VisitInfo>) {
  sema::Context& sema_ctx = m_ctx.get_sema_context();
  sema::Frame& frame = sema_ctx.stack.top();

  LValue* lval = svar.lval;
  ExprNode* rval = svar.rval;

  switch (lval->kind) {
    case LValue::Kind::Symbol: {
      u16 dst = sema::alloc_register(sema_ctx);
      frame.set_local(lval->sym->tok->to_string_view(), lval, rval, NULL);

      if (sema::is_constexpr(sema_ctx, rval)) {
        u16 kp;

        auto cvr = sema::to_constexpr(sema_ctx, rval);
        if (cvr.has_value()) {
          m_ctx.emit_constant(std::move(*cvr), &kp);
          m_ctx.emit_instruction(Opcode::PUSHK, {dst, kp});
        } else {
          m_ctx.get_diagnostics().diagnose<Error>(svar.loc, cvr.error());
        }
      } else {
        svar.rval->accept(m_vis, make_visit_info(dst));
        m_ctx.emit_instruction(Opcode::PUSH, {dst});
      }

      sema::free_register(sema_ctx, dst);
      break;
    }
    default:
      unimplemented("lvalue case");
  }
}

void StmtVisitor::visit(const NodeStmtScope& sscp, Box<VisitInfo>) {
  sema::Context& sema_ctx = m_ctx.get_sema_context();
  sema::Frame& frame = sema_ctx.stack.top();

  frame.save();
  m_ctx.emit_instruction(Opcode::SAVESP);

  for (const ast::StmtNode* stmt : sscp.stmts)
    stmt->accept(*this, {});

  frame.restore();
  m_ctx.emit_instruction(Opcode::RESTSP);
}

void StmtVisitor::visit(const NodeStmtIf& sif, Box<VisitInfo>) {}
void StmtVisitor::visit(const NodeStmtFor& sfor, Box<VisitInfo>) {}
void StmtVisitor::visit(const NodeStmtForEach& sfeach, Box<VisitInfo>) {}
void StmtVisitor::visit(const NodeStmtWhile& swhl, Box<VisitInfo>) {}
void StmtVisitor::visit(const NodeStmtAssign& sasgn, Box<VisitInfo>) {}
void StmtVisitor::visit(const NodeStmtEmpty& semt, Box<VisitInfo>) {}
void StmtVisitor::visit(const NodeStmtExpr& sexpr, Box<VisitInfo>) {}

}  // namespace gen

}  // namespace via
