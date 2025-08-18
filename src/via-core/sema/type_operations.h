// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_SEMA_TYPE_OPERATIONS_H_
#define VIA_CORE_SEMA_TYPE_OPERATIONS_H_

#include <via/config.h>
#include <via/types.h>
#include "type_base.h"
#include "type_primitives.h"

namespace via {

namespace sema {

namespace types {

// Binary operator categories
enum class BinOp {
  Add,   // +
  Sub,   // -
  Mul,   // *
  Div,   // /
  Pow,   // ^
  Mod,   // %
  Con,   // ..
  And,   // &&
  Or,    // ||
  Bor,   // |
  Bxor,  // ^
  Band,  // &
  Bshl,  // <<
  Bshr,  // >>
};

enum class UnOp {
  Neg,   // -x
  Not,   // !x
  Bnot,  // ~x
};

template <BinOp Op, type L, type R>
struct binary_result {
  using type = void;
};

template <UnOp Op, type T>
struct unary_result {
  using type = void;
};

template <BinOp Op>
struct binary_result<Op, int_type, int_type> {
  using type = int_type;
};

template <BinOp Op>
struct binary_result<Op, float_type, float_type> {
  using type = float_type;
};

template <BinOp Op>
struct binary_result<Op, int_type, float_type> {
  using type = float_type;
};

template <BinOp Op>
struct binary_result<Op, float_type, int_type> {
  using type = float_type;
};

template <type L, type R>
struct binary_result<BinOp::And, L, R> {
  using type = bool_type;
};

template <type L, type R>
struct binary_result<BinOp::Or, L, R> {
  using type = bool_type;
};

template <type L, type R>
struct binary_result<BinOp::Con, L, R> {
  using type = string_type;
};

template <>
struct unary_result<UnOp::Neg, int_type> {
  using type = int_type;
};

template <>
struct unary_result<UnOp::Neg, float_type> {
  using type = float_type;
};

template <type T>
struct unary_result<UnOp::Not, T> {
  using type = bool_type;
};

template <>
struct unary_result<UnOp::Bnot, int_type> {
  using type = int_type;
};

template <BinOp Op, type L, type R>
using binary_result_t = typename binary_result<Op, L, R>::type;

template <UnOp Op, type T>
using unary_result_t = typename unary_result<Op, T>::type;

}  // namespace types

}  // namespace sema

}  // namespace via

#endif  // VIA_CORE_SEMA_TYPE_OPERATIONS_H_
