// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "defs.h"

via::Def* via::Def::from(Allocator& alloc, const ir::Stmt* node)
{
  if TRY_COERCE (const ir::StmtFuncDecl, fn, node) {
    auto* fndef = alloc.emplace<FunctionDef>();
    fndef->kind = ImplKind::SOURCE;
    fndef->code.source = fn;
    fndef->symbol = fn->sym;
    return fndef;
  }

  return nullptr;
}

via::Def* via::Def::newFunction(Allocator& alloc,
                                const NativeCallback fn,
                                const sema::Type* ret,
                                std::initializer_list<DefParm>&& parms)
{
  auto* fndef = alloc.emplace<FunctionDef>();
  fndef->kind = ImplKind::NATIVE;
  fndef->code.native = fn;
  fndef->parms = parms;
  fndef->ret = ret;

  return fndef;
}

std::string via::FunctionDef::dump() const
{
  return fmt::format(
    "FunctionDef(symbol={}, ret={}, parms={}, kind={}, code={})", symbol,
    ret->dump(),
    debug::dump(parms,
                [](const auto& parm) {
                  return fmt::format("{}: {}", parm.symbol, parm.type->dump());
                }),
    magic_enum::enum_name(kind), reinterpret_cast<const void*>(code.native));
}
