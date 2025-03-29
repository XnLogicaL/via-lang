// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_TYPES_H
#define _VIA_TYPES_H

#include "ast.h"
#include "common.h"
#include "object.h"

VIA_NAMESPACE_BEGIN

template<typename T>
struct DataType;

template<>
struct DataType<std::monostate> {
  static constexpr ValueType value_type = ValueType::nil;
  static constexpr int precedence = -1;
};

template<>
struct DataType<TInteger> {
  static constexpr ValueType value_type = ValueType::integer;
  static constexpr int precedence = 1;
};

template<>
struct DataType<TFloat> {
  static constexpr ValueType value_type = ValueType::floating_point;
  static constexpr int precedence = 2;
};

template<>
struct DataType<bool> {
  static constexpr ValueType value_type = ValueType::boolean;
  static constexpr int precedence = -1;
};

template<>
struct DataType<std::string> {
  static constexpr ValueType value_type = ValueType::string;
  static constexpr int precedence = -1;
};

template<typename Base, typename Derived>
  requires std::is_base_of_v<Base, Derived>
Derived* get_derived_instance(Base& base) {
  return dynamic_cast<Derived*>(&base);
}

template<typename Base, typename Derived>
  requires std::is_base_of_v<Base, Derived>
bool is_derived_instance(Base& derived) {
  return get_derived_instance<Base, Derived>(derived) != nullptr;
}

VIA_INLINE bool is_constant_expression(ExprNode& expression) {
  return is_derived_instance<ExprNode, LiteralExprNode>(expression);
}

VIA_INLINE bool is_integral(pTypeNode& type) {
  using enum ValueType;

  if (PrimitiveTypeNode* primitive = get_derived_instance<TypeNode, PrimitiveTypeNode>(*type)) {
    return primitive->type == integer;
  }

  // TODO: Add aggregate type support by checking for arithmetic meta-methods
  return false;
}

VIA_INLINE bool is_floating_point(pTypeNode& type) {
  using enum ValueType;

  if (PrimitiveTypeNode* primitive = get_derived_instance<TypeNode, PrimitiveTypeNode>(*type)) {
    return primitive->type == floating_point;
  }

  // TODO: Add aggregate type support by checking for arithmetic meta-methods
  return false;
}

VIA_INLINE bool is_arithmetic(pTypeNode& type) {
  return is_integral(type) || is_floating_point(type);
}

VIA_INLINE bool is_callable(pTypeNode& type) {
  if (is_derived_instance<TypeNode, FunctionTypeNode>(*type)) {
    return true;
  }

  return false;
}

VIA_INLINE bool is_compatible(pTypeNode& left, pTypeNode& right) {
  if (PrimitiveTypeNode* primitive_left =
        get_derived_instance<TypeNode, PrimitiveTypeNode>(*left)) {
    if (PrimitiveTypeNode* primitive_right =
          get_derived_instance<TypeNode, PrimitiveTypeNode>(*right)) {

      return primitive_left->type == primitive_right->type;
    }
  }

  return false;
}

VIA_INLINE bool is_castable(pTypeNode& from, pTypeNode& into) {
  if (PrimitiveTypeNode* primitive_right =
        get_derived_instance<TypeNode, PrimitiveTypeNode>(*into)) {
    if (get_derived_instance<TypeNode, PrimitiveTypeNode>(*from)) {
      if (primitive_right->type == ValueType::string) {
        return true;
      }
      else if (is_arithmetic(into)) {
        return is_arithmetic(from);
      }
    }
  }

  return false;
}

VIA_INLINE bool is_castable(pTypeNode& from, ValueType to) {
  if (PrimitiveTypeNode* primitive_left =
        get_derived_instance<TypeNode, PrimitiveTypeNode>(*from)) {
    if (to == ValueType::string) {
      return true;
    }
    else if (to == ValueType::integer) {
      return primitive_left->type == ValueType::floating_point ||
             primitive_left->type == ValueType::string;
    }
  }

  return false;
}

VIA_NAMESPACE_END

#endif
