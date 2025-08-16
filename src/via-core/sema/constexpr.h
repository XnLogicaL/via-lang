// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_SEMA_CONSTEXPR_H_
#define VIA_SEMA_CONSTEXPR_H_

#include <via/config.h>
#include <via/types.h>
#include "context.h"
#include "parser/ast.h"

namespace via {

namespace sema {

// Recursive check to determine whether if an expression can be evaluated during
// compile-time.
bool is_constexpr(SemaContext& ctx, const ast::ExprNode* expr);

}  // namespace sema

}  // namespace via

#endif
