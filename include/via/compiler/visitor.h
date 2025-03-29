// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef vl_has_header_visitor_h
#define vl_has_header_visitor_h

#include "ast.h"
#include "register-allocator.h"
#include "token.h"
#include "common.h"
#include "bytecode.h"
#include "constant.h"
#include "error-bus.h"

#define vl_invalid_visit                                                                           \
  {                                                                                                \
    vl_assert(false, "invalid visit");                                                             \
  }

#define vl_tinference_failure(type, expr)                                                          \
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
namespace via {

using label_t = operand_t;

value_obj construct_constant(lit_expr_node&);

class node_visitor_base {
public:
  node_visitor_base(trans_unit_context& unit_ctx, error_bus& bus)
    : unit_ctx(unit_ctx),
      err_bus(bus) {}

  virtual vl_defdestructor(node_visitor_base);

  // Expression visitors
  virtual void visit(lit_expr_node&, operand_t) vl_invalid_visit;
  virtual void visit(sym_expr_node&, operand_t) vl_invalid_visit;
  virtual void visit(unary_expr_node&, operand_t) vl_invalid_visit;
  virtual void visit(grp_expr_node&, operand_t) vl_invalid_visit;
  virtual void visit(call_expr_node&, operand_t) vl_invalid_visit;
  virtual void visit(index_expr_node&, operand_t) vl_invalid_visit;
  virtual void visit(bin_expr_node&, operand_t) vl_invalid_visit;
  virtual void visit(cast_expr_node&, operand_t) vl_invalid_visit;

  // Type visitors (return type is due to type-decaying)
  virtual p_type_node_t visit(auto_type_node&) vl_invalid_visit;
  virtual p_type_node_t visit(generic_type_node&) vl_invalid_visit;
  virtual p_type_node_t visit(union_type_node&) vl_invalid_visit;
  virtual p_type_node_t visit(FunctionTypeNode&) vl_invalid_visit;

  // Statement visitors
  virtual void visit(decl_stmt_node&) vl_invalid_visit;
  virtual void visit(scope_stmt_node&) vl_invalid_visit;
  virtual void visit(func_stmt_node&) vl_invalid_visit;
  virtual void visit(assign_stmt_node&) vl_invalid_visit;
  virtual void visit(if_stmt_node&) vl_invalid_visit;
  virtual void visit(return_stmt_node&) vl_invalid_visit;
  virtual void visit(break_stmt_node&) vl_invalid_visit;
  virtual void visit(continue_stmt_node&) vl_invalid_visit;
  virtual void visit(while_stmt_node&) vl_invalid_visit;
  virtual void visit(expr_stmt_node&) vl_invalid_visit;

  virtual inline bool failed() {
    return visitor_failed;
  }

protected:
  bool visitor_failed = false;

  trans_unit_context& unit_ctx;

  error_bus& err_bus;

protected:
  void compiler_error(size_t begin, size_t end, const std::string&);
  void compiler_error(const token&, const std::string&);
  void compiler_error(const std::string&);

  void compiler_warning(size_t begin, size_t end, const std::string&);
  void compiler_warning(const token&, const std::string&);
  void compiler_warning(const std::string&);

  void compiler_info(size_t begin, size_t end, const std::string&);
  void compiler_info(const token&, const std::string&);
  void compiler_info(const std::string&);
};

#undef vl_invalid_visit

class expr_node_visitor : public node_visitor_base {
public:
  expr_node_visitor(trans_unit_context& unit_ctx, error_bus& bus, register_allocator& allocator)
    : node_visitor_base(unit_ctx, bus),
      allocator(allocator) {}

  void visit(lit_expr_node&, operand_t) override;
  void visit(sym_expr_node&, operand_t) override;
  void visit(unary_expr_node&, operand_t) override;
  void visit(grp_expr_node&, operand_t) override;
  void visit(call_expr_node&, operand_t) override;
  void visit(index_expr_node&, operand_t) override;
  void visit(bin_expr_node&, operand_t) override;
  void visit(cast_expr_node&, operand_t) override;

private:
  register_allocator& allocator;
};

class decay_node_visitor : public node_visitor_base {
public:
  decay_node_visitor(trans_unit_context& unit_ctx, error_bus& bus)
    : node_visitor_base(unit_ctx, bus) {}

  p_type_node_t visit(auto_type_node&) override;
  p_type_node_t visit(generic_type_node&) override;
  p_type_node_t visit(union_type_node&) override;
  p_type_node_t visit(FunctionTypeNode&) override;
};

class type_node_visitor : public node_visitor_base {
public:
  type_node_visitor(trans_unit_context& unit_ctx, error_bus& bus)
    : node_visitor_base(unit_ctx, bus) {}

  void visit(decl_stmt_node&) override;
  void visit(assign_stmt_node&) override;
  void visit(func_stmt_node&) override;
};

class stmt_node_visitor : public node_visitor_base {
public:
  stmt_node_visitor(trans_unit_context& unit_ctx, error_bus& bus, register_allocator& allocator)
    : node_visitor_base(unit_ctx, bus),
      allocator(allocator),
      expression_visitor(unit_ctx, bus, allocator),
      decay_visitor(unit_ctx, bus),
      type_visitor(unit_ctx, bus) {}

  void visit(decl_stmt_node&) override;
  void visit(scope_stmt_node&) override;
  void visit(func_stmt_node&) override;
  void visit(assign_stmt_node&) override;
  void visit(if_stmt_node&) override;
  void visit(return_stmt_node&) override;
  void visit(break_stmt_node&) override;
  void visit(continue_stmt_node&) override;
  void visit(while_stmt_node&) override;
  void visit(expr_stmt_node&) override;

  inline bool failed() override {
    return visitor_failed || expression_visitor.failed() || decay_visitor.failed() ||
           type_visitor.failed();
  }

private:
  register_allocator& allocator;

  expr_node_visitor expression_visitor;
  decay_node_visitor decay_visitor;
  type_node_visitor type_visitor;

  std::optional<label_t> escape_label = std::nullopt;
  std::optional<label_t> repeat_label = std::nullopt;
};

} // namespace via

#endif
