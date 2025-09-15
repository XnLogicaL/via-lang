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

class IRLocal final
{
 public:
  enum Qual : u8
  {
    CONST = 1 << 0,
  };

  struct Ref
  {
    u16 id;
    IRLocal& local;
  };

 public:
  IRLocal() = default;
  IRLocal(SymbolId symbol,
          const ast::Stmt* astDecl,
          const ir::Stmt* irDecl,
          usize version = 0,
          u8 quals = 0ULL)
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
  const u8 mQuals = 0ULL;
  const SymbolId mSymbol = -1;
  const ast::Stmt* mAstDecl = nullptr;
  const ir::Stmt* mIrDecl = nullptr;
};

}  // namespace sema

}  // namespace via
