/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

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
