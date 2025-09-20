/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <magic_enum/magic_enum.hpp>
#include <via/config.h>
#include <via/types.h>
#include "ast/ast.h"
#include "debug.h"
#include "support/ansi.h"
#include "support/expected.h"
#include "support/memory.h"

namespace via {
namespace sema {

class Type
{
  public:
    enum class Kind
    {
        Builtin,       // nil/bool/int/float/string
        Array,         // [T]
        Dict,          // {K: T}
        Function,      // fn(...T) -> R
        User,          // UserType<...>
        TemplateParam, // typename T
        TemplateSpec,  // UserType<T0, T1, ...>
        SubstParam,    // T -> Arg
    };

  public:
    bool is_dependent() const noexcept { return flags & 0x1; }
    bool is_arithmetic() const noexcept { return is_integral() || is_float(); }

    virtual bool is_integral() const noexcept { return false; }
    virtual bool is_float() const noexcept { return false; }
    virtual std::string get_dump() const { debug::unimplemented(); }
    virtual std::string to_string() const { debug::unimplemented(); }

  public:
    const Kind kind;
    const u8 flags;

  protected:
    explicit Type(Kind kind, u8 flags = 0)
        : kind(kind),
          flags(flags)
    {}
};

struct BuiltinType: public Type
{
    enum class Kind : u8
    {
        NIL,
        BOOL,
        INT,
        FLOAT,
        STRING
    };

    const Kind bt;
    explicit BuiltinType(Kind b)
        : Type(Type::Kind::Builtin, 0),
          bt(b)
    {}

    bool is_integral() const noexcept override { return bt == Kind::INT; }
    bool is_float() const noexcept override { return bt == Kind::FLOAT; }

    std::string get_dump() const noexcept override
    {
        return std::format("BuiltinType( {} )", magic_enum::enum_name(bt));
    }

    std::string to_string() const noexcept override
    {
        auto raw_name = magic_enum::enum_name(bt);
        std::string name;
        name.resize(raw_name.length());
        std::transform(raw_name.begin(), raw_name.end(), name.begin(), ::tolower);
        return ansi::format(
            name,
            ansi::Foreground::Magenta,
            ansi::Background::Black,
            ansi::Style::Bold
        );
    }
};

struct ArrayType: public Type
{
    const Type* elem;
    explicit ArrayType(const Type* elem)
        : Type(Kind::Array, elem->is_dependent()),
          elem(elem)
    {}
};

struct DictType: public Type
{
    const Type *key, *val;
    explicit DictType(const Type* key, const Type* val)
        : Type(Kind::Dict, (key->is_dependent() || val->is_dependent())),
          key(key),
          val(val)
    {}
};

struct FuncType: public Type
{
    std::vector<const Type*> params;
    const Type* result;

    static u8 compute_dependence(const std::vector<const Type*>& ps, const Type* rs)
    {
        bool dep = rs->is_dependent();
        for (auto* p: ps)
            dep |= p->is_dependent();
        return dep ? 1 : 0;
    }

    explicit FuncType(std::vector<const Type*> ps, const Type* rs)
        : Type(Kind::Function, compute_dependence(ps, rs)),
          params(std::move(ps)),
          result(rs)
    {}
};

struct UserType: public Type
{
    const ast::StmtTypeDecl* decl;
    explicit UserType(const ast::StmtTypeDecl* D)
        : Type(Kind::User, 0),
          decl(D)
    {}
};

struct TemplateParamType: public Type
{
    u32 depth, index;
    explicit TemplateParamType(u32 d, u32 i)
        : Type(Kind::TemplateParam, /*dependent*/ 1),
          depth(d),
          index(i)
    {}
};

struct TemplateSpecType: public Type
{
    const ast::StmtTypeDecl* primary;
    std::vector<const Type*> args;
    explicit TemplateSpecType(
        const ast::StmtTypeDecl* prim,
        std::vector<const Type*> A,
        bool dep
    )
        : Type(Kind::TemplateSpec, dep ? 1 : 0),
          primary(prim),
          args(std::move(A))
    {}
};

struct SubstParamType: public Type
{
    const TemplateParamType* parm;
    const Type* replacement;
    explicit SubstParamType(const TemplateParamType* par, const Type* rs)
        : Type(Kind::SubstParam, rs->is_dependent()),
          parm(par),
          replacement(rs)
    {}
};

} // namespace sema

} // namespace via
