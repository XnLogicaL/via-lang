// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "types.h"

namespace via {

// clang-format off
bool is_constant_expression(TransUnitContext& unit_ctx, const ExprNode* expression, size_t variable_depth) {
  // clang-format on
  if (dynamic_cast<const NodeLitExpr*>(expression) != nullptr) {
    return true;
  }
  else if (const NodeBinExpr* bin_expr = dynamic_cast<const NodeBinExpr*>(expression)) {
    return is_constant_expression(unit_ctx, bin_expr->lhs_expression, variable_depth + 1)
      && is_constant_expression(unit_ctx, bin_expr->rhs_expression, variable_depth + 1);
  }
  else if (const NodeSymExpr* sym_expr = dynamic_cast<const NodeSymExpr*>(expression)) {
    auto& current_closure = unit_ctx.function_stack.back();
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
  else if (const NodeArrExpr* arr_expr = dynamic_cast<const NodeArrExpr*>(expression)) {
    for (ExprNode* arr_val : arr_expr->values) {
      if (!is_constant_expression(unit_ctx, arr_val, variable_depth + 1)) {
        return false;
      }
    }

    return true;
  }

  return false;
}

bool is_nil(const TypeNode* type) {
  using enum Value::Tag;

  if (const NodePrimType* primitive = dynamic_cast<const NodePrimType*>(type)) {
    return primitive->type == Nil;
  }

  return false;
}

bool is_integral(const TypeNode* type) {
  using enum Value::Tag;

  if (const NodePrimType* primitive = dynamic_cast<const NodePrimType*>(type)) {
    return primitive->type == Int;
  }

  // TODO: Add aggregate type support by checking for arithmetic meta-methods
  return false;
}

bool is_floating_point(const TypeNode* type) {
  using enum Value::Tag;

  if (const NodePrimType* primitive = dynamic_cast<const NodePrimType*>(type)) {
    return primitive->type == Float;
  }

  // TODO: Add aggregate type support by checking for arithmetic meta-methods
  return false;
}

bool is_arithmetic(const TypeNode* type) {
  return is_integral(type) || is_floating_point(type);
}

bool is_callable(const TypeNode* type) {
  if (dynamic_cast<const NodeFuncType*>(type) != nullptr) {
    return true;
  }

  return false;
}

bool is_same(const TypeNode* left, const TypeNode* right) {
  if (const NodePrimType* primitive_left = dynamic_cast<const NodePrimType*>(left)) {
    if (const NodePrimType* primitive_right = dynamic_cast<const NodePrimType*>(right)) {
      return primitive_left->type == primitive_right->type;
    }
  }
  else if (const NodeOptType* nullable_left = dynamic_cast<const NodeOptType*>(left)) {
    if (const NodeOptType* nullable_right = dynamic_cast<const NodeOptType*>(right)) {
      return is_same(nullable_left->type, nullable_right->type);
    }
  }
  else if (const NodeGenType* generic_left = dynamic_cast<const NodeGenType*>(left)) {
    if (const NodeGenType* generic_right = dynamic_cast<const NodeGenType*>(right)) {
      if (generic_left->identifier.lexeme != generic_right->identifier.lexeme) {
        return false;
      }

      if (generic_left->generics.size() != generic_right->generics.size()) {
        return false;
      }

      size_t i = 0;
      for (TypeNode* type : generic_left->generics) {
        TypeNode* right = generic_right->generics[i++];
        if (!is_same(type, right)) {
          return false;
        }
      }

      return true;
    }
  }
  else if (const NodeArrType* array_left = dynamic_cast<const NodeArrType*>(left)) {
    if (const NodeArrType* array_right = dynamic_cast<const NodeArrType*>(right)) {
      return is_same(array_left->type, array_right->type);
    }
  }

  return false;
}

bool is_compatible(const TypeNode* left, const TypeNode* right) {
  if (is_same(left, right))
    return true;
  else if (const NodePrimType* primitive_left = dynamic_cast<const NodePrimType*>(left)) {
    if (const NodePrimType* primitive_right = dynamic_cast<const NodePrimType*>(right))
      return primitive_left->type == primitive_right->type;
  }

  if (const NodeOptType* nullable_right = dynamic_cast<const NodeOptType*>(right))
    return is_same(left, nullable_right->type) || is_nil(left);

  return false;
}

bool is_castable(const TypeNode* from, const TypeNode* into) {
  if (is_same(from, into))
    return true;

  if (const NodeOptType* nullable_left = dynamic_cast<const NodeOptType*>(from)) {
    return is_same(nullable_left->type, into) || is_nil(into);
  }

  if (const NodeOptType* nullable_right = dynamic_cast<const NodeOptType*>(into))
    return nullable_right->type == from || is_nil(from);
  else if (const NodePrimType* primitive_right = dynamic_cast<const NodePrimType*>(into)) {
    if (dynamic_cast<const NodePrimType*>(from)) {
      if (primitive_right->type == Value::Tag::String)
        return true;
      else if (is_arithmetic(into))
        return is_arithmetic(from);
    }
  }

  return false;
}

bool is_castable(const TypeNode* from, Value::Tag to) {
  if (const NodePrimType* primitive_left = dynamic_cast<const NodePrimType*>(from)) {
    if (to == Value::Tag::String) {
      return true;
    }
    else if (to == Value::Tag::Int) {
      return primitive_left->type == Value::Tag::Float
        || primitive_left->type == Value::Tag::String;
    }
  }

  return false;
}

} // namespace via
