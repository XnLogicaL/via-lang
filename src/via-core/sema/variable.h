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
  Local() = default;
  Local(StringView symbol,
        const ast::LValue* lval,
        const ast::ExprNode* rval,
        const ast::TypeNode* type,
        usize version = 0,
        u64 quals = 0ULL)
      : m_version(version),
        m_quals(quals),
        m_symbol(symbol),
        m_lval(lval),
        m_rval(rval),
        m_type(type) {}

 public:
  usize get_version() const { return m_version; }
  u64 get_qualifiers() const { return m_quals; }
  StringView get_symbol() const { return m_symbol; }
  const ast::LValue* get_lval() const { return m_lval; }
  const ast::ExprNode* get_rval() const { return m_rval; }
  const ast::TypeNode* get_type() const { return m_type; }

 protected:
  const usize m_version = 0;
  const u64 m_quals = 0ULL;
  const StringView m_symbol = "<invalid-local>";
  const ast::LValue* m_lval = NULL;
  const ast::ExprNode* m_rval = NULL;
  const ast::TypeNode* m_type = NULL;
};

struct LocalRef {
  u16 id;
  Local& local;
};

}  // namespace sema

}  // namespace via

#endif
