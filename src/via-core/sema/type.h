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
#include "support/bit_enum.h"
#include "support/expected.h"
#include "support/memory.h"

namespace via {
namespace sema {

enum class TypeFlags : u8
{
    NONE = 0,
    DEPENDENT = 1 << 0,
};

class Type
{
  public:
    bool is_dependent() const noexcept { return flags & 0x1; }
    bool is_arithmetic() const noexcept { return is_integral() || is_float(); }

    virtual bool is_integral() const noexcept { return false; }
    virtual bool is_float() const noexcept { return false; }
    virtual std::string get_dump() const { debug::unimplemented(); }
    virtual std::string to_string() const { debug::unimplemented(); }

  public:
    const TypeFlags flags;

  protected:
    explicit Type(TypeFlags flags = TypeFlags::NONE)
        : flags(flags)
    {}
};

enum class BuiltinKind : u8
{
    NIL,
    BOOL,
    INT,
    FLOAT,
    STRING
};

struct BuiltinType: public Type
{
    const BuiltinKind kind;
    explicit BuiltinType(BuiltinKind kind)
        : Type(TypeFlags::NONE),
          kind(kind)
    {}

    bool is_integral() const noexcept override { return kind == BuiltinKind::INT; }
    bool is_float() const noexcept override { return kind == BuiltinKind::FLOAT; }

    std::string get_dump() const noexcept override
    {
        return std::format("BuiltinType( {} )", magic_enum::enum_name(kind));
    }

    std::string to_string() const noexcept override
    {
        auto raw_name = magic_enum::enum_name(kind);
        std::string name;
        name.resize(raw_name.length());
        std::transform(raw_name.begin(), raw_name.end(), name.begin(), ::tolower);
        return ansi::format(
            name,
            ansi::Foreground::MAGENTA,
            ansi::Background::BLACK,
            ansi::Style::BOLD
        );
    }
};

struct OptionalType: public Type
{
    const Type* type;
    explicit OptionalType(const Type* type)
        : Type((TypeFlags) type->is_dependent()),
          type(type)
    {}
};

struct ArrayType: public Type
{
    const Type* type;
    explicit ArrayType(const Type* type)
        : Type((TypeFlags) type->is_dependent()),
          type(type)
    {}
};

struct DictType: public Type
{
    const Type *key, *val;
    explicit DictType(const Type* key, const Type* val)
        : Type(TypeFlags(key->is_dependent() || val->is_dependent())),
          key(key),
          val(val)
    {}
};

struct FuncType: public Type
{
    std::vector<const Type*> params;
    const Type* result;

    explicit FuncType(std::vector<const Type*> parms, const Type* ret)
        : Type(({
              bool dep = ret->is_dependent();
              for (auto* p: parms)
                  dep |= p->is_dependent();
              dep ? TypeFlags::DEPENDENT : TypeFlags::NONE;
          })),
          params(std::move(parms)),
          result(ret)
    {}
};

struct UserType: public Type
{
    const ast::StmtTypeDecl* decl;
    explicit UserType(const ast::StmtTypeDecl* decl)
        : Type(TypeFlags::NONE),
          decl(decl)
    {}
};

struct TemplateParamType: public Type
{
    u32 depth, index;
    explicit TemplateParamType(u32 d, u32 i)
        : Type(TypeFlags::DEPENDENT),
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
        : Type(dep ? TypeFlags::DEPENDENT : TypeFlags::NONE),
          primary(prim),
          args(std::move(A))
    {}
};

struct SubstParamType: public Type
{
    const TemplateParamType* parm;
    const Type* replacement;

    explicit SubstParamType(const TemplateParamType* parm, const Type* repl)
        : Type(TypeFlags(repl->is_dependent())),
          parm(parm),
          replacement(repl)
    {}
};

} // namespace sema
} // namespace via
