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
class ValueRef;
class CallInfo;
struct Def;

using NativeCallback = ValueRef (*)(CallInfo& ci);

struct SymbolInfo
{
  const Def* def;
  const Module* mod;
};

struct DefParm
{
  SymbolId symbol;
  const sema::Type* type;
};

struct DefTableEntry
{
  SymbolId id;
  const Def* def;
};

using DefTable = DefTableEntry[];

struct Def
{
  virtual String dump() const = 0;

  static Def* from(Allocator& alloc, const ir::Entity* e);
  static Def* newFunction(Allocator& alloc,
                          const NativeCallback fn,
                          InitList<DefParm> parms,
                          const sema::Type* ret);
};

struct FunctionDef : public Def
{
  enum class Kind
  {
    IR,
    NATIVE,
  } kind;

  union
  {
    const ir::Function* ir;
    NativeCallback ntv;
  };

  SymbolId symbol;
  const sema::Type* ret;
  Vec<DefParm> parms;

  String dump() const override;
};

}  // namespace via

#endif
