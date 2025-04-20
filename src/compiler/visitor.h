// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_VISITOR_H
#define VIA_HAS_HEADER_VISITOR_H

#include "common.h"
#include "error-bus.h"
#include "context.h"
#include "register.h"
#include "stack.h"
#include "bytecode.h"
#include "constant.h"

#include <lex/token.h>
#include <parse/ast.h>
#include <interpreter/state.h>
#include <utility/bits.h>
#include <utility/memory.h>
#include <utility/ustring.h>
#include <utility/color.h>

#define VIA_INVALID_VISIT                                                                          \
  {                                                                                                \
    VIA_ASSERT(false, "invalid visit");                                                            \
  }

#define CHECK_INFERED_TYPE(type, expr)                                                             \
  if (!type) {                                                                                     \
    compiler_error(ctx, expr->begin, expr->end, "Expression type could not be infered");           \
    compiler_info(                                                                                 \
      ctx,                                                                                         \
      "This error message likely indicates an internal compiler bug. Please create an "            \
      "issue "                                                                                     \
      "at https://github.com/XnLogicaL/via-lang"                                                   \
    );                                                                                             \
    return;                                                                                        \
  }

// =============================================================================================
// visitor.h
//
namespace via {

using label_t = operand_t;
using unused_expression_handler_t = std::function<void(const ExprStmtNode&)>;

struct VisitorContext {
  using opt_label_t = std::optional<label_t>;

  bool failed = false; // Fail status
  size_t errc = 0;     // Error count

  opt_label_t lesc = std::nullopt; // Escape label
  opt_label_t lrep = std::nullopt; // Repeat label

  TransUnitContext& unit_ctx;
  RegisterAllocator reg_alloc;
  CErrorBus err_bus;

  inline VisitorContext(TransUnitContext& ctx)
    : unit_ctx(ctx),
      reg_alloc(REGISTER_COUNT, true) {}
};

class NodeVisitorBase {
public:
  NodeVisitorBase(VisitorContext& ctx)
    : ctx(ctx) {}

  virtual ~NodeVisitorBase() = default;

  // Expression visitors
  virtual void visit(LitExprNode&, operand_t) VIA_INVALID_VISIT;
  virtual void visit(SymExprNode&, operand_t) VIA_INVALID_VISIT;
  virtual void visit(UnaryExprNode&, operand_t) VIA_INVALID_VISIT;
  virtual void visit(GroupExprNode&, operand_t) VIA_INVALID_VISIT;
  virtual void visit(CallExprNode&, operand_t) VIA_INVALID_VISIT;
  virtual void visit(IndexExprNode&, operand_t) VIA_INVALID_VISIT;
  virtual void visit(BinExprNode&, operand_t) VIA_INVALID_VISIT;
  virtual void visit(CastExprNode&, operand_t) VIA_INVALID_VISIT;
  virtual void visit(StepExprNode&, operand_t) VIA_INVALID_VISIT;
  virtual void visit(ArrayExprNode&, operand_t) VIA_INVALID_VISIT;

  // Type visitors (return type is due to type-decaying)
  virtual TypeNodeBase* visit(AutoTypeNode&) VIA_INVALID_VISIT;
  virtual TypeNodeBase* visit(GenericTypeNode&) VIA_INVALID_VISIT;
  virtual TypeNodeBase* visit(UnionTypeNode&) VIA_INVALID_VISIT;
  virtual TypeNodeBase* visit(FunctionTypeNode&) VIA_INVALID_VISIT;
  virtual TypeNodeBase* visit(ArrayTypeNode&) VIA_INVALID_VISIT;

  // Statement visitors
  virtual void visit(DeclStmtNode&) VIA_INVALID_VISIT;
  virtual void visit(ScopeStmtNode&) VIA_INVALID_VISIT;
  virtual void visit(FuncDeclStmtNode&) VIA_INVALID_VISIT;
  virtual void visit(AssignStmtNode&) VIA_INVALID_VISIT;
  virtual void visit(IfStmtNode&) VIA_INVALID_VISIT;
  virtual void visit(ReturnStmtNode&) VIA_INVALID_VISIT;
  virtual void visit(BreakStmtNode&) VIA_INVALID_VISIT;
  virtual void visit(ContinueStmtNode&) VIA_INVALID_VISIT;
  virtual void visit(WhileStmtNode&) VIA_INVALID_VISIT;
  virtual void visit(DeferStmtNode&) VIA_INVALID_VISIT;
  virtual void visit(ExprStmtNode&) VIA_INVALID_VISIT;

  virtual inline bool failed() {
    return ctx.failed;
  }

protected:
  VisitorContext& ctx;
};

#undef VIA_INVALID_VISIT

class ExprNodeVisitor : public NodeVisitorBase {
public:
  using NodeVisitorBase::NodeVisitorBase;

  void visit(LitExprNode&, operand_t) override;
  void visit(SymExprNode&, operand_t) override;
  void visit(UnaryExprNode&, operand_t) override;
  void visit(GroupExprNode&, operand_t) override;
  void visit(CallExprNode&, operand_t) override;
  void visit(IndexExprNode&, operand_t) override;
  void visit(BinExprNode&, operand_t) override;
  void visit(CastExprNode&, operand_t) override;
  void visit(StepExprNode&, operand_t) override;
  void visit(ArrayExprNode&, operand_t) override;
};

class DecayNodeVisitor : public NodeVisitorBase {
public:
  using NodeVisitorBase::NodeVisitorBase;

  TypeNodeBase* visit(AutoTypeNode&) override;
  TypeNodeBase* visit(GenericTypeNode&) override;
  TypeNodeBase* visit(UnionTypeNode&) override;
  TypeNodeBase* visit(FunctionTypeNode&) override;
  TypeNodeBase* visit(ArrayTypeNode&) override;
};

class TypeNodeVisitor : public NodeVisitorBase {
public:
  using NodeVisitorBase::NodeVisitorBase;

  void visit(DeclStmtNode&) override;
  void visit(AssignStmtNode&) override;
  void visit(FuncDeclStmtNode&) override;
};

class StmtNodeVisitor : public NodeVisitorBase {
public:
  StmtNodeVisitor(VisitorContext& ctx)
    : NodeVisitorBase(ctx),
      expression_visitor(ctx),
      decay_visitor(ctx),
      type_visitor(ctx) {}

  void visit(DeclStmtNode&) override;
  void visit(ScopeStmtNode&) override;
  void visit(FuncDeclStmtNode&) override;
  void visit(AssignStmtNode&) override;
  void visit(IfStmtNode&) override;
  void visit(ReturnStmtNode&) override;
  void visit(BreakStmtNode&) override;
  void visit(ContinueStmtNode&) override;
  void visit(WhileStmtNode&) override;
  void visit(DeferStmtNode&) override;
  void visit(ExprStmtNode&) override;

  inline bool failed() override {
    return ctx.failed || expression_visitor.failed() || decay_visitor.failed()
      || type_visitor.failed();
  }

public:
  std::optional<unused_expression_handler_t> unused_expr_handler;

private:
  ExprNodeVisitor expression_visitor;
  DecayNodeVisitor decay_visitor;
  TypeNodeVisitor type_visitor;
};

} // namespace via

#endif
