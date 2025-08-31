// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "constexpr.h"
#include "debug.h"
#include "type.h"

namespace via
{

namespace sema
{

using namespace ast;
using enum ConstValue::Kind;

bool isConstExpr(Module* m, const ir::Expr* expr)
{
  debug::unimplemented("isConstExpr");
}

Result<ConstValue, via::String> evalConstExpr(Module* m, const ir::Expr* expr)
{
  debug::unimplemented("evalConstExpr");
}

}  // namespace sema

}  // namespace via
