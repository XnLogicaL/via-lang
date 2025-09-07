// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "builder.h"
#include "iota.h"
#include "module/module.h"

namespace via
{

namespace ir
{

using enum Diagnosis::Kind;

static SymbolId internSymbol(const std::string& symbol)
{
  return SymbolTable::getInstance().intern(symbol);
}

static SymbolId internSymbol(const Token& symbol)
{
  return SymbolTable::getInstance().intern(symbol.toString());
}

struct ExprVisitInfo : public VisitInfo
{
  DiagContext* diags;
  via::Module* module;
  ir::Expr* result;

  static ExprVisitInfo* from(VisitInfo* raw)
  {
    if TRY_COERCE (ExprVisitInfo, vi, raw) {
      return vi;
    } else {
      debug::bug("invalid ExprVisitInfo");
    }
  }
};

class ExprVisitor final : public ast::Visitor
{
 public:
  void visit(const ast::ExprLit& elit, VisitInfo* raw) override
  {
    auto* vi = ExprVisitInfo::from(raw);
    auto& alloc = vi->module->getAllocator();

    auto* econst = alloc.emplace<ExprConstant>();
    auto cvr = sema::ConstValue::fromToken(*elit.tok);

    debug::assertm(cvr.has_value(),
                   "could not construct ConstValue from literal expression");

    econst->cv = *cvr;
    vi->result = econst;
  }

  void visit(const ast::ExprSymbol&, VisitInfo* vi) override {}
  void visit(const ast::ExprDynAccess&, VisitInfo* vi) override {}
  void visit(const ast::ExprStaticAccess&, VisitInfo* vi) override {}
  void visit(const ast::ExprUnary&, VisitInfo* vi) override {}
  void visit(const ast::ExprBinary&, VisitInfo* vi) override {}
  void visit(const ast::ExprGroup&, VisitInfo* vi) override {}
  void visit(const ast::ExprCall&, VisitInfo* vi) override {}
  void visit(const ast::ExprSubscript&, VisitInfo* vi) override {}
  void visit(const ast::ExprCast&, VisitInfo* vi) override {}
  void visit(const ast::ExprTernary&, VisitInfo* vi) override {}
  void visit(const ast::ExprArray&, VisitInfo* vi) override {}
  void visit(const ast::ExprTuple&, VisitInfo* vi) override {}
  void visit(const ast::ExprLambda&, VisitInfo* vi) override {}
};

struct StmtVisitInfo : public VisitInfo
{
  DiagContext* diags;
  via::Module* module;
  ExprVisitor* evis;
  ir::Stmt* result = nullptr;

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
  void visit(const ast::StmtVarDecl& stmtVarDecl, VisitInfo* raw) override
  {
    auto* vi = StmtVisitInfo::from(raw);
    auto& alloc = vi->module->getAllocator();

    ExprVisitInfo evi;
    evi.diags = vi->diags;
    evi.module = vi->module;

    stmtVarDecl.rval->accept(*vi->evis, &evi);

    auto* decl = alloc.emplace<StmtVarDecl>();
    decl->expr = evi.result;

    if TRY_COERCE (const ast::ExprSymbol, esym, stmtVarDecl.lval) {
      decl->sym = internSymbol(*esym->sym);
    } else {
      debug::assertm(false, "bad lvalue in variable decl");
    }

    vi->result = decl;
  }

  void visit(const ast::StmtScope& sscp, VisitInfo* raw) override
  {
    auto* vi = StmtVisitInfo::from(raw);
    auto& alloc = vi->module->getAllocator();

    auto* block = alloc.emplace<StmtBlock>();
    block->name = internSymbol(std::to_string(iota()));

    for (const auto& stmt : sscp.stmts) {
      stmt->accept(*this, vi);
      block->stmts.push_back(vi->result);
    }

    vi->result = block;
  }

  void visit(const ast::StmtIf&, VisitInfo* raw) override {}
  void visit(const ast::StmtFor&, VisitInfo* raw) override {}
  void visit(const ast::StmtForEach&, VisitInfo* raw) override {}
  void visit(const ast::StmtWhile&, VisitInfo* raw) override {}
  void visit(const ast::StmtAssign&, VisitInfo* raw) override {}
  void visit(const ast::StmtReturn&, VisitInfo* raw) override {}
  void visit(const ast::StmtEnum&, VisitInfo* raw) override {}

  void visit(const ast::StmtImport& stmtImport, VisitInfo* raw) override
  {
    using enum ast::StmtImport::TailKind;

    auto* vi = StmtVisitInfo::from(raw);
    QualPath path;

    for (const Token* tok : stmtImport.path) {
      path.push_back(tok->toString());
    }

    switch (stmtImport.kind) {
      case Import: {
        auto result = vi->module->resolveImport(path);
        if (!result.has_value()) {
          vi->diags->report<Error>(stmtImport.loc, result.error());
        }
      } break;
      default:
        debug::unimplemented("import tail");
    }
  }

  void visit(const ast::StmtModule&, VisitInfo* raw) override {}

  void visit(const ast::StmtFunctionDecl& sfn, VisitInfo* raw) override
  {
    auto* vi = StmtVisitInfo::from(raw);
    auto& alloc = vi->module->getAllocator();
    auto* fn = alloc.emplace<StmtFuncDecl>();
    fn->kind = StmtFuncDecl::Kind::IR;
    fn->sym = internSymbol(*sfn.name);

    if (sfn.ret != nullptr) {
      if (auto ret = sema::Type::from(alloc, sfn.ret)) {
        fn->ret = *ret;
      } else {
        vi->diags->report<Error>(
          sfn.ret->loc,
          fmt::format("Function '{}' has invalid return type '{}'",
                      sfn.name->toStringView(), ret.error()));
      }
    } else {
      debug::unimplemented();
    }

    for (const auto& astParm : sfn.parms) {
      Parm parm;
      parm.sym = internSymbol(*astParm->sym);

      if (auto type = sema::Type::from(alloc, sfn.ret)) {
        parm.type = *type;
      } else {
        vi->diags->report<Error>(
          sfn.ret->loc,
          fmt::format("Function parameter '{}' has invalid type '{}'",
                      astParm->sym->toStringView(), type.error()));
      }
    }

    sfn.scp->accept(*this, vi);
    fn->body = dynamic_cast<StmtBlock*>(vi->result);
    vi->result = fn;
  }

  void visit(const ast::StmtStructDecl&, VisitInfo* raw) override {}
  void visit(const ast::StmtTypeDecl&, VisitInfo* raw) override {}
  void visit(const ast::StmtUsing&, VisitInfo* raw) override {}
  void visit(const ast::StmtEmpty&, VisitInfo* raw) override {}
  void visit(const ast::StmtExpr&, VisitInfo* raw) override {}
};

IrTree Builder::build()
{
  IrTree tree;
  ExprVisitor evis;
  StmtVisitInfo vi;
  vi.diags = &mDiags;
  vi.module = mModule;
  vi.evis = &evis;

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
