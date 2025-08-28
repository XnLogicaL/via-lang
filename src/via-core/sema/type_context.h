// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_SEMA_TYPE_CONTEXT_H_
#define VIA_CORE_SEMA_TYPE_CONTEXT_H_

#include <via/config.h>
#include <via/types.h>
#include "type.h"

namespace via
{

namespace sema
{

struct DictKey
{
  const Type *key, *val;
};

struct FuncKey
{
  const Type* result;
  Vec<const Type*> tps;
};

struct UserKey
{
  const ast::StmtTypeDecl* decl;
};

template <typename T>
struct Hash;

template <typename T>
struct Eq;

static inline usize hash_combine(usize seed, usize v) noexcept
{
  // 0x9e37... is the 64-bit golden ratio used by boost::hash_combine
  return seed ^ (v + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2));
}

static inline usize hash_ptr(const void* ptr) noexcept
{
  return reinterpret_cast<usize>(reinterpret_cast<uptr>(ptr));
}

template <class It, class ElemHash>
static inline usize hash_range(It first, It last, usize seed, ElemHash hash)
{
  seed = hash_combine(seed, static_cast<usize>(std::distance(first, last)));
  for (auto it = first; it != last; ++it) {
    seed = hash_combine(seed, hash(*it));
  }
  return seed;
}

template <>
struct Hash<DictType>
{
  usize operator()(const DictKey& key) const noexcept
  {
    usize seed = hash_ptr(key.key);
    seed = hash_combine(seed, hash_ptr(key.val));
    return seed;
  }
};

template <>
struct Eq<DictType>
{
  bool operator()(const DictKey& a, const DictKey& b) const noexcept
  {
    return a.key == b.key && a.val == b.val;
  }
};

template <>
struct Hash<FuncType>
{
  usize operator()(const FuncKey& key) const noexcept
  {
    usize seed = 0;
    seed = hash_ptr(key.result);
    seed = hash_range(key.tps.begin(), key.tps.end(), seed, hash_ptr);
    return seed;
  }
};

template <>
struct Eq<FuncType>
{
  bool operator()(const FuncKey& a, const FuncKey& b) const noexcept
  {
    if (a.result != b.result)
      return false;
    if (a.tps.size() != b.tps.size())
      return false;
    for (usize i = 0; i < a.tps.size(); ++i)
      if (a.tps[i] != b.tps[i])
        return false;
    return true;
  }
};

template <>
struct Hash<UserType>
{
  bool operator()(const UserKey& key) const noexcept
  {
    usize seed = 0;
    seed = hash_ptr(key.decl);
    return seed;
  }
};

template <>
struct Eq<UserType>
{
  bool operator()(const UserKey& a, const UserKey& b) const noexcept
  {
    return a.decl == b.decl;
  }
};

class TypeEnv final
{
 public:
  void bind(u32 d, u32 i, const Type* type) { m_map.emplace(key(d, i), type); }

  const Type* lookup(u32 d, u32 i) const
  {
    auto it = m_map.find(key(d, i));
    return it == m_map.end() ? nullptr : it->second;
  }

 private:
  static usize key(u32 d, u32 i) { return (u64(d) << 32) | i; }

 private:
  Map<u64, const Type*> m_map;
};

class TypeContext final
{
 public:
  TypeContext() : m_alloc(8 * 1024 * 1024) {}

 public:
  const BuiltinType* get_builtin(BuiltinType::Kind kind);
  const ArrayType* get_array(const Type* type);
  const DictType* get_dict(const Type* key, const Type* val);
  const FuncType* get_function(const Type* res, Vec<const Type*> tps);
  const UserType* get_utype(const ast::StmtTypeDecl* decl);
  const TemplateParamType* get_tparam(u32 depth, u32 index);
  const TemplateSpecType* get_tspec(const ast::StmtTypeDecl* prim,
                                    Vec<const Type*> args);

  const Type* instantiate(const Type* tp, const TypeEnv& env);

 private:
  BumpAllocator<> m_alloc;
  Map<u8, const BuiltinType*> m_builtins;
  Map<const Type*, const ArrayType*> m_arrays;
  Map<DictKey, const DictType*, Hash<DictType>, Eq<DictType>> m_dicts;
  Map<FuncKey, const FuncType*, Hash<FuncType>, Eq<FuncType>> m_funcs;
  Map<UserKey, const UserType*, Hash<UserType>, Eq<UserType>> m_users;
};

}  // namespace sema

}  // namespace via

#endif
