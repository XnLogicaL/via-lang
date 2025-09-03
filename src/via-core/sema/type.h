// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_SEMA_TYPE_H_
#define VIA_CORE_SEMA_TYPE_H_

#include <via/config.h>
#include <via/types.h>
#include <magic_enum/magic_enum.hpp>
#include "ast/ast.h"
#include "memory.h"
#include "type_visitor.h"

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

  using InferResult = Result<Type*, String>;

 public:
  static InferResult from(Allocator& alloc, const ast::Type* type);
  static InferResult infer(Allocator& alloc, const ast::Expr* expr);

 public:
  bool isDependent() const { return flags & 0x1; }
  virtual void accept(TypeVisitor& vis, VisitInfo* vi) = 0;
  virtual String dump() const { debug::unimplemented(); }

 public:
  const Kind kind;
  const u8 flags;

 protected:
  explicit Type(Kind kind, u8 flags = 0) : kind(kind), flags(flags) {}
};

struct BuiltinType : Type
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

  void accept(TypeVisitor& vis, VisitInfo* vi) override
  {
    vis.visit(*this, vi);
  }

  String dump() const override
  {
    return fmt::format("BuiltinType({})", magic_enum::enum_name(bt));
  }
};

struct ArrayType : Type
{
  const Type* elem;
  explicit ArrayType(const Type* elem)
      : Type(Kind::Array, elem->isDependent()), elem(elem)
  {}

  void accept(TypeVisitor& vis, VisitInfo* vi) override
  {
    vis.visit(*this, vi);
  }
};

struct DictType : Type
{
  const Type *key, *val;
  explicit DictType(const Type* key, const Type* val)
      : Type(Kind::Dict, (key->isDependent() || val->isDependent())),
        key(key),
        val(val)
  {}

  void accept(TypeVisitor& vis, VisitInfo* vi) override
  {
    vis.visit(*this, vi);
  }
};

struct FuncType : Type
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

  void accept(TypeVisitor& vis, VisitInfo* vi) override
  {
    vis.visit(*this, vi);
  }
};

struct UserType : Type
{
  const ast::StmtTypeDecl* decl;
  explicit UserType(const ast::StmtTypeDecl* D) : Type(Kind::User, 0), decl(D)
  {}

  void accept(TypeVisitor& vis, VisitInfo* vi) override
  {
    vis.visit(*this, vi);
  }
};

struct TemplateParamType : Type
{
  u32 depth, index;
  explicit TemplateParamType(u32 d, u32 i)
      : Type(Kind::TemplateParam, /*dependent*/ 1), depth(d), index(i)
  {}

  void accept(TypeVisitor& vis, VisitInfo* vi) override
  {
    vis.visit(*this, vi);
  }
};

struct TemplateSpecType : Type
{
  const ast::StmtTypeDecl* primary;
  Vec<const Type*> args;
  explicit TemplateSpecType(const ast::StmtTypeDecl* prim,
                            Vec<const Type*> A,
                            bool dep)
      : Type(Kind::TemplateSpec, dep ? 1 : 0), primary(prim), args(std::move(A))
  {}

  void accept(TypeVisitor& vis, VisitInfo* vi) override
  {
    vis.visit(*this, vi);
  }
};

struct SubstParamType : Type
{
  const TemplateParamType* parm;
  const Type* replacement;
  explicit SubstParamType(const TemplateParamType* par, const Type* rs)
      : Type(Kind::SubstParam, rs->isDependent()), parm(par), replacement(rs)
  {}

  void accept(TypeVisitor& vis, VisitInfo* vi) override
  {
    vis.visit(*this, vi);
  }
};

}  // namespace sema

}  // namespace via

#endif
