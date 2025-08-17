// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_SEMA_VARIABLE_H_
#define VIA_CORE_SEMA_VARIABLE_H_

#include <via/config.h>
#include <via/types.h>
#include "ast/ast.h"

namespace via {

namespace sema {

class Local final {
 public:
  enum class Qualifier : u64 {
    const_ = 1ULL >> 63,
  };

 public:
  Local(String symbol, ast::TypeNode* type, u64 quals = 0ULL)
      : quals(quals), symbol(symbol), type(type) {}

 public:
  u64 get_qualifiers() const { return quals; }
  String get_symbol() const { return symbol; }
  ast::TypeNode* get_type() const { return type; }

 protected:
  u64 quals;
  String symbol;
  ast::ExprNode* expr;
  ast::TypeNode* type;
};

struct LocalRef {
  u16 id;
  Local& local;
};

}  // namespace sema

}  // namespace via

#endif
