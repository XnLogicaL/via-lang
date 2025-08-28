// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "type_context.h"

namespace via
{

namespace sema
{

const BuiltinType* TypeContext::get_builtin(BuiltinType::Kind kind)
{
  return m_builtins[(u8)kind];
}

const ArrayType* TypeContext::get_array(const Type* type)
{
  return m_arrays[type];
}

const DictType* TypeContext::get_dict(const Type* key, const Type* val)
{
  return m_dicts[DictKey{
      .key = key,
      .val = val,
  }];
}

const FuncType* TypeContext::get_function(const Type* res, Vec<const Type*> tps)
{
  return m_funcs[FuncKey{
      .result = res,
      .tps = tps,
  }];
}

const UserType* TypeContext::get_utype(const ast::StmtTypeDecl* decl)
{
  return m_users[UserKey{.decl = decl}];
}

const Type* TypeContext::instantiate(const Type* tp, const TypeEnv& env)
{
  switch (tp->kind) {
    case Type::Kind::Builtin:
    case Type::Kind::User:
      return tp;  // already canonical, no params

    case Type::Kind::TemplateParam: {
      auto* parm = static_cast<const TemplateParamType*>(tp);

      if (auto* rs = env.lookup(parm->depth, parm->index))
        return rs;  // fully substituted here

      return tp;  // still dependent
    }

    case Type::Kind::SubstParam: {
      auto* sbs = static_cast<const SubstParamType*>(tp);
      auto* rs = instantiate(sbs->replacement, env);

      if (rs == sbs->replacement)
        return tp;

      return m_alloc.emplace<SubstParamType>(sbs->parm, rs);
    }

    case Type::Kind::Array: {
      auto* at = static_cast<const ArrayType*>(tp);
      auto* tmp = instantiate(at->elem, env);
      return (tmp == at->elem) ? tp : get_array(tmp);
    }

    case Type::Kind::Dict: {
      auto* dt = static_cast<const DictType*>(tp);
      auto* key = instantiate(dt->key, env);
      auto* val = instantiate(dt->val, env);
      return (key == dt->key && val == dt->val) ? tp : get_dict(key, val);
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
      return same ? tp : get_function(rs, tps);
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

      return same ? tp : get_tspec(S->primary, args);
    }
  }

  return tp;  // defensive
}

}  // namespace sema

}  // namespace via
