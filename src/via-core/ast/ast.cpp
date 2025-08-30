// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "ast.h"

namespace via
{

namespace ast
{

inline usize ZERO = 0;

#define INDENT String(depth, ' ')

template <typename T>
static String dump(const Vec<T>& vec,
                   Function<String(const std::remove_cv_t<T>&)>&& fn)
{
  std::ostringstream oss;
  oss << '{';

  for (usize i = 0; i < vec.size(); i++) {
    oss << fn(vec[i]);
    if (i != vec.size() - 1) {
      oss << ", ";
    }
  }

  oss << '}';
  return oss.str();
}

String TupleBinding::dump() const
{
  return fmt::format("TupleBinding({})", ast::dump(binds, [](const auto& bind) {
                       return bind->dump();
                     }));
}

String Path::dump() const
{
  return fmt::format("Path({})", ast::dump(path, [](const auto& node) {
                       return node->dump();
                     }));
}

String LValue::dump() const
{
  switch (kind) {
    case Kind::SYM:
      return fmt::format("LValue(symbol={})", sym->dump());
    case Kind::TPB:
      return fmt::format("LValue(tpb={})", tpb->dump());
    case Kind::SP:
      return fmt::format("LValue(sp={})", path->dump());
    case Kind::DP:
      return fmt::format("LValue(dp={})", path->dump());
  }
}

String PlValue::dump() const
{
  switch (kind) {
    case Kind::SYM:
      return fmt::format("PlValue(symbol={})", sym->dump());
    case Kind::SP:
      return fmt::format("PlValue(sp={})", path->dump());
    case Kind::DP:
      return fmt::format("PlValue(dp={})", path->dump());
  }
}

String Parameter::dump() const
{
  return fmt::format("Parameter(sym={}, type={})", sym->dump(),
                     type->dump(ZERO));
}

String AttributeGroup::dump() const
{
  return fmt::format(
      "AttributeGroup({})", ast::dump(ats, [](const auto& atr) {
        return fmt::format(
            "Attribute(sp={}, args={})", atr.sp->dump(),
            ast::dump(atr.args, [](const auto& arg) { return arg->dump(); }));
      }));
}

String ExprLit::dump(usize&) const
{
  return fmt::format("ExprLit({})", tok->dump());
}

String ExprSymbol::dump(usize&) const
{
  return fmt::format("ExprSymbol({})", sym->dump());
}

String ExprDynAccess::dump(usize&) const
{
  return fmt::format("ExprDynAccess({}, {})", expr->dump(ZERO),
                     expr->dump(ZERO));
}

String ExprStaticAccess::dump(usize&) const
{
  return fmt::format("ExprStaticAccess({}, {})", expr->dump(ZERO),
                     expr->dump(ZERO));
}

String ExprUnary::dump(usize&) const
{
  return fmt::format("ExprUnary({}, {})", op->dump(), expr->dump(ZERO));
}

String ExprBinary::dump(usize&) const
{
  return fmt::format("ExprBinary({}, {}, {})", op->dump(), lhs->dump(ZERO),
                     rhs->dump(ZERO));
}

String ExprGroup::dump(usize&) const
{
  return fmt::format("ExprGroup({})", expr->dump(ZERO));
}

String ExprCall::dump(usize&) const
{
  return fmt::format(
      "ExprCall(callee={}, args={})", lval->dump(ZERO),
      ast::dump(args, [](const auto& arg) { return arg->dump(ZERO); }));
}

String ExprSubscript::dump(usize&) const
{
  return fmt::format("ExprSubscript({}, {})", lval->dump(ZERO),
                     idx->dump(ZERO));
}

String ExprCast::dump(usize&) const
{
  return fmt::format("ExprCast({}, {})", expr->dump(ZERO), type->dump(ZERO));
}

String ExprTernary::dump(usize&) const
{
  return fmt::format("ExprTernary(cnd={}, lhs={}, rhs={})", cnd->dump(ZERO),
                     lhs->dump(ZERO), rhs->dump(ZERO));
}

String ExprArray::dump(usize&) const
{
  return fmt::format("ExprArray(init={})", ast::dump(init, [](const auto& ini) {
                       return ini->dump(ZERO);
                     }));
}

String ExprTuple::dump(usize&) const
{
  return fmt::format("ExprTuple(vals={})", ast::dump(vals, [](const auto& ini) {
                       return ini->dump(ZERO);
                     }));
}

String ExprLambda::dump(usize&) const
{
  return fmt::format("ExprLambda(<lambda>)");
}

String StmtVarDecl::dump(usize& depth) const
{
  return INDENT + fmt::format("StmtVarDecl(lval={}, rval={}, type={})",
                              lval->dump(), rval->dump(ZERO),
                              type ? type->dump(ZERO) : "<no-annotated-type>");
}

String StmtScope::dump(usize& depth) const
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

String StmtIf::dump(usize& depth) const
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

String StmtFor::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT
      << fmt::format("StmtFor(init={}, target={}, step={})\n", init->dump(ZERO),
                     target->dump(ZERO), step->dump(ZERO));
  depth++;

  for (const Stmt* stmt : br->stmts) {
    oss << stmt->dump(depth) << "\n";
  }

  depth--;
  oss << INDENT << "EndFor()";
  return oss.str();
}

String StmtForEach::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT
      << fmt::format("StmtForEach(lval={}, iter={})\n", lval->dump(),
                     iter->dump(ZERO));
  depth++;

  for (const Stmt* stmt : br->stmts) {
    oss << stmt->dump(depth) << "\n";
  }

  depth--;
  oss << INDENT << "EndForEach()";
  return oss.str();
}

String StmtWhile::dump(usize& depth) const
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

String StmtAssign::dump(usize& depth) const
{
  return INDENT + fmt::format("StmtAssign(op={}, lval={}, rval={})", op->dump(),
                              lval->dump(ZERO), rval->dump(ZERO));
}

String StmtReturn::dump(usize& depth) const
{
  return INDENT + fmt::format("StmtReturn({})", expr->dump(ZERO));
}

String StmtEnum::dump(usize& depth) const
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

String StmtModule::dump(usize& depth) const
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

String StmtImport::dump(usize& depth) const
{
  return INDENT +
         fmt::format("StmtImport({})", ast::dump(path, [](const auto& node) {
                       return node->dump();
                     }));
}

String StmtFunctionDecl::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT + fmt::format("StmtFunctionDecl(name={}, ret={}, parms={})\n",
                              name->dump(), ret->dump(ZERO),
                              ast::dump(parms, [](const auto& parm) {
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

String StmtStructDecl::dump(usize& depth) const
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

String StmtTypeDecl::dump(usize& depth) const
{
  return INDENT + fmt::format("StmtTypeDecl(sym={}, type={})", sym->dump(),
                              type->dump(ZERO));
}

String StmtUsing::dump(usize& depth) const
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

String StmtEmpty::dump(usize& depth) const
{
  return INDENT + "StmtEmpty()";
}

String StmtExpr::dump(usize& depth) const
{
  return INDENT + fmt::format("StmtExpr({})", expr->dump(ZERO));
}

String TypeBuiltin::dump(usize&) const
{
  return fmt::format("TypeBuiltin({})", tok->dump());
}

String TypeArray::dump(usize&) const
{
  return fmt::format("TypeArray({})", type->dump(ZERO));
}

String TypeDict::dump(usize&) const
{
  return fmt::format("TypeDict(key={}, val={})", key->dump(ZERO),
                     val->dump(ZERO));
}

String TypeFunc::dump(usize&) const
{
  return fmt::format(
      "TypeFunc(ret={}, parms={})", ret->dump(ZERO),
      ast::dump(params, [](const auto& parm) { return parm->dump(); }));
}

}  // namespace ast

namespace debug
{

[[nodiscard]] String dump(const SyntaxTree& ast)
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
