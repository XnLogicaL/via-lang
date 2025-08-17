// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_SEMA_CONSTEXPR_H_
#define VIA_SEMA_CONSTEXPR_H_

#include <via/config.h>
#include <via/types.h>
#include "ast/ast.h"
#include "const_value.h"
#include "context.h"

namespace via {

namespace sema {

bool is_constexpr(Context& ctx, const ast::ExprNode* expr);
ConstValue to_constexpr(Context& ctx, const ast::ExprNode* expr);

}  // namespace sema

}  // namespace via

#endif
