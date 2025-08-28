// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "defs.h"
#include "module.h"

namespace via
{

Def* Def::from(Module* m, const ir::Entity* e)
{
  Allocator& alloc = m->get_allocator();

  if TRY_COERCE (const ir::Function, fn, e) {
    auto* fndef = alloc.emplace<FunctionDef>();
    fndef->decl = fn;
    fndef->symbol = fn->symbol;
    return fndef;
  }

  return nullptr;
}

}  // namespace via
