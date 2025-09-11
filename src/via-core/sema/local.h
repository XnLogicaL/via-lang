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
#include "ast/ast.h"
#include "ir/ir.h"
#include "module/symbol.h"
#include "type.h"

namespace via
{

namespace sema
{

class Local final
{
 public:
  enum class Qualifier : u64
  {
    Const = 1ULL >> 63,
  };

 public:
  Local() = default;
  Local(SymbolId symbol,
        const ast::StmtVarDecl* astDecl,
        const ir::StmtVarDecl* irDecl,
        usize version = 0,
        u64 quals = 0ULL)
      : mVersion(version),
        mQuals(quals),
        mSymbol(symbol),
        mAstDecl(astDecl),
        mIrDecl(irDecl)
  {}

 public:
  auto getVersion() const { return mVersion; }
  auto getQualifiers() const { return mQuals; }
  auto getSymbol() const { return mSymbol; }
  const auto* getAstDecl() const { return mAstDecl; }
  const auto* getIrDecl() const { return mIrDecl; }

 protected:
  const usize mVersion = 0;
  const u64 mQuals = 0ULL;
  const SymbolId mSymbol = -1;
  const ast::StmtVarDecl* mAstDecl = nullptr;
  const ir::StmtVarDecl* mIrDecl = nullptr;
};

struct LocalRef
{
  u16 id;
  Local& local;
};

}  // namespace sema

}  // namespace via
