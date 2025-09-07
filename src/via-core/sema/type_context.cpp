// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "type_context.h"

namespace sema = via::sema;

static inline size_t hashCombine(size_t seed, size_t v) noexcept
{
  return seed ^ (v + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2));
}

static inline size_t hashPtr(const void* ptr) noexcept
{
  return reinterpret_cast<size_t>(reinterpret_cast<uintptr_t>(ptr));
}

template <class It, class ElemHash>
static inline size_t hashRange(It first, It last, size_t seed, ElemHash hash)
{
  seed = hashCombine(seed, static_cast<size_t>(std::distance(first, last)));
  for (auto it = first; it != last; ++it) {
    seed = hashCombine(seed, hash(*it));
  }
  return seed;
}

size_t std::hash<sema::DictKey>::operator()(
  const sema::DictKey& key) const noexcept
{
  size_t seed = hashPtr(key.key);
  seed = hashCombine(seed, hashPtr(key.val));
  return seed;
}

bool std::equal_to<sema::DictKey>::operator()(
  const sema::DictKey& a,
  const sema::DictKey& b) const noexcept
{
  return a.key == b.key && a.val == b.val;
}

size_t std::hash<sema::FuncKey>::operator()(
  const sema::FuncKey& key) const noexcept
{
  size_t seed = hashPtr(key.result);
  seed = hashRange(key.tps.begin(), key.tps.end(), seed, hashPtr);
  return seed;
}

bool std::equal_to<sema::FuncKey>::operator()(
  const sema::FuncKey& a,
  const sema::FuncKey& b) const noexcept
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

size_t std::hash<sema::UserKey>::operator()(
  const sema::UserKey& key) const noexcept
{
  return hashPtr(key.decl);
}

bool std::equal_to<sema::UserKey>::operator()(
  const sema::UserKey& a,
  const sema::UserKey& b) const noexcept
{
  return a.decl == b.decl;
}

template <typename Tp, typename Key, typename... Args>
static const Tp* instantiateBase(via::BumpAllocator<>& alloc,
                                 via::Map<Key, const Tp*>& map,
                                 Args&&... args)
{
  Key key(args...);
  if (auto it = map.find(key); it != map.end()) {
    return it->second;
  } else {
    auto* bt = alloc.emplace<Tp>(args...);
    map[key] = bt;
    return bt;
  }
}

const sema::BuiltinType* sema::TypeContext::getBuiltinTypeInstance(
  BuiltinType::Kind kind)
{
  return instantiateBase<BuiltinType, BuiltinType::Kind>(mAlloc, mBuiltins,
                                                         kind);
}

const sema::ArrayType* sema::TypeContext::getArrayTypeInstance(const Type* type)
{
  return mArrays[type];
}

const sema::DictType* sema::TypeContext::getDictTypeInstance(const Type* key,
                                                             const Type* val)
{
  return mDicts[DictKey{
    .key = key,
    .val = val,
  }];
}

const sema::FuncType* sema::TypeContext::getFunctionTypeInstance(
  const Type* res,
  Vec<const Type*> tps)
{
  return mFuncs[FuncKey{
    .result = res,
    .tps = tps,
  }];
}

const sema::UserType* sema::TypeContext::getUserTypeInstance(
  const ast::StmtTypeDecl* decl)
{
  return mUsers[UserKey{.decl = decl}];
}

const sema::Type* sema::TypeContext::instantiate(const Type* tp,
                                                 const TypeEnv& env)
{
  switch (tp->kind) {
    case Type::Kind::Builtin:
    case Type::Kind::User:
      return tp;  // already canonical, no params

    case Type::Kind::TemplateParam: {
      auto* parm = static_cast<const TemplateParamType*>(tp);

      if (auto* rs = env.lookup(parm->depth, parm->index)) {
        return rs;  // fully substituted here
      }

      return tp;  // still dependent
    }

    case Type::Kind::SubstParam: {
      auto* sbs = static_cast<const SubstParamType*>(tp);
      auto* rs = instantiate(sbs->replacement, env);

      if (rs == sbs->replacement) {
        return tp;
      }

      return mAlloc.emplace<SubstParamType>(sbs->parm, rs);
    }

    case Type::Kind::Array: {
      auto* at = static_cast<const ArrayType*>(tp);
      auto* tmp = instantiate(at->elem, env);
      return (tmp == at->elem) ? tp : getArrayTypeInstance(tmp);
    }

    case Type::Kind::Dict: {
      auto* dt = static_cast<const DictType*>(tp);
      auto* key = instantiate(dt->key, env);
      auto* val = instantiate(dt->val, env);
      return (key == dt->key && val == dt->val) ? tp
                                                : getDictTypeInstance(key, val);
    }

    case Type::Kind::Function: {
      auto* ft = static_cast<const FuncType*>(tp);
      Vec<const Type*> tps;
      tps.reserve(ft->params.size());

      bool same = true;
      for (auto* par : ft->params) {
        auto* np = instantiate(par, env);
        same &= (np == par);
        tps.push_back(np);
      }

      auto* rs = instantiate(ft->result, env);
      same &= (rs == ft->result);
      return same ? tp : getFunctionTypeInstance(rs, tps);
    }

    case Type::Kind::TemplateSpec: {
      auto* S = static_cast<const TemplateSpecType*>(tp);
      Vec<const Type*> args;
      args.reserve(S->args.size());

      bool same = true;
      for (auto* arg : S->args) {
        auto* na = instantiate(arg, env);
        same &= (na == arg);
        args.push_back(na);
      }

      return same ? tp : getTemplateSpecInstance(S->primary, args);
    }
  }

  return tp;  // defensive
}
