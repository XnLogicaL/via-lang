// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "types.h"

namespace via {

// clang-format off
bool is_constant_expression(TransUnitContext& unit_ctx, const ExprNodeBase* expression, size_t variable_depth) {
  // clang-format on
  if (dynamic_cast<const LitExprNode*>(expression) != nullptr) {
    return true;
  }
  else if (const BinExprNode* bin_expr = dynamic_cast<const BinExprNode*>(expression)) {
    return is_constant_expression(unit_ctx, bin_expr->lhs_expression, variable_depth + 1)
      && is_constant_expression(unit_ctx, bin_expr->rhs_expression, variable_depth + 1);
  }
  else if (const SymExprNode* sym_expr = dynamic_cast<const SymExprNode*>(expression)) {
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
  else if (const ArrayExprNode* arr_expr = dynamic_cast<const ArrayExprNode*>(expression)) {
    for (ExprNodeBase* arr_val : arr_expr->values) {
      if (!is_constant_expression(unit_ctx, arr_val, variable_depth + 1)) {
        return false;
      }
    }

    return true;
  }

  return false;
}

bool is_nil(const TypeNodeBase* type) {
  using enum Value::Tag;

  if (const PrimTypeNode* primitive = dynamic_cast<const PrimTypeNode*>(type)) {
    return primitive->type == Nil;
  }

  return false;
}

bool is_integral(const TypeNodeBase* type) {
  using enum Value::Tag;

  if (const PrimTypeNode* primitive = dynamic_cast<const PrimTypeNode*>(type)) {
    return primitive->type == Int;
  }

  // TODO: Add aggregate type support by checking for arithmetic meta-methods
  return false;
}

bool is_floating_point(const TypeNodeBase* type) {
  using enum Value::Tag;

  if (const PrimTypeNode* primitive = dynamic_cast<const PrimTypeNode*>(type)) {
    return primitive->type == Float;
  }

  // TODO: Add aggregate type support by checking for arithmetic meta-methods
  return false;
}

bool is_arithmetic(const TypeNodeBase* type) {
  return is_integral(type) || is_floating_point(type);
}

bool is_callable(const TypeNodeBase* type) {
  if (dynamic_cast<const FunctionTypeNode*>(type) != nullptr) {
    return true;
  }

  return false;
}

bool is_same(const TypeNodeBase* left, const TypeNodeBase* right) {
  if (const PrimTypeNode* primitive_left = dynamic_cast<const PrimTypeNode*>(left)) {
    if (const PrimTypeNode* primitive_right = dynamic_cast<const PrimTypeNode*>(right)) {
      return primitive_left->type == primitive_right->type;
    }
  }
  else if (const NullableTypeNode* nullable_left = dynamic_cast<const NullableTypeNode*>(left)) {
    if (const NullableTypeNode* nullable_right = dynamic_cast<const NullableTypeNode*>(right)) {
      return is_same(nullable_left->type, nullable_right->type);
    }
  }
  else if (const GenericTypeNode* generic_left = dynamic_cast<const GenericTypeNode*>(left)) {
    if (const GenericTypeNode* generic_right = dynamic_cast<const GenericTypeNode*>(right)) {
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
  else if (const ArrayTypeNode* array_left = dynamic_cast<const ArrayTypeNode*>(left)) {
    if (const ArrayTypeNode* array_right = dynamic_cast<const ArrayTypeNode*>(right)) {
      return is_same(array_left->type, array_right->type);
    }
  }

  return false;
}

bool is_compatible(const TypeNodeBase* left, const TypeNodeBase* right) {
  if (is_same(left, right))
    return true;
  else if (const PrimTypeNode* primitive_left = dynamic_cast<const PrimTypeNode*>(left)) {
    if (const PrimTypeNode* primitive_right = dynamic_cast<const PrimTypeNode*>(right))
      return primitive_left->type == primitive_right->type;
  }

  if (const NullableTypeNode* nullable_right = dynamic_cast<const NullableTypeNode*>(right))
    return is_same(left, nullable_right->type) || is_nil(left);

  return false;
}

bool is_castable(const TypeNodeBase* from, const TypeNodeBase* into) {
  if (is_same(from, into))
    return true;

  if (const NullableTypeNode* nullable_left = dynamic_cast<const NullableTypeNode*>(from)) {
    return is_same(nullable_left->type, into) || is_nil(into);
  }

  if (const NullableTypeNode* nullable_right = dynamic_cast<const NullableTypeNode*>(into))
    return nullable_right->type == from || is_nil(from);
  else if (const PrimTypeNode* primitive_right = dynamic_cast<const PrimTypeNode*>(into)) {
    if (dynamic_cast<const PrimTypeNode*>(from)) {
      if (primitive_right->type == Value::Tag::String)
        return true;
      else if (is_arithmetic(into))
        return is_arithmetic(from);
    }
  }

  return false;
}

bool is_castable(const TypeNodeBase* from, Value::Tag to) {
  if (const PrimTypeNode* primitive_left = dynamic_cast<const PrimTypeNode*>(from)) {
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
