// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

/**
 * @file visitor.h
 * @brief Declares visitor interfaces for the Abstract Syntax Tree (AST) used in the via compiler.
 *
 * This file provides a hierarchy of visitor classes that traverse or transform the AST of the via
 * programming language. These include visitors for expression evaluation, type inference,
 * and statement processing. The `VisitorContext` struct stores shared visitor state, including
 * register allocation and error tracking.
 */

#ifndef VIA_HAS_HEADER_VISITOR_H
#define VIA_HAS_HEADER_VISITOR_H

#include "common.h"
#include "error_bus.h"
#include "context.h"
#include "token.h"
#include "ast.h"
#include "state.h"
#include "bits.h"
#include "memory.h"
#include "ustring.h"
#include "color.h"
#include "sema.h"

/**
 * @brief Fails with an compiler error if type inference fails.
 * @internal
 * @todo Remove this once type inference is fully stable.
 */
#define VIA_CHECK_INFERED(type, expr)                                                              \
  if (!type) {                                                                                     \
    error(ctx, expr->begin, expr->end, "Expression type could not be infered");                    \
    info(                                                                                          \
      ctx,                                                                                         \
      "This message indicates a likely compiler bug. Please report it at "                         \
      "https://github.com/XnLogicaL/via-lang"                                                      \
    );                                                                                             \
    return;                                                                                        \
  }

/// @namespace via
/// @brief All core compiler logic is within this namespace.
namespace via {

/**
 * @typedef label_t operand_t
 * @brief Alias for instruction operand type used for jump labels.
 */
using label_t = operand_t;

/**
 * @struct VisitorContext
 * @brief Encapsulates state for visitor objects during AST traversal.
 */
struct VisitorContext {
  /// Optional label type used for break/continue tracking.
  using opt_label_t = std::optional<label_t>;

  bool failed = false;             ///< Visitor failure status flag.
  size_t errc = 0;                 ///< Error count, reflecting errors in CErrorBus.
  operand_t args = 0;              ///< Argument register index (head).
  opt_label_t lesc = std::nullopt; ///< Label for break/escape control flow.
  opt_label_t lrep = std::nullopt; ///< Label for continue/repeat control flow.

  Context& lctx;               ///< Translation unit context.
  RegisterAllocator reg_alloc; ///< Register allocator for code generation.
  CErrorBus err_bus;           ///< Local error bus for tracking visitor errors.

  /// @brief Constructs a visitor context from a translation unit.
  inline VisitorContext(Context& ctx)
    : lctx(ctx),
      reg_alloc(REGISTER_COUNT, true) {}
};

/**
 * @brief Default implementation for unimplemented visit methods.
 * @internal
 */
#define INVALID_VISIT                                                                              \
  { VIA_ASSERT(false, "invalid visit"); }

/**
 * @class NodeVisitorBase
 * @brief Abstract base class for all AST visitors. Provides virtual `visit` methods for every AST
 * node type.
 */
class NodeVisitorBase {
public:
  explicit NodeVisitorBase(VisitorContext& ctx)
    : ctx(ctx) {}

  virtual ~NodeVisitorBase() = default;

  // --- Expression Visitors ---
  virtual void visit(AstNode*, NodeLitExpr&, operand_t) INVALID_VISIT;
  virtual void visit(AstNode*, NodeSymExpr&, operand_t) INVALID_VISIT;
  virtual void visit(AstNode*, NodeUnExpr&, operand_t) INVALID_VISIT;
  virtual void visit(AstNode*, NodeGroupExpr&, operand_t) INVALID_VISIT;
  virtual void visit(AstNode*, NodeCallExpr&, operand_t) INVALID_VISIT;
  virtual void visit(AstNode*, NodeIndexExpr&, operand_t) INVALID_VISIT;
  virtual void visit(AstNode*, NodeBinExpr&, operand_t) INVALID_VISIT;
  virtual void visit(AstNode*, NodeCastExpr&, operand_t) INVALID_VISIT;
  virtual void visit(AstNode*, NodeStepExpr&, operand_t) INVALID_VISIT;
  virtual void visit(AstNode*, NodeArrExpr&, operand_t) INVALID_VISIT;
  virtual void visit(AstNode*, NodeIntrExpr&, operand_t) INVALID_VISIT;

  // --- Type Visitors ---
  virtual AstNode* visit(AstNode*, NodeGenType&) INVALID_VISIT;
  virtual AstNode* visit(AstNode*, NodeUnionType&) INVALID_VISIT;
  virtual AstNode* visit(AstNode*, NodeFuncType&) INVALID_VISIT;
  virtual AstNode* visit(AstNode*, NodeArrType&) INVALID_VISIT;

  // --- Statement Visitors ---
  virtual void visit(AstNode*, NodeDeclStmt&) INVALID_VISIT;
  virtual void visit(AstNode*, NodeScopeStmt&) INVALID_VISIT;
  virtual void visit(AstNode*, NodeFuncDeclStmt&) INVALID_VISIT;
  virtual void visit(AstNode*, NodeAsgnStmt&) INVALID_VISIT;
  virtual void visit(AstNode*, NodeIfStmt&) INVALID_VISIT;
  virtual void visit(AstNode*, NodeRetStmt&) INVALID_VISIT;
  virtual void visit(AstNode*, NodeWhileStmt&) INVALID_VISIT;
  virtual void visit(AstNode*, NodeDeferStmt&) INVALID_VISIT;
  virtual void visit(AstNode*, NodeExprStmt&) INVALID_VISIT;

  /// @brief Indicates if the visitor has failed.
  virtual inline bool failed() {
    return ctx.failed;
  }

protected:
  VisitorContext& ctx; ///< Shared visitor context.
};

#undef INVALID_VISIT

/**
 * @class ExprNodeVisitor
 * @brief AST visitor for expression nodes only.
 */
class ExprNodeVisitor : public NodeVisitorBase {
public:
  using NodeVisitorBase::NodeVisitorBase;

  void visit(AstNode*, NodeLitExpr&, operand_t) override;
  void visit(AstNode*, NodeSymExpr&, operand_t) override;
  void visit(AstNode*, NodeUnExpr&, operand_t) override;
  void visit(AstNode*, NodeGroupExpr&, operand_t) override;
  void visit(AstNode*, NodeCallExpr&, operand_t) override;
  void visit(AstNode*, NodeIndexExpr&, operand_t) override;
  void visit(AstNode*, NodeBinExpr&, operand_t) override;
  void visit(AstNode*, NodeCastExpr&, operand_t) override;
  void visit(AstNode*, NodeStepExpr&, operand_t) override;
  void visit(AstNode*, NodeArrExpr&, operand_t) override;
  void visit(AstNode*, NodeIntrExpr&, operand_t) override;
};

/**
 * @class DecayNodeVisitor
 * @brief AST visitor that resolves type nodes to their underlying forms.
 */
class DecayNodeVisitor : public NodeVisitorBase {
public:
  using NodeVisitorBase::NodeVisitorBase;

  AstNode* visit(AstNode*, NodeGenType&) override;
  AstNode* visit(AstNode*, NodeUnionType&) override;
  AstNode* visit(AstNode*, NodeFuncType&) override;
  AstNode* visit(AstNode*, NodeArrType&) override;
};

/**
 * @class TypeNodeVisitor
 * @brief AST visitor for analyzing or transforming statements relevant to type resolution.
 */
class TypeNodeVisitor : public NodeVisitorBase {
public:
  using NodeVisitorBase::NodeVisitorBase;

  void visit(AstNode*, NodeDeclStmt&) override;
  void visit(AstNode*, NodeAsgnStmt&) override;
  void visit(AstNode*, NodeFuncDeclStmt&) override;
};

/**
 * @class StmtNodeVisitor
 * @brief AST visitor for statement nodes. Composes expression, decay, and type visitors.
 */
class StmtNodeVisitor : public NodeVisitorBase {
public:
  explicit StmtNodeVisitor(VisitorContext& ctx)
    : NodeVisitorBase(ctx),
      expression_visitor(ctx),
      decay_visitor(ctx),
      type_visitor(ctx) {}

  void visit(AstNode*, NodeDeclStmt&) override;
  void visit(AstNode*, NodeScopeStmt&) override;
  void visit(AstNode*, NodeFuncDeclStmt&) override;
  void visit(AstNode*, NodeAsgnStmt&) override;
  void visit(AstNode*, NodeIfStmt&) override;
  void visit(AstNode*, NodeRetStmt&) override;
  void visit(AstNode*, NodeWhileStmt&) override;
  void visit(AstNode*, NodeDeferStmt&) override;
  void visit(AstNode*, NodeExprStmt&) override;

  /// @brief Checks whether any of the sub-visitors encountered a failure.
  inline bool failed() override {
    return ctx.failed || expression_visitor.failed() || decay_visitor.failed()
      || type_visitor.failed();
  }

private:
  ExprNodeVisitor expression_visitor; ///< Visitor for expression nodes.
  DecayNodeVisitor decay_visitor;     ///< Visitor for decaying type nodes.
  TypeNodeVisitor type_visitor;       ///< Visitor for statements that affect type inference.
};

} // namespace via

#endif // VIA_HAS_HEADER_VISITOR_H
