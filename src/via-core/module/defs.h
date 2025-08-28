// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_MODULE_DEFS_H_
#define VIA_CORE_MODULE_DEFS_H_

#include <via/config.h>
#include <via/types.h>
#include "ir/ir.h"
#include "sema/type.h"

namespace via
{

class Module;

struct Def
{
  virtual ~Def() = default;

  static Def* from(Module* m, const ir::Entity* e);
};

struct Module;
struct SymbolInfo
{
  const Def* def;
  const Module* mod;
};

struct FunctionDef : public Def
{
  struct Parm
  {
    SymbolId symbol;
    sema::Type* type;
  };

  SymbolId symbol;
  sema::Type* ret;
  Vec<Parm> parms;
  const ir::Function* decl;
};

struct ModuleDef : public Def
{
  SymbolId symbol;
  Map<String, Def*> defs;
  const ir::Module* decl;

  Def* lookup(SymbolId symbol);
};

}  // namespace via

#endif
