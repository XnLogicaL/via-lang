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

enum class ImplKind
{
  SOURCE,
  NATIVE,
};

union ImplStorage
{
  const ir::StmtFuncDecl* source;
  NativeCallback native;
};

struct SymbolInfo
{
  const Def* symbol;
  const Module* module;
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
  virtual std::string dump() const = 0;

  static Def* from(Allocator& alloc, const ir::Stmt* node);
  static Def* newFunction(Allocator& alloc,
                          const NativeCallback fn,
                          const sema::Type* ret,
                          std::initializer_list<DefParm>&& parms);
};

struct FunctionDef : public Def
{
  ImplKind kind;
  ImplStorage code;
  SymbolId symbol;
  Vec<DefParm> parms;
  const sema::Type* ret;

  std::string dump() const override;
};

}  // namespace via

#endif
