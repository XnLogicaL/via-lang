// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "expr_visitor.h"
#include "generator.h"
#include "sema/const_value.h"
#include "sema/constexpr.h"

namespace via {

namespace gen {

using sema::ConstValue;
using enum Diagnosis::Kind;

void ExprVisitor::visit(const ast::NodeExprLit& elit, u16 dst) {
  if (auto kval = ConstValue::from_literal_token(*elit.tok)) {
    u16 kp;
    ctx.emit_constant(std::move(*kval), &kp);
    ctx.emit_instruction(Opcode::LOADK, {dst, kp});
    return;
  }

  VIA_BUG("bad token literal");
}

void ExprVisitor::visit(const ast::NodeExprSym& esym, u16 dst) {
  String symbol = esym.tok->to_string();
  if (auto local = ctx.stack.top().find_local(symbol)) {
    ctx.emit_instruction(Opcode::GETLOCALREF, {dst, local->id});
    return;
  }

  ctx.diags.diagnosef<Error>(esym.loc, "Unknown symbol '{}'", symbol);
}

void ExprVisitor::visit(const ast::NodeExprUn& eun, u16 dst) {}
void ExprVisitor::visit(const ast::NodeExprBin& ebin, u16 dst) {}
void ExprVisitor::visit(const ast::NodeExprGroup& egrp, u16 dst) {}
void ExprVisitor::visit(const ast::NodeExprCall& ecall, u16 dst) {}
void ExprVisitor::visit(const ast::NodeExprSubs& esubs, u16 dst) {}
void ExprVisitor::visit(const ast::NodeExprTuple& etup, u16 dst) {}
void ExprVisitor::visit(const ast::NodeExprLambda& elam, u16 dst) {}

}  // namespace gen

}  // namespace via
