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
  Local(std::string_view symbol,
        const ast::Expr* lval,
        const ast::Expr* rval,
        const sema::Type* type,
        usize version = 0,
        u64 quals = 0ULL)
      : mVersion(version),
        mQuals(quals),
        mSymbol(symbol),
        mLVal(lval),
        mRVal(rval),
        mType(type)
  {}

 public:
  usize getVersion() const { return mVersion; }
  u64 getQualifiers() const { return mQuals; }
  std::string_view getSymbol() const { return mSymbol; }
  const ast::Expr* getAstLVal() const { return mLVal; }
  const ast::Expr* getAstRVal() const { return mRVal; }
  const sema::Type* getType() const { return mType; }

 protected:
  const usize mVersion = 0;
  const u64 mQuals = 0ULL;
  const std::string_view mSymbol = "<invalid-local>";
  const ast::Expr* mLVal = nullptr;
  const ast::Expr* mRVal = nullptr;
  const sema::Type* mType = nullptr;
};

struct LocalRef
{
  u16 id;
  Local& local;
};

}  // namespace sema

}  // namespace via
