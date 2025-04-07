//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================
#include "visitor.h"
#include "compiler-types.h"
#include <cmath>

namespace via {

using enum comp_err_lvl;

// Helper function to return line and column number from a character offset
std::pair<size_t, size_t> get_line_and_column(const std::string& source, size_t offset) {
  size_t line = 1;
  size_t column = 1;

  // Iterate over the string until we reach the offset
  for (size_t i = 0; i < offset; ++i) {
    if (source[i] == '\n') {
      ++line;
      column = 1; // Reset column to 1 for a new line
    }
    else {
      ++column;
    }
  }

  return {line, column};
}

value_obj expr_node_visitor::construct_constant(lit_expr_node& literal_node) {
  using enum value_type;
  return std::visit(
    [](auto&& val) -> value_obj {
      using T = std::decay_t<decltype(val)>;

      if constexpr (std::is_same_v<T, int>) {
        return value_obj(val);
      }
      else if constexpr (std::is_same_v<T, bool>) {
        return value_obj(val);
      }
      else if constexpr (std::is_same_v<T, float>) {
        return value_obj(val);
      }
      else if constexpr (std::is_same_v<T, std::string>) {
        return value_obj(val.data());
      }
      else if constexpr (std::is_same_v<T, std::monostate>) {
        return value_obj();
      }

      VIA_UNREACHABLE();
    },
    literal_node.value
  );
}

#if VIA_COMPILER == C_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winfinite-recursion"
#endif

lit_expr_node expr_node_visitor::fold_constant(expr_node_base& expr, size_t fold_depth) {
  using enum token_type;
  using evaluator_2i_t = std::function<int(int, int)>;
  using evaluator_2f_t = std::function<float(float, float)>;
  using evaluator_fi_t = std::function<float(float, int)>;

  static const std::unordered_map<token_type, evaluator_2i_t> evaluators_2i = {
    {OP_ADD, [](int a, int b) { return a + b; }},
    {OP_SUB, [](int a, int b) { return a - b; }},
    {OP_MUL, [](int a, int b) { return a * b; }},
    {OP_DIV, [](int a, int b) { return a / b; }},
    {OP_EXP, [](int a, int b) { return std::pow(a, b); }},
    {OP_MOD, [](int a, int b) { return a % b; }},
  };

  static const std::unordered_map<token_type, evaluator_2f_t> evaluators_2f = {
    {OP_ADD, [](float a, float b) { return a + b; }},
    {OP_SUB, [](float a, float b) { return a - b; }},
    {OP_MUL, [](float a, float b) { return a * b; }},
    {OP_DIV, [](float a, float b) { return a / b; }},
    {OP_EXP, [](float a, float b) { return std::pow(a, b); }},
    {OP_MOD, [](float a, float b) { return std::fmod(a, b); }},
  };

  static const std::unordered_map<token_type, evaluator_fi_t> evaluators_fi = {
    {OP_ADD, [](float a, int b) { return a + b; }},
    {OP_SUB, [](float a, int b) { return a - b; }},
    {OP_MUL, [](float a, int b) { return a * b; }},
    {OP_DIV, [](float a, int b) { return a / b; }},
    {OP_EXP, [](float a, int b) { return std::pow(a, b); }},
    {OP_MOD, [](float a, int b) { return std::fmod(a, b); }},
  };

  if (lit_expr_node* lit_expr = get_derived_instance<expr_node_base, lit_expr_node>(expr)) {
    return *get_derived_instance<expr_node_base, lit_expr_node>(*lit_expr->clone());
  }
  if (bin_expr_node* bin_expr = get_derived_instance<expr_node_base, bin_expr_node>(expr)) {
    lit_expr_node left = fold_constant(*bin_expr->lhs_expression, fold_depth + 1);
    lit_expr_node right = fold_constant(*bin_expr->rhs_expression, fold_depth + 1);

    if (int* int_left = std::get_if<int>(&left.value)) {
      if (int* int_right = std::get_if<int>(&right.value)) {
        evaluator_2i_t evaluator = evaluators_2i.at(bin_expr->op.type);
        return lit_expr_node(token(), evaluator(*int_left, *int_right));
      }
      else if (float* float_right = std::get_if<float>(&right.value)) {
        evaluator_fi_t evaluator = evaluators_fi.at(bin_expr->op.type);
        return lit_expr_node(token(), evaluator(*float_right, *int_left));
      }

      p_type_node_t left_type = left.infer_type(unit_ctx);
      p_type_node_t right_type = right.infer_type(unit_ctx);

      VIA_ASSERT(left_type.get() && right_type.get(), "!!tmp!! inference failed");

      compiler_error(
        expr.begin,
        expr.end,
        std::format(
          "Constant binary expression on incompatible types {} and {}",
          left_type->to_output_string(),
          right_type->to_output_string()
        )
      );
      goto bad_fold;
    }
    else if (float* float_left = std::get_if<float>(&left.value)) {
      if (float* float_right = std::get_if<float>(&right.value)) {
        evaluator_2f_t evaluator = evaluators_2f.at(bin_expr->op.type);
        return lit_expr_node(token(), evaluator(*float_left, *float_right));
      }
      else if (int* int_right = std::get_if<int>(&right.value)) {
        evaluator_fi_t evaluator = evaluators_fi.at(bin_expr->op.type);
        return lit_expr_node(token(), evaluator(*float_left, *int_right));
      }

      p_type_node_t left_type = left.infer_type(unit_ctx);
      p_type_node_t right_type = right.infer_type(unit_ctx);

      VIA_ASSERT(left_type.get() && right_type.get(), "!!tmp!! inference failed");

      compiler_error(
        expr.begin,
        expr.end,
        std::format(
          "Constant binary expression on incompatible types {} and {}",
          left_type->to_output_string(),
          right_type->to_output_string()
        )
      );
      goto bad_fold;
    }
  }
  else if (sym_expr_node* sym_expr = get_derived_instance<expr_node_base, sym_expr_node>(expr)) {
    auto stk_id = unit_ctx.internal.variable_stack->find_symbol(sym_expr->identifier.lexeme);
    if (!stk_id.has_value()) {
      goto bad_fold;
    }

    auto var_obj = unit_ctx.internal.variable_stack->at(*stk_id);

    // Check if call exceeds variable depth limit
    if (fold_depth > 5) {
      compiler_error(expr.begin, expr.end, "Constant fold variable depth exceeded");
      compiler_info("This error message likely indicates an internal compiler bug. Please create "
                    "an issue at https://github.com/XnLogicaL/via-lang.");
      goto bad_fold;
    }

    return fold_constant(*var_obj->value, ++fold_depth);
  }

bad_fold:
  return lit_expr_node(token(), std::monostate());
}

#if VIA_COMPILER == C_GCC
#pragma GCC diagnostic pop
#endif

void node_visitor_base::compiler_error(size_t begin, size_t end, const std::string& message) {
  auto lc_info = get_line_and_column(unit_ctx.file_source, begin);

  visitor_failed = true;
  err_bus.log({false, message, unit_ctx, ERROR_, {lc_info.first, lc_info.second, begin, end}});
}

void node_visitor_base::compiler_error(const token& token, const std::string& message) {
  visitor_failed = true;
  err_bus.log({false, message, unit_ctx, ERROR_, token});
}

void node_visitor_base::compiler_error(const std::string& message) {
  visitor_failed = true;
  err_bus.log({true, message, unit_ctx, ERROR_, {}});
}

void node_visitor_base::compiler_warning(size_t begin, size_t end, const std::string& message) {
  auto lc_info = get_line_and_column(unit_ctx.file_source, begin);
  err_bus.log({false, message, unit_ctx, WARNING, {lc_info.first, lc_info.second, begin, end}});
}

void node_visitor_base::compiler_warning(const token& token, const std::string& message) {
  err_bus.log({false, message, unit_ctx, WARNING, token});
}

void node_visitor_base::compiler_warning(const std::string& message) {
  err_bus.log({true, message, unit_ctx, WARNING, {}});
}

void node_visitor_base::compiler_info(size_t begin, size_t end, const std::string& message) {
  auto lc_info = get_line_and_column(unit_ctx.file_source, begin);
  err_bus.log({false, message, unit_ctx, INFO, {lc_info.first, lc_info.second, begin, end}});
}

void node_visitor_base::compiler_info(const token& token, const std::string& message) {
  err_bus.log({false, message, unit_ctx, INFO, token});
}

void node_visitor_base::compiler_info(const std::string& message) {
  err_bus.log({true, message, unit_ctx, INFO, {}});
}

} // namespace via
