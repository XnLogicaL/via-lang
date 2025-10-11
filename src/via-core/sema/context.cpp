/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "context.hpp"
#include "support/math.hpp"

namespace sema = via::sema;

size_t std::hash<sema::DictKey>::operator()(const sema::DictKey& key) const noexcept
{
    return via::hash_all(via::hash_ptr(key.key), via::hash_ptr(key.val));
}

bool std::equal_to<sema::DictKey>::operator()(
    const sema::DictKey& a,
    const sema::DictKey& b
) const noexcept
{
    return a.key == b.key && a.val == b.val;
}

size_t std::hash<sema::FuncKey>::operator()(const sema::FuncKey& key) const noexcept
{
    size_t seed = via::hash_ptr(key.result);
    seed = via::hash_range(key.tps.begin(), key.tps.end(), seed, via::hash_ptr);
    return seed;
}

bool std::equal_to<sema::FuncKey>::operator()(
    const sema::FuncKey& a,
    const sema::FuncKey& b
) const noexcept
{
    if (a.result != b.result)
        return false;
    if (a.tps.size() != b.tps.size())
        return false;
    for (size_t i = 0; i < a.tps.size(); ++i)
        if (a.tps[i] != b.tps[i])
            return false;
    return true;
}

size_t std::hash<sema::UserKey>::operator()(const sema::UserKey& key) const noexcept
{
    return via::hash_ptr(key.decl);
}

bool std::equal_to<sema::UserKey>::operator()(
    const sema::UserKey& a,
    const sema::UserKey& b
) const noexcept
{
    return a.decl == b.decl;
}

template <typename Tp, typename Key, typename... Args>
    requires std::is_constructible_v<Tp, Args...> && std::is_constructible_v<Key, Args...>
static const Tp* instantiate_base(
    via::BumpAllocator<>& alloc,
    std::unordered_map<Key, const Tp*>& map,
    Args&&... args
)
{
    Key key(args...);
    if (auto it = map.find(key); it != map.end()) {
        return it->second;
    } else {
        auto* type = alloc.emplace<Tp>(args...);
        map[key] = type;
        return type;
    }
}

const sema::BuiltinType* sema::TypeContext::get_builtin(BuiltinKind kind)
{
    return instantiate_base<BuiltinType>(m_alloc, m_builtins, kind);
}

const sema::OptionalType* sema::TypeContext::get_optional(const Type* type)
{
    return instantiate_base<OptionalType>(m_alloc, m_optionals, type);
}

const sema::ArrayType* sema::TypeContext::get_array(const Type* type)
{
    return instantiate_base<ArrayType>(m_alloc, m_arrays, type);
}

const sema::DictType* sema::TypeContext::get_dict(const Type* key, const Type* val)
{
    return instantiate_base<DictType>(m_alloc, m_dicts, key, val);
}

const sema::FuncType*
sema::TypeContext::get_function(const Type* ret, std::vector<const Type*> parms)
{
    return instantiate_base<FuncType>(m_alloc, m_funcs, ret, parms);
}

const sema::UserType* sema::TypeContext::get_user(const ast::StmtTypeDecl* decl)
{
    return instantiate_base<UserType>(m_alloc, m_users, decl);
}
