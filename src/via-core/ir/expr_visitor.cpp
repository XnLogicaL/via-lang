// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "expr_visitor.h"
#include "module/module.h"

namespace via
{

namespace ir
{

static ExprVisitInfo* visit_info(VisitInfo* raw_vi)
{
  if TRY_COERCE (ExprVisitInfo, vi, raw_vi) {
    return vi;
  } else {
    return nullptr;
  }
}

void ExprVisitor::visit(const ast::ExprLit& elit, VisitInfo* raw_vi)
{
  ExprVisitInfo* vi = visit_info(raw_vi);
  Allocator& alloc = vi->module->get_allocator();

  if (auto cv = sema::ConstValue::from_literal_token(*elit.tok)) {
    auto* k = alloc.emplace<ExprConstant>();
    k->cv = std::move(*cv);
  }
}

}  // namespace ir

}  // namespace via
