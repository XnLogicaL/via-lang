//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================

#ifndef VIA_HAS_HEADER_VISITOR_H
#define VIA_HAS_HEADER_VISITOR_H

#include "ast.h"
#include "register-allocator.h"
#include "token.h"
#include "common.h"
#include "bytecode.h"
#include "constant.h"
#include "error-bus.h"

#define VIA_INVALID_VISIT                                                                          \
  {                                                                                                \
    VIA_ASSERT(false, "invalid visit");                                                            \
  }

#define VIA_TINFERENCE_FAILURE(type, expr)                                                         \
  if (!type) {                                                                                     \
    visitor_failed = true;                                                                         \
    compiler_error(expr->begin, expr->end, "Expression type could not be infered");                \
    compiler_info(                                                                                 \
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

class NodeVisitorBase {
public:
  NodeVisitorBase(TransUnitContext& unit_ctx, CErrorBus& bus)
    : unit_ctx(unit_ctx),
      err_bus(bus) {}

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
    return visitor_failed;
  }

  void compiler_error(size_t begin, size_t end, const std::string&);
  void compiler_error(const Token&, const std::string&);
  void compiler_error(const std::string&);

  void compiler_warning(size_t begin, size_t end, const std::string&);
  void compiler_warning(const Token&, const std::string&);
  void compiler_warning(const std::string&);

  void compiler_info(size_t begin, size_t end, const std::string&);
  void compiler_info(const Token&, const std::string&);
  void compiler_info(const std::string&);

  void compiler_output_end();
  size_t get_compiler_error_count();

protected:
  bool visitor_failed = false;
  size_t errors_generated = 0;

  TransUnitContext& unit_ctx;
  CErrorBus& err_bus;
};

#undef VIA_INVALID_VISIT

class ExprNodeVisitor : public NodeVisitorBase {
public:
  ExprNodeVisitor(TransUnitContext& unit_ctx, CErrorBus& bus, RegisterAllocator& allocator)
    : NodeVisitorBase(unit_ctx, bus),
      allocator(allocator) {}

  IValue construct_constant(LitExprNode&);
  LitExprNode fold_constant(ExprNodeBase&, size_t fold_depth = 0);

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

private:
  RegisterAllocator& allocator;
};

class DecayNodeVisitor : public NodeVisitorBase {
public:
  DecayNodeVisitor(TransUnitContext& unit_ctx, CErrorBus& bus)
    : NodeVisitorBase(unit_ctx, bus) {}

  TypeNodeBase* visit(AutoTypeNode&) override;
  TypeNodeBase* visit(GenericTypeNode&) override;
  TypeNodeBase* visit(UnionTypeNode&) override;
  TypeNodeBase* visit(FunctionTypeNode&) override;
  TypeNodeBase* visit(ArrayTypeNode&) override;
};

class type_node_visitor : public NodeVisitorBase {
public:
  type_node_visitor(TransUnitContext& unit_ctx, CErrorBus& bus)
    : NodeVisitorBase(unit_ctx, bus) {}

  void visit(DeclStmtNode&) override;
  void visit(AssignStmtNode&) override;
  void visit(FuncDeclStmtNode&) override;
};

class StmtNodeVisitor : public NodeVisitorBase {
public:
  StmtNodeVisitor(TransUnitContext& unit_ctx, CErrorBus& bus, RegisterAllocator& allocator)
    : NodeVisitorBase(unit_ctx, bus),
      allocator(allocator),
      expression_visitor(unit_ctx, bus, allocator),
      decay_visitor(unit_ctx, bus),
      type_visitor(unit_ctx, bus) {}

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
    return visitor_failed || expression_visitor.failed() || decay_visitor.failed()
      || type_visitor.failed();
  }

public:
  std::optional<unused_expression_handler_t> unused_expr_handler;

private:
  RegisterAllocator& allocator;

  ExprNodeVisitor expression_visitor;
  DecayNodeVisitor decay_visitor;
  type_node_visitor type_visitor;

  std::optional<label_t> escape_label = std::nullopt;
  std::optional<label_t> repeat_label = std::nullopt;
};

} // namespace via

#endif
