// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_AST_PASS_H_
#define VIA_CORE_AST_PASS_H_

#include <via/config.h>
#include <via/types.h>
#include "ast/ast.h"
#include "sema/context.h"

namespace via {

namespace ast {

class Pass {
 public:
  Pass(Allocator& alloc, sema::Context& ctx) : m_alloc(alloc), m_ctx(ctx) {}

 public:
  virtual StmtNode* apply(StmtNode* stmt) = 0;

 protected:
  Allocator& m_alloc;
  sema::Context& m_ctx;
};

namespace types {

template <typename T>
struct is_pass {
  static constexpr bool value = std::is_base_of_v<Pass, T>;
};

template <typename T>
inline constexpr auto is_pass_v = is_pass<T>::value;

template <typename T>
concept pass = is_pass_v<T>;

}  // namespace types

}  // namespace ast

}  // namespace via

#endif
