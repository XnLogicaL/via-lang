// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef vl_has_header_types_h
#define vl_has_header_types_h

#include "ast.h"
#include "common.h"
#include "object.h"

namespace via {

template<typename T>
struct data_type;

template<>
struct data_type<std::monostate> {
  static constexpr value_type value_type = value_type::nil;
  static constexpr int precedence = -1;
};

template<>
struct data_type<TInteger> {
  static constexpr value_type value_type = value_type::integer;
  static constexpr int precedence = 1;
};

template<>
struct data_type<TFloat> {
  static constexpr value_type value_type = value_type::floating_point;
  static constexpr int precedence = 2;
};

template<>
struct data_type<bool> {
  static constexpr value_type value_type = value_type::boolean;
  static constexpr int precedence = -1;
};

template<>
struct data_type<std::string> {
  static constexpr value_type value_type = value_type::string;
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

vl_inline bool is_constant_expression(expr_node_base& expression) {
  return is_derived_instance<expr_node_base, lit_expr_node>(expression);
}

vl_inline bool is_integral(p_type_node_t& type) {
  using enum value_type;

  if (primitive_type_node* primitive =
        get_derived_instance<type_node_base, primitive_type_node>(*type)) {
    return primitive->type == integer;
  }

  // TODO: Add aggregate type support by checking for arithmetic meta-methods
  return false;
}

vl_inline bool is_floating_point(p_type_node_t& type) {
  using enum value_type;

  if (primitive_type_node* primitive =
        get_derived_instance<type_node_base, primitive_type_node>(*type)) {
    return primitive->type == floating_point;
  }

  // TODO: Add aggregate type support by checking for arithmetic meta-methods
  return false;
}

vl_inline bool is_arithmetic(p_type_node_t& type) {
  return is_integral(type) || is_floating_point(type);
}

vl_inline bool is_callable(p_type_node_t& type) {
  if (is_derived_instance<type_node_base, FunctionTypeNode>(*type)) {
    return true;
  }

  return false;
}

vl_inline bool is_compatible(p_type_node_t& left, p_type_node_t& right) {
  if (primitive_type_node* primitive_left =
        get_derived_instance<type_node_base, primitive_type_node>(*left)) {
    if (primitive_type_node* primitive_right =
          get_derived_instance<type_node_base, primitive_type_node>(*right)) {

      return primitive_left->type == primitive_right->type;
    }
  }

  return false;
}

vl_inline bool is_castable(p_type_node_t& from, p_type_node_t& into) {
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

vl_inline bool is_castable(p_type_node_t& from, value_type to) {
  if (primitive_type_node* primitive_left =
        get_derived_instance<type_node_base, primitive_type_node>(*from)) {
    if (to == value_type::string) {
      return true;
    }
    else if (to == value_type::integer) {
      return primitive_left->type == value_type::floating_point ||
             primitive_left->type == value_type::string;
    }
  }

  return false;
}

} // namespace via

#endif
