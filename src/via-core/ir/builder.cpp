// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "builder.h"
#include "module/module.h"

namespace via
{

namespace ir
{

using enum Diagnosis::Kind;

struct StmtVisitInfo : public VisitInfo
{
  DiagContext* diags;
  via::Module* module;
  ir::Entity* result = nullptr;

  static StmtVisitInfo* from(VisitInfo* raw_vi)
  {
    if TRY_COERCE (StmtVisitInfo, vi, raw_vi) {
      return vi;
    } else {
      debug::bug("invalid StmtVisitInfo");
    }
  }
};

class StmtVisitor final : public ast::Visitor
{
 public:
  void visit(const ast::StmtVarDecl&, VisitInfo* raw_vi) {}
  void visit(const ast::StmtScope&, VisitInfo* raw_vi) {}
  void visit(const ast::StmtIf&, VisitInfo* raw_vi) {}
  void visit(const ast::StmtFor&, VisitInfo* raw_vi) {}
  void visit(const ast::StmtForEach&, VisitInfo* raw_vi) {}
  void visit(const ast::StmtWhile&, VisitInfo* raw_vi) {}
  void visit(const ast::StmtAssign&, VisitInfo* raw_vi) {}
  void visit(const ast::StmtReturn&, VisitInfo* raw_vi) {}
  void visit(const ast::StmtEnum&, VisitInfo* raw_vi) {}

  void visit(const ast::StmtImport& stmt_impt, VisitInfo* raw_vi)
  {
    using enum ast::StmtImport::TailKind;

    auto* vi = StmtVisitInfo::from(raw_vi);
    QualPath qs;

    for (const Token* tok : stmt_impt.path) {
      qs.push_back(tok->to_string());
    }

    switch (stmt_impt.kind) {
      case Import: {
        auto result = vi->module->resolve_import(qs);
        if (!result.has_value()) {
          vi->diags->report<Error>(stmt_impt.loc, result.error());
        }
      } break;
      default:
        vi->diags->report<Error>(stmt_impt.loc,
                                 "Unsupported import tail (TBA)");
        break;
    }
  }

  void visit(const ast::StmtModule&, VisitInfo* raw_vi) {}
  void visit(const ast::StmtFunctionDecl&, VisitInfo* raw_vi) {}
  void visit(const ast::StmtStructDecl&, VisitInfo* raw_vi) {}
  void visit(const ast::StmtTypeDecl&, VisitInfo* raw_vi) {}
  void visit(const ast::StmtUsing&, VisitInfo* raw_vi) {}
  void visit(const ast::StmtEmpty&, VisitInfo* raw_vi) {}
  void visit(const ast::StmtExpr&, VisitInfo* raw_vi) {}
};

IrTree Builder::build()
{
  IrTree tree;
  StmtVisitInfo vi;
  vi.diags = &m_diags;
  vi.module = m_module;

  StmtVisitor vis;

  for (const ast::Stmt* stmt : m_ast) {
    stmt->accept(vis, &vi);
    if (vi.result != nullptr) {
      tree.push_back(vi.result);
      vi.result = nullptr;
    }
  }

  return tree;
}

}  // namespace ir

}  // namespace via
