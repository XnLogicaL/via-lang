/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "types.hpp"
#include <functional>
#include <sstream>
#include <vector>
#include "ast/ast.hpp"
#include "support/math.hpp"

using enum via::BuiltinKind;
using enum via::CastResult;

template <typename T, typename K, typename... Args>
    requires std::is_constructible_v<T, Args...> && std::is_constructible_v<K, Args...>
static const T* instantiate_base(
    via::BumpAllocator<>& alloc,
    std::unordered_map<K, const T*>& map,
    Args&&... args
)
{
    K key(args...);
    if (auto it = map.find(key); it != map.end())
        return it->second;
    T* type = alloc.emplace<T>(args...);
    return map[key] = type, type;
}

via::CastResult via::QualType::cast_result(QualType to) const
{
    if (is_const() && !to.is_const())
        return INVALID; // No casting away const qualifier
    if (is_strong() != to.is_strong())
        return INVALID; // No casting away strong qualifier
    if (is_reference() != to.is_reference())
        return INVALID; // No casting away reference qualifier
    return m_type->cast_result(to.m_type);
}

std::string via::QualType::to_string() const
{
    if (m_type == nullptr)
        return "<type error>";

    std::ostringstream oss;
    if (is_const())
        oss << "const ";
    if (is_strong())
        oss << "strong ";
    if (is_reference())
        oss << "&";
    oss << m_type->to_string();
    return oss.str();
}

const via::BuiltinType* via::BuiltinType::instance(TypeContext& ctx, BuiltinKind kind)
{
    return instantiate_base<BuiltinType, BuiltinKind>(ctx.m_alloc, ctx.m_builtins, kind);
}

via::CastResult via::BuiltinType::cast_result(const Type* to) const
{
    if TRY_COERCE (const BuiltinType, builtin_type, to) {
        switch (m_kind) {
        case INT:
            return builtin_type->is_one_of<FLOAT, STRING>() ? OK : INVALID;
        case FLOAT:
            return builtin_type->is_one_of<INT, STRING>() ? OK : INVALID;
        case BOOL:
        case STRING:
            return OK;
        default:
            break;
        }
    }
    return INVALID;
}

std::string via::BuiltinType::to_string() const
{
    std::string_view raw_name = via::to_string(m_kind);
    std::string name;
    name.resize(raw_name.length());
    std::transform(raw_name.begin(), raw_name.end(), name.begin(), ::tolower);
    return name;
}

const via::OptionalType* via::OptionalType::instance(TypeContext& ctx, QualType type)
{
    return instantiate_base<OptionalType, QualType>(ctx.m_alloc, ctx.m_optionals, type);
}

via::CastResult via::OptionalType::cast_result(const Type* to) const
{
    if (m_type.unwrap() == to)
        return THROW;
    if TRY_COERCE (const BuiltinType, builtin_type, to)
        return builtin_type->is_one_of<NIL>() ? THROW : INVALID;
    return INVALID;
}

std::string via::OptionalType::to_string() const
{
    return std::format("{}?", m_type.to_string());
}

const via::ArrayType* via::ArrayType::instance(TypeContext& ctx, QualType type)
{
    return instantiate_base<ArrayType, QualType>(ctx.m_alloc, ctx.m_arrays, type);
}

via::CastResult via::ArrayType::cast_result(const Type* to) const
{
    if TRY_COERCE (const BuiltinType, builtin_type, to)
        return builtin_type->is_one_of<STRING>() ? OK : INVALID;
    if TRY_COERCE (const MapType, map_type, to) {
        if TRY_COERCE (const BuiltinType, key_type, map_type->key().unwrap()) {
            if (!key_type->is_one_of<INT>())
                return INVALID;
            return m_type == map_type->value() ? OK : INVALID;
        }
    }
    return INVALID;
}

std::string via::ArrayType::to_string() const
{
    return std::format("[{}]", m_type.to_string());
}

const via::MapType* via::MapType::instance(TypeContext& ctx, QualType key, QualType value)
{
    return instantiate_base<MapType, MapKey>(ctx.m_alloc, ctx.m_maps, key, value);
}

via::CastResult via::MapType::cast_result(const Type* to) const
{
    if TRY_COERCE (const BuiltinType, builtin_type, to)
        return builtin_type->is_one_of<STRING>() ? OK : INVALID;
    return INVALID;
}

std::string via::MapType::to_string() const
{
    return std::format("{{{}: {}}}", m_key.to_string(), m_value.to_string());
}

const via::FunctionType*
via::FunctionType::instance(TypeContext& ctx, QualType ret, std::vector<QualType> parms)
{
    return instantiate_base<FunctionType, FunctionKey>(
        ctx.m_alloc,
        ctx.m_functions,
        ret,
        parms
    );
}

via::CastResult via::FunctionType::cast_result(const Type* to) const
{
    if TRY_COERCE (const BuiltinType, builtin_type, to)
        return builtin_type->is_one_of<STRING>() ? OK : INVALID;
    return INVALID;
}

std::string via::FunctionType::to_string() const
{
    std::ostringstream oss;
    oss << "fn (";

    for (size_t i = 0; const auto& parm: m_parms) {
        oss << parm.to_string();
        if (++i != m_parms.size())
            oss << ", ";
    }

    oss << ") -> ";
    oss << m_return.to_string();
    return oss.str();
}

using via::hash_all;
using via::hash_combine;
using via::hash_ptr;
using via::hash_range;

static size_t hash_qual_type(const via::QualType& type)
{
    return hash_all(hash_ptr(type.unwrap()), (size_t) type.qualifiers());
}

bool std::equal_to<via::QualType>::operator()(
    const via::QualType& a,
    const via::QualType& b
) const
{
    std::hash<via::QualType> hash;
    return hash(a) == hash(b);
}

bool std::equal_to<via::MapKey>::operator()(
    const via::MapKey& a,
    const via::MapKey& b
) const
{
    std::hash<via::MapKey> hash;
    return hash(a) == hash(b);
}

bool std::equal_to<via::FunctionKey>::operator()(
    const via::FunctionKey& a,
    const via::FunctionKey& b
) const
{
    std::hash<via::FunctionKey> hash;
    return hash(a) == hash(b);
}

size_t std::hash<via::QualType>::operator()(const via::QualType& type) const
{
    return hash_qual_type(type);
}

size_t std::hash<via::MapKey>::operator()(const via::MapKey& key) const
{
    return hash_all(hash_qual_type(key.key), hash_qual_type(key.val));
}

size_t std::hash<via::FunctionKey>::operator()(const via::FunctionKey& key) const
{
    return hash_all(
        hash_qual_type(key.result),
        hash_range(key.parms.begin(), key.parms.end(), hash_qual_type)
    );
}
