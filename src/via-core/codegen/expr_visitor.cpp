// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "expr_visitor.h"
#include "debug.h"
#include "generator.h"
#include "sema/const_value.h"
#include "sema/constexpr.h"

namespace via {

namespace gen {

using sema::ConstValue;
using enum Diagnosis::Kind;
using namespace ast;

void ExprVisitor::visit(const NodeExprLit& elit, Box<VisitInfo> vi) {
  if (auto kval = ConstValue::from_literal_token(*elit.tok)) {
    u16 kp;
    m_ctx.emit_constant(std::move(*kval), &kp);
    m_ctx.emit_instruction(Opcode::LOADK, {vi->dst, kp});
    return;
  }

  bug("bad token literal");
}

void ExprVisitor::visit(const NodeExprSym& esym, Box<VisitInfo> vi) {
  sema::Context& sema_ctx = m_ctx.get_sema_context();
  sema::Frame& frame = sema_ctx.stack.top();

  String symbol = esym.tok->to_string();

  if (auto lref = sema_ctx.stack.top().get_local(symbol)) {
    m_ctx.emit_instruction(Opcode::GETLOCAL, {vi->dst, lref->id});
    return;
  }

  m_ctx.get_diagnostics().diagnosef<Error>(esym.loc, "Unknown symbol '{}'",
                                           symbol);
}

void ExprVisitor::visit(const NodeExprUn& eun, Box<VisitInfo> vi) {}
void ExprVisitor::visit(const NodeExprBin& ebin, Box<VisitInfo> vi) {}
void ExprVisitor::visit(const NodeExprGroup& egrp, Box<VisitInfo> vi) {}
void ExprVisitor::visit(const NodeExprCall& ecall, Box<VisitInfo> vi) {}
void ExprVisitor::visit(const NodeExprSubs& esubs, Box<VisitInfo> vi) {}
void ExprVisitor::visit(const NodeExprTuple& etup, Box<VisitInfo> vi) {}
void ExprVisitor::visit(const NodeExprLambda& elam, Box<VisitInfo> vi) {}

}  // namespace gen

}  // namespace via
