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

bool is_constexpr(Module* m, const ir::Expr* expr)
{
  debug::unimplemented("is_constexpr");
}

Result<ConstValue, via::String> to_constexpr(Module* m, const ir::Expr* expr)
{
  debug::unimplemented("to_constexpr");
}

}  // namespace sema

}  // namespace via
