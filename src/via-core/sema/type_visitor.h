// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_SEMA_TYPE_VISITOR_H_
#define VIA_CORE_SEMA_TYPE_VISITOR_H_

#include <via/config.h>
#include <via/types.h>
#include "ast/visitor.h"
#include "debug.h"

namespace via
{

namespace sema
{

struct BuiltinType;
struct ArrayType;
struct DictType;
struct FuncType;
struct UserType;
struct TemplateParamType;
struct TemplateSpecType;
struct SubstParamType;

struct TypeVisitor
{
  // clang-format off
  virtual void visit(const BuiltinType& bt, VisitInfo* vi) { debug::unimplemented(); }
  virtual void visit(const ArrayType& at, VisitInfo* vi) { debug::unimplemented(); }
  virtual void visit(const DictType& dt, VisitInfo* vi) { debug::unimplemented(); }
  virtual void visit(const FuncType& ft, VisitInfo* vi) { debug::unimplemented(); }
  virtual void visit(const UserType& ut, VisitInfo* vi) { debug::unimplemented(); }
  virtual void visit(const TemplateParamType& tpt, VisitInfo* vi) { debug::unimplemented(); }
  virtual void visit(const TemplateSpecType& tst, VisitInfo* vi) { debug::unimplemented(); }
  virtual void visit(const SubstParamType& spt, VisitInfo* vi) { debug::unimplemented(); }
  // clang-format on
};

}  // namespace sema

}  // namespace via

#endif
