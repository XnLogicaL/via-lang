//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================

#ifndef VIA_HAS_HEADER_TYPES_H
#define VIA_HAS_HEADER_TYPES_H

#include "ast.h"
#include "common.h"
#include "object.h"
#include "stack.h"

namespace via {

template<typename T>
struct DataType;

template<>
struct DataType<std::monostate> {
  static constexpr IValueType type = IValueType::nil;
  static constexpr int precedence = -1;
};

template<>
struct DataType<int> {
  static constexpr IValueType type = IValueType::integer;
  static constexpr int precedence = 1;
};

template<>
struct DataType<float> {
  static constexpr IValueType type = IValueType::floating_point;
  static constexpr int precedence = 2;
};

template<>
struct DataType<bool> {
  static constexpr IValueType type = IValueType::boolean;
  static constexpr int precedence = -1;
};

template<>
struct DataType<std::string> {
  static constexpr IValueType type = IValueType::string;
  static constexpr int precedence = -1;
};

template<typename base, typename derived>
  requires std::is_base_of_v<base, derived>
derived* get_derived_instance(base* der) {
  return dynamic_cast<derived*>(der);
}

template<typename base, typename derived>
const derived* get_derived_instance(const base* der) {
  return dynamic_cast<const derived*>(der);
}

template<typename base, typename derived>
  requires std::is_base_of_v<base, derived>
bool is_derived_instance(const base* der) {
  return get_derived_instance<base, derived>(der) != nullptr;
}

// clang-format off
VIA_IMPLEMENTATION bool is_constant_expression(
  TransUnitContext& unit_ctx,
  const ExprNodeBase* expression,
  size_t variable_depth = 0
) 
{ // clang-format on
  if (is_derived_instance<ExprNodeBase, LitExprNode>(expression)) {
    return true;
  }
  else if (const BinExprNode* bin_expr =
             get_derived_instance<ExprNodeBase, BinExprNode>(expression)) {
    return is_constant_expression(unit_ctx, bin_expr->lhs_expression, variable_depth + 1)
      && is_constant_expression(unit_ctx, bin_expr->rhs_expression, variable_depth + 1);
  }
  else if (const SymExprNode* sym_expr =
             get_derived_instance<ExprNodeBase, SymExprNode>(expression)) {
    auto& current_closure = unit_ctx.internal.function_stack->top();
    auto var_obj = current_closure.locals.get_local_by_symbol(sym_expr->identifier.lexeme);
    if (!var_obj.has_value()) {
      return false;
    }

    // Check if call exceeds variable depth limit
    if (variable_depth > 5) {
      return false;
    }

    return is_constant_expression(unit_ctx, (*var_obj)->value, ++variable_depth);
  }

  return false;
}

VIA_IMPLEMENTATION bool is_nil(const TypeNodeBase* type) {
  using enum IValueType;

  if (const PrimTypeNode* primitive = get_derived_instance<TypeNodeBase, PrimTypeNode>(type)) {
    return primitive->type == nil;
  }

  return false;
}

VIA_IMPLEMENTATION bool is_integral(const TypeNodeBase* type) {
  using enum IValueType;

  if (const PrimTypeNode* primitive = get_derived_instance<TypeNodeBase, PrimTypeNode>(type)) {
    return primitive->type == integer;
  }

  // TODO: Add aggregate type support by checking for arithmetic meta-methods
  return false;
}

VIA_IMPLEMENTATION bool is_floating_point(const TypeNodeBase* type) {
  using enum IValueType;

  if (const PrimTypeNode* primitive = get_derived_instance<TypeNodeBase, PrimTypeNode>(type)) {
    return primitive->type == floating_point;
  }

  // TODO: Add aggregate type support by checking for arithmetic meta-methods
  return false;
}

VIA_IMPLEMENTATION bool is_arithmetic(const TypeNodeBase* type) {
  return is_integral(type) || is_floating_point(type);
}

VIA_IMPLEMENTATION bool is_callable(const TypeNodeBase* type) {
  if (is_derived_instance<TypeNodeBase, FunctionTypeNode>(type)) {
    return true;
  }

  return false;
}

VIA_IMPLEMENTATION bool is_same(const TypeNodeBase* left, const TypeNodeBase* right) {
  if (const PrimTypeNode* primitive_left = get_derived_instance<TypeNodeBase, PrimTypeNode>(left)) {
    if (const PrimTypeNode* primitive_right =
          get_derived_instance<TypeNodeBase, PrimTypeNode>(right)) {
      return primitive_left->type == primitive_right->type;
    }
  }
  else if (const GenericTypeNode* generic_left =
             get_derived_instance<TypeNodeBase, GenericTypeNode>(left)) {
    if (const GenericTypeNode* generic_right =
          get_derived_instance<TypeNodeBase, GenericTypeNode>(right)) {
      if (generic_left->identifier.lexeme != generic_right->identifier.lexeme) {
        return false;
      }

      if (generic_left->generics.size() != generic_right->generics.size()) {
        return false;
      }

      size_t i = 0;
      for (TypeNodeBase* type : generic_left->generics) {
        TypeNodeBase* right = generic_right->generics[i++];
        if (!is_same(type, right)) {
          return false;
        }
      }

      return true;
    }
  }
  else if (const ArrayTypeNode* array_left =
             get_derived_instance<TypeNodeBase, ArrayTypeNode>(left)) {
    if (const ArrayTypeNode* array_right =
          get_derived_instance<TypeNodeBase, ArrayTypeNode>(right)) {
      return is_same(array_left->type, array_right->type);
    }
  }

  return false;
}

VIA_IMPLEMENTATION bool is_compatible(const TypeNodeBase* left, const TypeNodeBase* right) {
  if (const PrimTypeNode* primitive_left = get_derived_instance<TypeNodeBase, PrimTypeNode>(left)) {
    if (const PrimTypeNode* primitive_right =
          get_derived_instance<TypeNodeBase, PrimTypeNode>(right)) {

      return primitive_left->type == primitive_right->type;
    }
  }

  return is_same(left, right);
}

VIA_IMPLEMENTATION bool is_castable(const TypeNodeBase* from, const TypeNodeBase* into) {
  if (const PrimTypeNode* primitive_right =
        get_derived_instance<TypeNodeBase, PrimTypeNode>(into)) {
    if (get_derived_instance<TypeNodeBase, PrimTypeNode>(from)) {
      if (primitive_right->type == IValueType::string) {
        return true;
      }
      else if (is_arithmetic(into)) {
        return is_arithmetic(from);
      }
    }
  }

  return false;
}

VIA_IMPLEMENTATION bool is_castable(const TypeNodeBase* from, IValueType to) {
  if (const PrimTypeNode* primitive_left = get_derived_instance<TypeNodeBase, PrimTypeNode>(from)) {
    if (to == IValueType::string) {
      return true;
    }
    else if (to == IValueType::integer) {
      return primitive_left->type == IValueType::floating_point
        || primitive_left->type == IValueType::string;
    }
  }

  return false;
}

} // namespace via

#endif
