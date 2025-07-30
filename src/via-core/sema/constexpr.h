// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_SEMA_CONSTEXPR_H_
#define VIA_SEMA_CONSTEXPR_H_

#include <via/config.h>
#include "parser/ast.h"

namespace via {

namespace core {

namespace sema {

bool is_constexpr(const parser::ast::ExprNode* expr);

}  // namespace sema

}  // namespace core

}  // namespace via

#endif
