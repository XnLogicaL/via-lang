// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "constexpr.h"
#include "debug.h"

namespace sema = via::sema;
namespace ir = via::ir;

bool isConstExpr(via::Module* m, const ir::Expr* expr)
{
  via::debug::unimplemented("isConstExpr");
}

via::Expected<sema::ConstValue> evalConstExpr(via::Module* m,
                                              const ir::Expr* expr)
{
  via::debug::unimplemented("evalConstExpr");
}
