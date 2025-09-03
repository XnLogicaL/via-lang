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

struct DictKey;
struct FuncKey;
struct UserKey;

}  // namespace sema

}  // namespace via

template <>
struct std::hash<via::sema::DictKey>
{
  size_t operator()(const via::sema::DictKey& key) const noexcept;
};

template <>
struct std::equal_to<via::sema::DictKey>
{
  bool operator()(const via::sema::DictKey& a,
                  const via::sema::DictKey& b) const noexcept;
};

template <>
struct std::hash<via::sema::FuncKey>
{
  size_t operator()(const via::sema::FuncKey& key) const noexcept;
};

template <>
struct std::equal_to<via::sema::FuncKey>
{
  bool operator()(const via::sema::FuncKey& a,
                  const via::sema::FuncKey& b) const noexcept;
};

template <>
struct std::hash<via::sema::UserKey>
{
  size_t operator()(const via::sema::UserKey& key) const noexcept;
};

template <>
struct std::equal_to<via::sema::UserKey>
{
  bool operator()(const via::sema::UserKey& a,
                  const via::sema::UserKey& b) const noexcept;
};

namespace via
{

namespace sema
{

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
  Vec<const Type*> tps;
};

struct UserKey
{
  const ast::StmtTypeDecl* decl;
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
  TypeContext() : mAlloc(8 * 1024 * 1024) {}

  const BuiltinType* getBuiltinTypeInstance(BuiltinType::Kind kind);
  const ArrayType* getArrayTypeInstance(const Type* type);
  const DictType* getDictTypeInstance(const Type* key, const Type* val);
  const FuncType* getFunctionTypeInstance(const Type* res,
                                          Vec<const Type*> tps);
  const UserType* getUserTypeInstance(const ast::StmtTypeDecl* decl);

  const TemplateParamType* getTemplateParmInstance(u32 depth, u32 index)
  {
    debug::unimplemented();
  }

  const TemplateSpecType* getTemplateSpecInstance(const ast::StmtTypeDecl* prim,
                                                  Vec<const Type*> args)
  {
    debug::unimplemented();
  }

  const Type* instantiate(const Type* tp, const TypeEnv& env);

 private:
  BumpAllocator<> mAlloc;
  Map<BuiltinType::Kind, const BuiltinType*> mBuiltins;
  Map<const Type*, const ArrayType*> mArrays;
  Map<DictKey, const DictType*> mDicts;
  Map<FuncKey, const FuncType*> mFuncs;
  Map<UserKey, const UserType*> mUsers;
};

}  // namespace sema

}  // namespace via

#endif  // VIA_CORE_SEMA_TYPE_CONTEXT_H_
