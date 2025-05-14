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
#include "sema_var.h"
#include "ast.h"
#include "tvalue.h"

/**
 * @namespace via
 * @ingroup via_namespace
 * @{
 */
namespace via {

namespace sema {

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
bool is_constexpr(Context& ctx, const AstNode* expr, size_t depth = 0);

/**
 * @brief Returns whether if the given type is or can decay into `nil`
 * @param type Target type
 * @return Is nil
 */
bool is_nil(const AstNode* type);

/**
 * @brief Returns whether if the given type is or can decay into `int`
 * @param type Target type
 * @return Is integral
 */
bool is_integral(const AstNode* type);

/**
 * @brief Returns whether if the given type is or can decay into `float`
 * @param type Target type
 * @return Is floating point
 */
bool is_floating_point(const AstNode* type);

/**
 * @brief Returns whether if the given type is or can decay into a number
 * @param type Target type
 * @return Is arithmetic
 */
bool is_arithmetic(const AstNode* type);

/**
 * @brief Returns whether if the given type is a callable
 * @param type Target type
 * @return Is callable
 */
bool is_callable(const AstNode* type);

/**
 * @brief Returns whether if the two given types are the same
 * @param left First type
 * @param right Second type
 * @return Is same
 */
bool is_same(const AstNode* left, const AstNode* right);

/**
 * @brief Returns whether if the two given types are compatible (e.g. `int` and `float`)
 * @param left First type
 * @param right Second type
 * @return Is compatible
 */
bool is_compatible(const AstNode* left, const AstNode* right);

/**
 * @brief Returns whether if the first type is castable into the second type
 * @param left First type
 * @param right Second type
 * @return Is castable
 */
bool is_castable(const AstNode* from, const AstNode* to);

std::string to_string(const AstNode* node);

} // namespace sema

} // namespace via

/** @} */

#endif
