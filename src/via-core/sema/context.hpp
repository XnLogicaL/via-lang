/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <cstddef>
#include <via/config.hpp>
#include "debug.hpp"
#include "support/memory.hpp"
#include "type.hpp"

namespace via {
namespace sema {

struct DictKey;
struct FuncKey;
struct UserKey;

} // namespace sema
} // namespace via

template <>
struct std::hash<via::sema::DictKey>
{
    size_t operator()(const via::sema::DictKey& key) const noexcept;
};

template <>
struct std::equal_to<via::sema::DictKey>
{
    bool
    operator()(const via::sema::DictKey& a, const via::sema::DictKey& b) const noexcept;
};

template <>
struct std::hash<via::sema::FuncKey>
{
    size_t operator()(const via::sema::FuncKey& key) const noexcept;
};

template <>
struct std::equal_to<via::sema::FuncKey>
{
    bool
    operator()(const via::sema::FuncKey& a, const via::sema::FuncKey& b) const noexcept;
};

template <>
struct std::hash<via::sema::UserKey>
{
    size_t operator()(const via::sema::UserKey& key) const noexcept;
};

template <>
struct std::equal_to<via::sema::UserKey>
{
    bool
    operator()(const via::sema::UserKey& a, const via::sema::UserKey& b) const noexcept;
};

namespace via {
namespace sema {

struct DictKey;
struct FuncKey;
struct UserKey;

struct DictKey
{
    const Type *key, *val;
};

struct FuncKey
{
    const Type* result;
    std::vector<const Type*> tps;
};

struct UserKey
{
    const ast::StmtTypeDecl* decl;
};

class TypeContext final
{
  public:
    TypeContext() noexcept
        : m_alloc(8 * 1024 * 1024)
    {}

    const BuiltinType* get_builtin(BuiltinKind kind);
    const OptionalType* get_optional(const Type* type);
    const ArrayType* get_array(const Type* type);
    const DictType* get_dict(const Type* key, const Type* val);
    const FuncType* get_function(const Type* res, std::vector<const Type*> tps);
    const UserType* get_user(const ast::StmtTypeDecl* decl);

    const TemplateParamType* get_template_parm(uint32_t depth, uint32_t index)
    {
        debug::unimplemented();
    }
    const TemplateSpecType*
    get_template_spec(const ast::StmtTypeDecl* prim, std::vector<const Type*> args)
    {
        debug::unimplemented();
    }

  private:
    BumpAllocator<> m_alloc;
    std::unordered_map<BuiltinKind, const BuiltinType*> m_builtins;
    std::unordered_map<const Type*, const OptionalType*> m_optionals;
    std::unordered_map<const Type*, const ArrayType*> m_arrays;
    std::unordered_map<DictKey, const DictType*> m_dicts;
    std::unordered_map<FuncKey, const FuncType*> m_funcs;
    std::unordered_map<UserKey, const UserType*> m_users;
};

} // namespace sema
} // namespace via
