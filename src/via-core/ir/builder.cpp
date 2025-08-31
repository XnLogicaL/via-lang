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

  static StmtVisitInfo* from(VisitInfo* raw)
  {
    if TRY_COERCE (StmtVisitInfo, vi, raw) {
      return vi;
    } else {
      debug::bug("invalid StmtVisitInfo");
    }
  }
};

class StmtVisitor final : public ast::Visitor
{
 public:
  void visit(const ast::StmtVarDecl&, VisitInfo* raw) {}
  void visit(const ast::StmtScope&, VisitInfo* raw) {}
  void visit(const ast::StmtIf&, VisitInfo* raw) {}
  void visit(const ast::StmtFor&, VisitInfo* raw) {}
  void visit(const ast::StmtForEach&, VisitInfo* raw) {}
  void visit(const ast::StmtWhile&, VisitInfo* raw) {}
  void visit(const ast::StmtAssign&, VisitInfo* raw) {}
  void visit(const ast::StmtReturn&, VisitInfo* raw) {}
  void visit(const ast::StmtEnum&, VisitInfo* raw) {}

  void visit(const ast::StmtImport& stmtImport, VisitInfo* raw)
  {
    using enum ast::StmtImport::TailKind;

    auto* vi = StmtVisitInfo::from(raw);
    QualPath qs;

    for (const Token* tok : stmtImport.path) {
      qs.push_back(tok->toString());
    }

    switch (stmtImport.kind) {
      case Import: {
        auto result = vi->module->resolveImport(qs);
        if (!result.has_value()) {
          vi->diags->report<Error>(stmtImport.loc, result.error());
        }
      } break;
      default:
        vi->diags->report<Error>(stmtImport.loc,
                                 "Unsupported import tail (TBA)");
        break;
    }
  }

  void visit(const ast::StmtModule&, VisitInfo* raw) {}
  void visit(const ast::StmtFunctionDecl&, VisitInfo* raw) {}
  void visit(const ast::StmtStructDecl&, VisitInfo* raw) {}
  void visit(const ast::StmtTypeDecl&, VisitInfo* raw) {}
  void visit(const ast::StmtUsing&, VisitInfo* raw) {}
  void visit(const ast::StmtEmpty&, VisitInfo* raw) {}
  void visit(const ast::StmtExpr&, VisitInfo* raw) {}
};

IrTree Builder::build()
{
  IrTree tree;
  StmtVisitInfo vi;
  vi.diags = &mDiags;
  vi.module = mModule;

  StmtVisitor vis;

  for (const ast::Stmt* stmt : mAst) {
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
