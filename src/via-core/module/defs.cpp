// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "defs.h"

namespace via
{

Def* Def::from(Allocator& alloc, const ir::Stmt* node)
{
  if TRY_COERCE (const ir::StmtFuncDecl, fn, node) {
    auto* fndef = alloc.emplace<FunctionDef>();
    fndef->kind = FunctionDef::Kind::IR;
    fndef->ir = fn;
    fndef->symbol = fn->sym;
    return fndef;
  }

  return nullptr;
}

Def* Def::newFunction(Allocator& alloc,
                      const NativeCallback fn,
                      InitList<DefParm> parms,
                      const sema::Type* ret)
{
  auto* fndef = alloc.emplace<FunctionDef>();
  fndef->kind = FunctionDef::Kind::NATIVE;
  fndef->ntv = fn;
  fndef->parms = parms;
  fndef->ret = ret;

  return fndef;
}

String FunctionDef::dump() const
{
  return fmt::format(
      "FunctionDef(symbol={}, ret={}, parms={}, kind={}, code={})", symbol,
      ret->dump(),
      debug::dump(parms,
                  [](const auto& parm) {
                    return fmt::format("{}: {}", parm.symbol,
                                       parm.type->dump());
                  }),
      magic_enum::enum_name(kind), reinterpret_cast<const void*>(ntv));
}

}  // namespace via
