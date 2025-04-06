// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef VIA_HAS_HEADER_TYPES_H
#define VIA_HAS_HEADER_TYPES_H

#include "ast.h"
#include "common.h"
#include "object.h"
#include "stack.h"

namespace via {

template<typename T>
struct data_type;

template<>
struct data_type<std::monostate> {
  static constexpr value_type type = value_type::nil;
  static constexpr int precedence = -1;
};

template<>
struct data_type<TInteger> {
  static constexpr value_type type = value_type::integer;
  static constexpr int precedence = 1;
};

template<>
struct data_type<TFloat> {
  static constexpr value_type type = value_type::floating_point;
  static constexpr int precedence = 2;
};

template<>
struct data_type<bool> {
  static constexpr value_type type = value_type::boolean;
  static constexpr int precedence = -1;
};

template<>
struct data_type<std::string> {
  static constexpr value_type type = value_type::string;
  static constexpr int precedence = -1;
};

template<typename base, typename derived>
  requires std::is_base_of_v<base, derived>
derived* get_derived_instance(base& der) {
  return dynamic_cast<derived*>(&der);
}

template<typename base, typename derived>
  requires std::is_base_of_v<base, derived>
bool is_derived_instance(base& der) {
  return get_derived_instance<base, derived>(der) != nullptr;
}

VIA_IMPLEMENTATION bool is_constant_expression(
  trans_unit_context& unit_ctx, expr_node_base& expression, size_t variable_depth = 0
) {
  if (is_derived_instance<expr_node_base, lit_expr_node>(expression)) {
    return true;
  }
  else if (bin_expr_node* bin_expr =
             get_derived_instance<expr_node_base, bin_expr_node>(expression)) {
    return is_constant_expression(unit_ctx, *bin_expr->lhs_expression, variable_depth + 1)
      && is_constant_expression(unit_ctx, *bin_expr->rhs_expression, variable_depth + 1);
  }
  else if (sym_expr_node* sym_expr =
             get_derived_instance<expr_node_base, sym_expr_node>(expression)) {
    auto stk_id = unit_ctx.internal.variable_stack->find_symbol(sym_expr->identifier.lexeme);
    if (!stk_id.has_value()) {
      return false;
    }

    auto var_obj = unit_ctx.internal.variable_stack->at(*stk_id);

    // Check if call exceeds variable depth limit
    if (variable_depth > 5) {
      return false;
    }

    return is_constant_expression(unit_ctx, *var_obj->value, ++variable_depth);
  }

  return false;
}

VIA_IMPLEMENTATION bool is_nil(p_type_node_t& type) {
  using enum value_type;

  if (primitive_type_node* primitive =
        get_derived_instance<type_node_base, primitive_type_node>(*type)) {
    return primitive->type == nil;
  }

  return false;
}

VIA_IMPLEMENTATION bool is_integral(p_type_node_t& type) {
  using enum value_type;

  if (primitive_type_node* primitive =
        get_derived_instance<type_node_base, primitive_type_node>(*type)) {
    return primitive->type == integer;
  }

  // TODO: Add aggregate type support by checking for arithmetic meta-methods
  return false;
}

VIA_IMPLEMENTATION bool is_floating_point(p_type_node_t& type) {
  using enum value_type;

  if (primitive_type_node* primitive =
        get_derived_instance<type_node_base, primitive_type_node>(*type)) {
    return primitive->type == floating_point;
  }

  // TODO: Add aggregate type support by checking for arithmetic meta-methods
  return false;
}

VIA_IMPLEMENTATION bool is_arithmetic(p_type_node_t& type) {
  return is_integral(type) || is_floating_point(type);
}

VIA_IMPLEMENTATION bool is_callable(p_type_node_t& type) {
  if (is_derived_instance<type_node_base, function_type_node>(*type)) {
    return true;
  }

  return false;
}

VIA_IMPLEMENTATION bool is_compatible(p_type_node_t& left, p_type_node_t& right) {
  if (primitive_type_node* primitive_left =
        get_derived_instance<type_node_base, primitive_type_node>(*left)) {
    if (primitive_type_node* primitive_right =
          get_derived_instance<type_node_base, primitive_type_node>(*right)) {

      return primitive_left->type == primitive_right->type;
    }
  }

  return false;
}

VIA_IMPLEMENTATION bool is_castable(p_type_node_t& from, p_type_node_t& into) {
  if (primitive_type_node* primitive_right =
        get_derived_instance<type_node_base, primitive_type_node>(*into)) {
    if (get_derived_instance<type_node_base, primitive_type_node>(*from)) {
      if (primitive_right->type == value_type::string) {
        return true;
      }
      else if (is_arithmetic(into)) {
        return is_arithmetic(from);
      }
    }
  }

  return false;
}

VIA_IMPLEMENTATION bool is_castable(p_type_node_t& from, value_type to) {
  if (primitive_type_node* primitive_left =
        get_derived_instance<type_node_base, primitive_type_node>(*from)) {
    if (to == value_type::string) {
      return true;
    }
    else if (to == value_type::integer) {
      return primitive_left->type == value_type::floating_point
        || primitive_left->type == value_type::string;
    }
  }

  return false;
}

} // namespace via

#endif
