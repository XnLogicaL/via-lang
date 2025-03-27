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
  { VIA_ASSERT(false, "invalid visit"); }

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

TValue construct_constant(LiteralNode&);

class NodeVisitor {
public:
  NodeVisitor(TransUnitContext& unit_ctx, ErrorBus& bus)
      : unit_ctx(unit_ctx),
        err_bus(bus) {}

  virtual VIA_DEFAULT_DESTRUCTOR(NodeVisitor);

  // Expression visitors
  virtual void visit(LiteralNode&, Operand) INVALID_VISIT;
  virtual void visit(SymbolNode&, Operand) INVALID_VISIT;
  virtual void visit(UnaryNode&, Operand) INVALID_VISIT;
  virtual void visit(GroupNode&, Operand) INVALID_VISIT;
  virtual void visit(CallNode&, Operand) INVALID_VISIT;
  virtual void visit(IndexNode&, Operand) INVALID_VISIT;
  virtual void visit(BinaryNode&, Operand) INVALID_VISIT;
  virtual void visit(TypeCastNode&, Operand) INVALID_VISIT;

  // Type visitors (return type is due to type-decaying)
  virtual pTypeNode visit(AutoNode&) INVALID_VISIT;
  virtual pTypeNode visit(GenericNode&) INVALID_VISIT;
  virtual pTypeNode visit(UnionNode&) INVALID_VISIT;
  virtual pTypeNode visit(FunctionTypeNode&) INVALID_VISIT;

  // Statement visitors
  virtual void visit(DeclarationNode&) INVALID_VISIT;
  virtual void visit(ScopeNode&) INVALID_VISIT;
  virtual void visit(FunctionNode&) INVALID_VISIT;
  virtual void visit(AssignNode&) INVALID_VISIT;
  virtual void visit(IfNode&) INVALID_VISIT;
  virtual void visit(ReturnNode&) INVALID_VISIT;
  virtual void visit(BreakNode&) INVALID_VISIT;
  virtual void visit(ContinueNode&) INVALID_VISIT;
  virtual void visit(WhileNode&) INVALID_VISIT;
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

  void visit(LiteralNode&, Operand) override;
  void visit(SymbolNode&, Operand) override;
  void visit(UnaryNode&, Operand) override;
  void visit(GroupNode&, Operand) override;
  void visit(CallNode&, Operand) override;
  void visit(IndexNode&, Operand) override;
  void visit(BinaryNode&, Operand) override;
  void visit(TypeCastNode&, Operand) override;

private:
  RegisterAllocator& allocator;
};

class DecayVisitor : public NodeVisitor {
public:
  DecayVisitor(TransUnitContext& unit_ctx, ErrorBus& bus)
      : NodeVisitor(unit_ctx, bus) {}

  pTypeNode visit(AutoNode&) override;
  pTypeNode visit(GenericNode&) override;
  pTypeNode visit(UnionNode&) override;
  pTypeNode visit(FunctionTypeNode&) override;
};

class TypeVisitor : public NodeVisitor {
public:
  TypeVisitor(TransUnitContext& unit_ctx, ErrorBus& bus)
      : NodeVisitor(unit_ctx, bus) {}

  void visit(DeclarationNode&) override;
  void visit(AssignNode&) override;
  void visit(FunctionNode&) override;
};

class StmtVisitor : public NodeVisitor {
public:
  StmtVisitor(TransUnitContext& unit_ctx, ErrorBus& bus, RegisterAllocator& allocator)
      : NodeVisitor(unit_ctx, bus),
        allocator(allocator),
        expression_visitor(unit_ctx, bus, allocator),
        decay_visitor(unit_ctx, bus),
        type_visitor(unit_ctx, bus) {}

  void visit(DeclarationNode&) override;
  void visit(ScopeNode&) override;
  void visit(FunctionNode&) override;
  void visit(AssignNode&) override;
  void visit(IfNode&) override;
  void visit(ReturnNode&) override;
  void visit(BreakNode&) override;
  void visit(ContinueNode&) override;
  void visit(WhileNode&) override;
  void visit(ExprStmtNode&) override;

  inline bool failed() override {
    return visitor_failed || expression_visitor.failed() || decay_visitor.failed() ||
           type_visitor.failed();
  }

public:
  Label label_counter = 0;

private:
  RegisterAllocator& allocator;

  ExprVisitor  expression_visitor;
  DecayVisitor decay_visitor;
  TypeVisitor  type_visitor;

  std::optional<Label> escape_label = std::nullopt;
  std::optional<Label> repeat_label = std::nullopt;
};

VIA_NAMESPACE_END

#endif
