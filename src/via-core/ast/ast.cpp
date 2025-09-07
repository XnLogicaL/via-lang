// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "ast.h"

namespace via
{

namespace ast
{

inline usize ZERO = 0;

#define INDENT std::string(depth, ' ')

std::string AccessIdent::dump() const
{
  return fmt::format(
      "AccessIdent(instantiated={}, symbol={}, generics={})", inst,
      symbol->dump(),
      debug::dump(gens, [](const auto& tp) { return tp->dump(ZERO); }));
}

std::string Path::dump() const
{
  return fmt::format("Path({})", debug::dump(path, [](const auto& node) {
                       return node->dump();
                     }));
}

std::string Parameter::dump() const
{
  return fmt::format("Parameter(sym={}, type={})", sym->dump(),
                     type->dump(ZERO));
}

std::string AttributeGroup::dump() const
{
  return fmt::format(
      "AttributeGroup({})", debug::dump(ats, [](const auto& atr) {
        return fmt::format(
            "Attribute(sp={}, args={})", atr.sp->dump(),
            debug::dump(atr.args, [](const auto& arg) { return arg->dump(); }));
      }));
}

std::string ExprLit::dump(usize&) const
{
  return fmt::format("ExprLit({})", tok->dump());
}

std::string ExprSymbol::dump(usize&) const
{
  return fmt::format("ExprSymbol({})", sym->dump());
}

std::string ExprDynAccess::dump(usize&) const
{
  return fmt::format("ExprDynAccess({}, {})", expr->dump(ZERO),
                     expr->dump(ZERO));
}

std::string ExprStaticAccess::dump(usize&) const
{
  return fmt::format("ExprStaticAccess({}, {})", expr->dump(ZERO), aid->dump());
}

std::string ExprUnary::dump(usize&) const
{
  return fmt::format("ExprUnary({}, {})", op->dump(), expr->dump(ZERO));
}

std::string ExprBinary::dump(usize&) const
{
  return fmt::format("ExprBinary({}, {}, {})", op->dump(), lhs->dump(ZERO),
                     rhs->dump(ZERO));
}

std::string ExprGroup::dump(usize&) const
{
  return fmt::format("ExprGroup({})", expr->dump(ZERO));
}

std::string ExprCall::dump(usize&) const
{
  return fmt::format(
      "ExprCall(callee={}, args={})", lval->dump(ZERO),
      debug::dump(args, [](const auto& arg) { return arg->dump(ZERO); }));
}

std::string ExprSubscript::dump(usize&) const
{
  return fmt::format("ExprSubscript({}, {})", lval->dump(ZERO),
                     idx->dump(ZERO));
}

std::string ExprCast::dump(usize&) const
{
  return fmt::format("ExprCast({}, {})", expr->dump(ZERO), type->dump(ZERO));
}

std::string ExprTernary::dump(usize&) const
{
  return fmt::format("ExprTernary(cnd={}, lhs={}, rhs={})", cnd->dump(ZERO),
                     lhs->dump(ZERO), rhs->dump(ZERO));
}

std::string ExprArray::dump(usize&) const
{
  return fmt::format(
      "ExprArray(init={})",
      debug::dump(init, [](const auto& ini) { return ini->dump(ZERO); }));
}

std::string ExprTuple::dump(usize&) const
{
  return fmt::format(
      "ExprTuple(vals={})",
      debug::dump(vals, [](const auto& ini) { return ini->dump(ZERO); }));
}

std::string ExprLambda::dump(usize&) const
{
  return fmt::format("ExprLambda(<lambda>)");
}

std::string StmtVarDecl::dump(usize& depth) const
{
  return INDENT + fmt::format("StmtVarDecl(lval={}, rval={}, type={})",
                              lval->dump(ZERO), rval->dump(ZERO),
                              type ? type->dump(ZERO) : "<infered>");
}

std::string StmtScope::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT << "StmtScope()\n";
  depth++;

  for (const Stmt* stmt : stmts) {
    oss << stmt->dump(depth) << "\n";
  }

  depth--;
  oss << INDENT << "EndScope()";
  return oss.str();
}

std::string StmtIf::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT << "StmtIf()\n";
  depth++;

  for (const auto& br : brs) {
    oss << INDENT
        << fmt::format("Branch({})\n", br.cnd ? br.cnd->dump(ZERO) : "");
    depth++;

    for (const Stmt* stmt : br.br->stmts) {
      oss << stmt->dump(depth) << "\n";
    }

    depth--;
    oss << INDENT << "EndBranch()\n";
  }

  depth--;
  oss << INDENT << "EndIf()";
  return oss.str();
}

std::string StmtFor::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT
      << fmt::format("StmtFor(init={}, target={}, step={})\n", init->dump(ZERO),
                     target->dump(ZERO), step ? step->dump(ZERO) : "<infered>");
  depth++;

  for (const Stmt* stmt : br->stmts) {
    oss << stmt->dump(depth) << "\n";
  }

  depth--;
  oss << INDENT << "EndFor()";
  return oss.str();
}

std::string StmtForEach::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT
      << fmt::format("StmtForEach(lval={}, iter={})\n", lval->dump(ZERO),
                     iter->dump(ZERO));
  depth++;

  for (const Stmt* stmt : br->stmts) {
    oss << stmt->dump(depth) << "\n";
  }

  depth--;
  oss << INDENT << "EndForEach()";
  return oss.str();
}

std::string StmtWhile::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT << fmt::format("StmtWhile(cnd={})\n", cnd->dump(ZERO));
  depth++;

  for (const Stmt* stmt : br->stmts) {
    oss << stmt->dump(depth) << "\n";
  }

  depth--;
  oss << INDENT << "EndWhile()";
  return oss.str();
}

std::string StmtAssign::dump(usize& depth) const
{
  return INDENT + fmt::format("StmtAssign(op={}, lval={}, rval={})", op->dump(),
                              lval->dump(ZERO), rval->dump(ZERO));
}

std::string StmtReturn::dump(usize& depth) const
{
  return INDENT + fmt::format("StmtReturn({})", expr->dump(ZERO));
}

std::string StmtEnum::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT
      << fmt::format("StmtEnum(sym={}, type={})\n", sym->dump(),
                     type->dump(ZERO));
  depth++;

  for (const auto& entry : pairs) {
    oss << fmt::format("EnumEntry(sym={}, expr={})\n", entry.sym->dump(),
                       entry.expr->dump(ZERO));
  }

  depth--;
  oss << INDENT << "EndEnum()";
  return oss.str();
}

std::string StmtModule::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT << fmt::format("StmtModule({})\n", sym->dump());
  depth++;

  for (const auto& stmt : scp) {
    oss << stmt->dump(depth) << "\n";
  }

  depth--;
  oss << INDENT << "EndModule()";
  return oss.str();
}

std::string StmtImport::dump(usize& depth) const
{
  return INDENT +
         fmt::format("StmtImport({})", debug::dump(path, [](const auto& node) {
                       return node->dump();
                     }));
}

std::string StmtFunctionDecl::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT + fmt::format("StmtFunctionDecl(name={}, ret={}, parms={})\n",
                              name->dump(), ret->dump(ZERO),
                              debug::dump(parms, [](const auto& parm) {
                                return parm->dump();
                              }));
  depth++;

  for (const auto& stmt : scp->stmts) {
    oss << stmt->dump(depth) << "\n";
  }

  depth--;
  oss << INDENT << "EndFunctionDecl()";
  return oss.str();
}

std::string StmtStructDecl::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT + fmt::format("StmtStructDecl({})\n", name->dump());
  depth++;

  for (const auto& stmt : scp) {
    oss << stmt->dump(depth) << "\n";
  }

  depth--;
  oss << INDENT << "EndStructDecl()";
  return oss.str();
}

std::string StmtTypeDecl::dump(usize& depth) const
{
  return INDENT + fmt::format("StmtTypeDecl(sym={}, type={})", sym->dump(),
                              type->dump(ZERO));
}

std::string StmtUsing::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT + fmt::format("StmtUsing({})\n", sp->dump());
  depth++;

  for (const auto& stmt : scp->stmts) {
    oss << stmt->dump(depth) << "\n";
  }

  depth--;
  oss << INDENT << "EndUsing()";
  return oss.str();
}

std::string StmtEmpty::dump(usize& depth) const
{
  return INDENT + "StmtEmpty()";
}

std::string StmtExpr::dump(usize& depth) const
{
  return INDENT + fmt::format("StmtExpr({})", expr->dump(ZERO));
}

std::string TypeBuiltin::dump(usize&) const
{
  return fmt::format("TypeBuiltin({})", tok->dump());
}

std::string TypeArray::dump(usize&) const
{
  return fmt::format("TypeArray({})", type->dump(ZERO));
}

std::string TypeDict::dump(usize&) const
{
  return fmt::format("TypeDict(key={}, val={})", key->dump(ZERO),
                     val->dump(ZERO));
}

std::string TypeFunc::dump(usize&) const
{
  return fmt::format(
      "TypeFunc(ret={}, parms={})", ret->dump(ZERO),
      debug::dump(params, [](const auto& parm) { return parm->dump(); }));
}

}  // namespace ast

namespace debug
{

[[nodiscard]] std::string dump(const SyntaxTree& ast)
{
  std::ostringstream oss;
  usize depth = 0;

  for (const auto* st : ast) {
    oss << st->dump(depth) << "\n";
  }

  return oss.str();
}

}  // namespace debug

}  // namespace via
