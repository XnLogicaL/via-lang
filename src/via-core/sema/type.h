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
#include <magic_enum/magic_enum.hpp>
#include "ast/ast.h"
#include "debug.h"
#include "expected.h"
#include "memory.h"

namespace via
{

namespace sema
{

class Type
{
 public:
  enum class Kind
  {
    Builtin,        // nil/bool/int/float/string
    Array,          // [T]
    Dict,           // {K: T}
    Function,       // fn(...T) -> R
    User,           // UserType<...>
    TemplateParam,  // typename T
    TemplateSpec,   // UserType<T0, T1, ...>
    SubstParam,     // T -> Arg
  };

 public:
  bool isDependent() const noexcept { return flags & 0x1; }
  bool isArithmetic() const noexcept { return isIntegral() || isFloat(); }

  virtual bool isIntegral() const noexcept { return false; }
  virtual bool isFloat() const noexcept { return false; }
  virtual std::string dump() const { debug::unimplemented(); }

 public:
  const Kind kind;
  const u8 flags;

 protected:
  explicit Type(Kind kind, u8 flags = 0) : kind(kind), flags(flags) {}
};

struct BuiltinType : public Type
{
  enum class Kind : u8
  {
    Nil,
    Bool,
    Int,
    Float,
    String
  };

  const Kind bt;
  explicit BuiltinType(Kind b) : Type(Type::Kind::Builtin, 0), bt(b) {}

  bool isIntegral() const noexcept override { return bt == Kind::Int; }
  bool isFloat() const noexcept override { return bt == Kind::Float; }

  std::string dump() const override
  {
    return std::format("BuiltinType({})", magic_enum::enum_name(bt));
  }
};

struct ArrayType : public Type
{
  const Type* elem;
  explicit ArrayType(const Type* elem)
      : Type(Kind::Array, elem->isDependent()), elem(elem)
  {}
};

struct DictType : public Type
{
  const Type *key, *val;
  explicit DictType(const Type* key, const Type* val)
      : Type(Kind::Dict, (key->isDependent() || val->isDependent())),
        key(key),
        val(val)
  {}
};

struct FuncType : public Type
{
  Vec<const Type*> params;
  const Type* result;

  static u8 computeDependence(const Vec<const Type*>& ps, const Type* rs)
  {
    bool dep = rs->isDependent();
    for (auto* p : ps)
      dep |= p->isDependent();
    return dep ? 1 : 0;
  }

  explicit FuncType(Vec<const Type*> ps, const Type* rs)
      : Type(Kind::Function, computeDependence(ps, rs)),
        params(std::move(ps)),
        result(rs)
  {}
};

struct UserType : public Type
{
  const ast::StmtTypeDecl* decl;
  explicit UserType(const ast::StmtTypeDecl* D) : Type(Kind::User, 0), decl(D)
  {}
};

struct TemplateParamType : public Type
{
  u32 depth, index;
  explicit TemplateParamType(u32 d, u32 i)
      : Type(Kind::TemplateParam, /*dependent*/ 1), depth(d), index(i)
  {}
};

struct TemplateSpecType : public Type
{
  const ast::StmtTypeDecl* primary;
  Vec<const Type*> args;
  explicit TemplateSpecType(const ast::StmtTypeDecl* prim,
                            Vec<const Type*> A,
                            bool dep)
      : Type(Kind::TemplateSpec, dep ? 1 : 0), primary(prim), args(std::move(A))
  {}
};

struct SubstParamType : public Type
{
  const TemplateParamType* parm;
  const Type* replacement;
  explicit SubstParamType(const TemplateParamType* par, const Type* rs)
      : Type(Kind::SubstParam, rs->isDependent()), parm(par), replacement(rs)
  {}
};

}  // namespace sema

}  // namespace via
