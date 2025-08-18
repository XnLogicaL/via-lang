// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_SEMA_TYPE_H_
#define VIA_CORE_SEMA_TYPE_H_

#include <via/config.h>
#include <via/types.h>
#include "ast/ast.h"
#include "context.h"
#include "type_base.h"
#include "type_operations.h"
#include "type_primitives.h"
#include "type_truthiness.h"

namespace via {

namespace sema {

namespace types {

using any = Variant<types::nil_type,
                    types::bool_type,
                    types::int_type,
                    types::float_type,
                    types::string_type>;

};

types::any get_type(const Context& ctx, const ast::ExprNode* expr);

template <types::type T>
String get_typename(const Context& ctx) {
  if constexpr (std::is_same_v<T, types::nil_type>)
    return "nil";
  else if constexpr (std::is_same_v<T, types::bool_type>)
    return "boolean";
  else if constexpr (std::is_same_v<T, types::int_type>)
    return "int";
  else if constexpr (std::is_same_v<T, types::float_type>)
    return "float";
  else if constexpr (std::is_same_v<T, types::string_type>)
    return "string";

  return "<unknown-type>";
}

}  // namespace sema

}  // namespace via

#endif
