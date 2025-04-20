// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_TYPES_H
#define VIA_HAS_HEADER_TYPES_H

#include "common.h"
#include "stack.h"

#include <parse/ast.h>
#include <interpreter/tvalue.h>

namespace via {

template<typename T>
struct DataType;

template<>
struct DataType<std::monostate> {
  static constexpr Value::Tag type = Value::Tag::Nil;
  static constexpr int precedence = -1;
};

template<>
struct DataType<int> {
  static constexpr Value::Tag type = Value::Tag::Int;
  static constexpr int precedence = 1;
};

template<>
struct DataType<float> {
  static constexpr Value::Tag type = Value::Tag::Float;
  static constexpr int precedence = 2;
};

template<>
struct DataType<bool> {
  static constexpr Value::Tag type = Value::Tag::Bool;
  static constexpr int precedence = -1;
};

template<>
struct DataType<std::string> {
  static constexpr Value::Tag type = Value::Tag::String;
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
bool is_constant_expression(TransUnitContext& unit_ctx, const ExprNodeBase* expression, size_t variable_depth = 0);
// clang-format on
bool is_nil(const TypeNodeBase* type);

bool is_integral(const TypeNodeBase* type);

bool is_floating_point(const TypeNodeBase* type);

bool is_arithmetic(const TypeNodeBase* type);

bool is_callable(const TypeNodeBase* type);

bool is_same(const TypeNodeBase* left, const TypeNodeBase* right);

bool is_compatible(const TypeNodeBase* left, const TypeNodeBase* right);

bool is_castable(const TypeNodeBase* from, const TypeNodeBase* to);

} // namespace via

#endif
