// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_SEMA_TYPE_H_
#define VIA_CORE_SEMA_TYPE_H_

#include <via/config.h>
#include <via/types.h>
#include "ast/ast.h"
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
  bool is_dependent() const { return flags & 0x1; }
  virtual void accept(TypeVisitor& vis, VisitInfo* vi) = 0;

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
};

struct ArrayType : Type
{
  const Type* elem;
  explicit ArrayType(const Type* elem)
      : Type(Kind::Array, elem->is_dependent()), elem(elem)
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
      : Type(Kind::Dict, (key->is_dependent() || val->is_dependent())),
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

  static u8 compute_dep(const Vec<const Type*>& ps, const Type* rs)
  {
    bool dep = rs->is_dependent();
    for (auto* p : ps)
      dep |= p->is_dependent();
    return dep ? 1 : 0;
  }

  explicit FuncType(Vec<const Type*> ps, const Type* rs)
      : Type(Kind::Function, compute_dep(ps, rs)),
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
  explicit TemplateSpecType(const ast::StmtTypeDecl* Prim,
                            Vec<const Type*> A,
                            bool dep)
      : Type(Kind::TemplateSpec, dep ? 1 : 0), primary(Prim), args(std::move(A))
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
      : Type(Kind::SubstParam, rs->is_dependent()), parm(par), replacement(rs)
  {}

  void accept(TypeVisitor& vis, VisitInfo* vi) override
  {
    vis.visit(*this, vi);
  }
};

}  // namespace sema

}  // namespace via

#endif
