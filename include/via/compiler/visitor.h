// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_VISITOR_H
#define _VIA_VISITOR_H

#include "ast.h"
#include "register-allocator.h"
#include "token.h"
#include "common.h"
#include "bytecode.h"
#include "constant.h"
#include "error-bus.h"

#define INVALID_VISIT                                                                              \
  {                                                                                                \
    VIA_ASSERT(false, "invalid visit");                                                            \
  }

#define CHECK_TYPE_INFERENCE_FAILURE(type, expr)                                                   \
  if (!type.get()) {                                                                               \
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
VIA_NAMESPACE_BEGIN

using Label = Operand;

TValue construct_constant(LiteralExprNode&);

class NodeVisitor {
public:
  NodeVisitor(TransUnitContext& unit_ctx, ErrorBus& bus)
    : unit_ctx(unit_ctx),
      err_bus(bus) {}

  virtual VIA_DEFAULT_DESTRUCTOR(NodeVisitor);

  // Expression visitors
  virtual void visit(LiteralExprNode&, Operand) INVALID_VISIT;
  virtual void visit(SymbolExprNode&, Operand) INVALID_VISIT;
  virtual void visit(UnaryExprNode&, Operand) INVALID_VISIT;
  virtual void visit(GroupExprNode&, Operand) INVALID_VISIT;
  virtual void visit(CallExprNode&, Operand) INVALID_VISIT;
  virtual void visit(IndexExprNode&, Operand) INVALID_VISIT;
  virtual void visit(BinaryExprNode&, Operand) INVALID_VISIT;
  virtual void visit(TypeCastExprNode&, Operand) INVALID_VISIT;

  // Type visitors (return type is due to type-decaying)
  virtual pTypeNode visit(AutoTypeNode&) INVALID_VISIT;
  virtual pTypeNode visit(GenericTypeNode&) INVALID_VISIT;
  virtual pTypeNode visit(UnionTypeNode&) INVALID_VISIT;
  virtual pTypeNode visit(FunctionTypeNode&) INVALID_VISIT;

  // Statement visitors
  virtual void visit(DeclarationStmtNode&) INVALID_VISIT;
  virtual void visit(ScopeStmtNode&) INVALID_VISIT;
  virtual void visit(FunctionStmtNode&) INVALID_VISIT;
  virtual void visit(AssignStmtNode&) INVALID_VISIT;
  virtual void visit(IfStmtNode&) INVALID_VISIT;
  virtual void visit(ReturnStmtNode&) INVALID_VISIT;
  virtual void visit(BreakStmtNode&) INVALID_VISIT;
  virtual void visit(ContinueStmtNode&) INVALID_VISIT;
  virtual void visit(WhileStmtNode&) INVALID_VISIT;
  virtual void visit(ExprStmtNode&) INVALID_VISIT;

  virtual inline bool failed() {
    return visitor_failed;
  }

protected:
  bool visitor_failed = false;

  TransUnitContext& unit_ctx;

  ErrorBus& err_bus;

protected:
  void compiler_error(size_t begin, size_t end, const std::string&);
  void compiler_error(const Token&, const std::string&);
  void compiler_error(const std::string&);

  void compiler_warning(size_t begin, size_t end, const std::string&);
  void compiler_warning(const Token&, const std::string&);
  void compiler_warning(const std::string&);

  void compiler_info(size_t begin, size_t end, const std::string&);
  void compiler_info(const Token&, const std::string&);
  void compiler_info(const std::string&);
};

#undef INVALID_VISIT

class ExprVisitor : public NodeVisitor {
public:
  ExprVisitor(TransUnitContext& unit_ctx, ErrorBus& bus, RegisterAllocator& allocator)
    : NodeVisitor(unit_ctx, bus),
      allocator(allocator) {}

  void visit(LiteralExprNode&, Operand) override;
  void visit(SymbolExprNode&, Operand) override;
  void visit(UnaryExprNode&, Operand) override;
  void visit(GroupExprNode&, Operand) override;
  void visit(CallExprNode&, Operand) override;
  void visit(IndexExprNode&, Operand) override;
  void visit(BinaryExprNode&, Operand) override;
  void visit(TypeCastExprNode&, Operand) override;

private:
  RegisterAllocator& allocator;
};

class DecayVisitor : public NodeVisitor {
public:
  DecayVisitor(TransUnitContext& unit_ctx, ErrorBus& bus)
    : NodeVisitor(unit_ctx, bus) {}

  pTypeNode visit(AutoTypeNode&) override;
  pTypeNode visit(GenericTypeNode&) override;
  pTypeNode visit(UnionTypeNode&) override;
  pTypeNode visit(FunctionTypeNode&) override;
};

class TypeVisitor : public NodeVisitor {
public:
  TypeVisitor(TransUnitContext& unit_ctx, ErrorBus& bus)
    : NodeVisitor(unit_ctx, bus) {}

  void visit(DeclarationStmtNode&) override;
  void visit(AssignStmtNode&) override;
  void visit(FunctionStmtNode&) override;
};

class StmtVisitor : public NodeVisitor {
public:
  StmtVisitor(TransUnitContext& unit_ctx, ErrorBus& bus, RegisterAllocator& allocator)
    : NodeVisitor(unit_ctx, bus),
      allocator(allocator),
      expression_visitor(unit_ctx, bus, allocator),
      decay_visitor(unit_ctx, bus),
      type_visitor(unit_ctx, bus) {}

  void visit(DeclarationStmtNode&) override;
  void visit(ScopeStmtNode&) override;
  void visit(FunctionStmtNode&) override;
  void visit(AssignStmtNode&) override;
  void visit(IfStmtNode&) override;
  void visit(ReturnStmtNode&) override;
  void visit(BreakStmtNode&) override;
  void visit(ContinueStmtNode&) override;
  void visit(WhileStmtNode&) override;
  void visit(ExprStmtNode&) override;

  inline bool failed() override {
    return visitor_failed || expression_visitor.failed() || decay_visitor.failed() ||
           type_visitor.failed();
  }

private:
  RegisterAllocator& allocator;

  ExprVisitor expression_visitor;
  DecayVisitor decay_visitor;
  TypeVisitor type_visitor;

  std::optional<Label> escape_label = std::nullopt;
  std::optional<Label> repeat_label = std::nullopt;
};

VIA_NAMESPACE_END

#endif
