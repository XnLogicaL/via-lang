/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "type_context.h"

namespace sema = via::sema;

static inline size_t hash_combine(size_t seed, size_t v) noexcept
{
    return seed ^ (v + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2));
}

static inline size_t hash_ptr(const void* ptr) noexcept
{
    return reinterpret_cast<size_t>(reinterpret_cast<uintptr_t>(ptr));
}

template <class It, class ElemHash>
static inline size_t hash_range(It first, It last, size_t seed, ElemHash hash)
{
    seed = hash_combine(seed, static_cast<size_t>(std::distance(first, last)));
    for (auto it = first; it != last; ++it) {
        seed = hash_combine(seed, hash(*it));
    }
    return seed;
}

size_t std::hash<sema::DictKey>::operator()(const sema::DictKey& key) const noexcept
{
    size_t seed = hash_ptr(key.key);
    seed = hash_combine(seed, hash_ptr(key.val));
    return seed;
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
    size_t seed = hash_ptr(key.result);
    seed = hash_range(key.tps.begin(), key.tps.end(), seed, hash_ptr);
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
    return hash_ptr(key.decl);
}

bool std::equal_to<sema::UserKey>::operator()(
    const sema::UserKey& a,
    const sema::UserKey& b
) const noexcept
{
    return a.decl == b.decl;
}

template <typename Tp, typename Key, typename... Args>
static const Tp* instantiate_base(
    via::BumpAllocator<>& alloc,
    std::unordered_map<Key, const Tp*>& map,
    Args&&... args
)
{
    Key key(args...);
    if (auto it = map.find(key); it != map.end()) {
        return it->second;
    }
    else {
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
    return instantiate_base<DictType>(m_alloc, m_dicts, DictKey{key, val});
}

const sema::FuncType*
sema::TypeContext::get_function(const Type* ret, std::vector<const Type*> parms)
{
    return instantiate_base<FuncType>(m_alloc, m_funcs, FuncKey{ret, parms});
}

const sema::UserType* sema::TypeContext::get_user(const ast::StmtTypeDecl* decl)
{
    return instantiate_base<UserType>(m_alloc, m_users, UserKey{decl});
}

const sema::Type* sema::TypeContext::instantiate(const Type* type, const TypeEnv& env)
{
    if (TRY_IS(const BuiltinType, type) || TRY_IS(const UserType, type))
        return type; // already canonical, no params
    else if TRY_COERCE (const TemplateParamType, parm, type) {
        if (auto* result = env.lookup(parm->depth, parm->index))
            return result; // fully substituted here
        return type;       // still dependent
    }
    else if TRY_COERCE (const SubstParamType, subst, type) {
        auto* result = instantiate(subst->replacement, env);
        if (result == subst->replacement)
            return type;
        return m_alloc.emplace<SubstParamType>(subst->parm, result);
    }
    else if TRY_COERCE (const ArrayType, array, type) {
        auto* tmp = instantiate(array->type, env);
        return (tmp == array->type) ? type : get_array(tmp);
    }
    else if TRY_COERCE (const DictType, dict, type) {
        auto* key = instantiate(dict->key, env);
        auto* val = instantiate(dict->val, env);
        return (key == dict->key && val == dict->val) ? type : get_dict(key, val);
    }
    else if TRY_COERCE (const FuncType, function, type) {
        std::vector<const Type*> parm_types;
        parm_types.reserve(function->params.size());

        bool same = true;
        for (auto* parm: function->params) {
            auto* parm_inst = instantiate(parm, env);
            same &= (parm_inst == parm);
            parm_types.push_back(parm_inst);
        }

        auto* result = instantiate(function->result, env);
        same &= (result == function->result);
        return same ? type : get_function(result, parm_types);
    }
    else if TRY_COERCE (const TemplateSpecType, spec, type) {
        std::vector<const Type*> args_types;
        args_types.reserve(spec->args.size());

        bool same = true;
        for (auto* arg: spec->args) {
            auto* arg_inst = instantiate(arg, env);
            same &= (arg_inst == arg);
            args_types.push_back(arg_inst);
        }

        return same ? type : get_template_spec(spec->primary, args_types);
    }

    return type; // defensive
}
