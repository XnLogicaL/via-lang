// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_SEMA_CONSTEXPR_H_
#define VIA_SEMA_CONSTEXPR_H_

#include <via/config.h>
#include <via/types.h>
#include "const_value.h"
#include "expected.h"
#include "ir/ir.h"

namespace via
{

class Module;

namespace sema
{

bool isConstExpr(Module* m, const ir::Expr* expr);
Expected<ConstValue> evalConstExpr(Module* m, const ir::Expr* expr);

}  // namespace sema

}  // namespace via

#endif
