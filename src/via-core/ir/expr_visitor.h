// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_IR_EXPR_VISITOR_H_
#define VIA_CORE_IR_EXPR_VISITOR_H_

#include <via/config.h>
#include <via/types.h>
#include "ast/visitor.h"
#include "ir/ir.h"

namespace via
{

class Module;

namespace ir
{

struct ExprVisitInfo : public VisitInfo
{
  via::Module* module;
  IrTree* tree;
};

class ExprVisitor : public ast::Visitor
{
 public:
  void visit(const ast::ExprLit&, VisitInfo* vi) override;
  void visit(const ast::ExprSymbol&, VisitInfo* vi) override;
  void visit(const ast::ExprDynAccess&, VisitInfo* vi) override;
  void visit(const ast::ExprStaticAccess&, VisitInfo* vi) override;
  void visit(const ast::ExprUnary&, VisitInfo* vi) override;
  void visit(const ast::ExprBinary&, VisitInfo* vi) override;
  void visit(const ast::ExprGroup&, VisitInfo* vi) override;
  void visit(const ast::ExprCall&, VisitInfo* vi) override;
  void visit(const ast::ExprSubscript&, VisitInfo* vi) override;
  void visit(const ast::ExprCast&, VisitInfo* vi) override;
  void visit(const ast::ExprTuple&, VisitInfo* vi) override;
  void visit(const ast::ExprLambda&, VisitInfo* vi) override;
};

}  // namespace ir

}  // namespace via

#endif
