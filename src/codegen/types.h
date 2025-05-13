// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

/**
 * @file types.h
 * @brief Declares all compilation-time type related core and utilities
 */
#ifndef VIA_HAS_HEADER_TYPES_H
#define VIA_HAS_HEADER_TYPES_H

#include "common.h"
#include "context.h"
#include "stack.h"

#include <parse/ast.h>
#include <interpreter/tvalue.h>

/**
 * @namespace via
 * @ingroup via_namespace
 * @{
 */
namespace via {

/**
 * @struct DataType
 * @tparam T
 */
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

/**
 * @brief Returns whether if the given expression is a constant expression
 * @param ctx Translation unit context
 * @param expr Expression node
 * @param depth Max depth for variables
 * @return Is constant
 */
bool is_constant_expression(TransUnitContext& ctx, const ExprNode* expr, size_t depth = 0);

/**
 * @brief Returns whether if the given type is or can decay into `nil`
 * @param type Target type
 * @return Is nil
 */
bool is_nil(const TypeNode* type);

/**
 * @brief Returns whether if the given type is or can decay into `int`
 * @param type Target type
 * @return Is integral
 */
bool is_integral(const TypeNode* type);

/**
 * @brief Returns whether if the given type is or can decay into `float`
 * @param type Target type
 * @return Is floating point
 */
bool is_floating_point(const TypeNode* type);

/**
 * @brief Returns whether if the given type is or can decay into a number
 * @param type Target type
 * @return Is arithmetic
 */
bool is_arithmetic(const TypeNode* type);

/**
 * @brief Returns whether if the given type is a callable
 * @param type Target type
 * @return Is callable
 */
bool is_callable(const TypeNode* type);

/**
 * @brief Returns whether if the two given types are the same
 * @param left First type
 * @param right Second type
 * @return Is same
 */
bool is_same(const TypeNode* left, const TypeNode* right);

/**
 * @brief Returns whether if the two given types are compatible (e.g. `int` and `float`)
 * @param left First type
 * @param right Second type
 * @return Is compatible
 */
bool is_compatible(const TypeNode* left, const TypeNode* right);

/**
 * @brief Returns whether if the first type is castable into the second type
 * @param left First type
 * @param right Second type
 * @return Is castable
 */
bool is_castable(const TypeNode* from, const TypeNode* to);

} // namespace via

/** @} */

#endif
