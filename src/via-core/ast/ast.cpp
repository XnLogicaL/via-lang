/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "ast.h"
#include "debug.h"

namespace ast = via::ast;

using Tk = via::Token::Kind;

inline via::usize ZERO = 0;

#define INDENT std::string(depth, ' ')

via::UnaryOp via::toUnaryOp(Tk kind) noexcept
{
  switch (kind) {
    case Tk::OP_AMP:
      return UnaryOp::REF;
    case Tk::OP_MINUS:
      return UnaryOp::NOT;
    case Tk::KW_NOT:
      return UnaryOp::NOT;
    case Tk::OP_TILDE:
      return UnaryOp::BNOT;
    default:
      break;
  }

  via::debug::bug("unmapped UnaryOp TokenKind");
}

via::BinaryOp via::toBinaryOp(Tk kind) noexcept
{
  switch (kind) {
    case Tk::OP_PLUS:
      return BinaryOp::ADD;
    case Tk::OP_MINUS:
      return BinaryOp::SUB;
    case Tk::OP_STAR:
      return BinaryOp::MUL;
    case Tk::OP_SLASH:
      return BinaryOp::DIV;
    case Tk::OP_STAR_STAR:
      return BinaryOp::POW;
    case Tk::OP_PERCENT:
      return BinaryOp::MOD;
    case Tk::KW_AND:
      return BinaryOp::AND;
    case Tk::KW_OR:
      return BinaryOp::OR;
    case Tk::OP_AMP:
      return BinaryOp::BAND;
    case Tk::OP_PIPE:
      return BinaryOp::BOR;
    case Tk::OP_CARET:
      return BinaryOp::BXOR;
    case Tk::OP_SHL:
      return BinaryOp::BSHL;
    case Tk::OP_SHR:
      return BinaryOp::BSHR;
    default:
      break;
  }

  via::debug::bug("unmapped BinaryOp TokenKind");
}

std::string ast::AccessIdent::dump() const
{
  return std::format(
    "AccessIdent(instantiated={}, symbol={}, generics={})", inst,
    symbol->dump(),
    debug::dump(gens, [](const auto& tp) { return tp->dump(ZERO); }));
}

std::string ast::Path::dump() const
{
  return std::format("Path({})", debug::dump(path, [](const auto& node) {
                       return node->dump();
                     }));
}

std::string ast::Parameter::dump() const
{
  return std::format("Parameter(sym={}, type={})", sym->dump(),
                     type->dump(ZERO));
}

std::string ast::AttributeGroup::dump() const
{
  return std::format(
    "AttributeGroup({})", debug::dump(ats, [](const auto& atr) {
      return std::format(
        "Attribute(sp={}, args={})", atr.sp->dump(),
        debug::dump(atr.args, [](const auto& arg) { return arg->dump(); }));
    }));
}

std::string ast::ExprLit::dump(usize&) const
{
  return std::format("ExprLit({})", tok->dump());
}

std::string ast::ExprSymbol::dump(usize&) const
{
  return std::format("ExprSymbol({})", sym->dump());
}

std::string ast::ExprDynAccess::dump(usize&) const
{
  return std::format("ExprDynAccess({}, {})", root->dump(ZERO),
                     index->toString());
}

std::string ast::ExprStaticAccess::dump(usize&) const
{
  return std::format("ExprStaticAccess({}, {})", root->dump(ZERO),
                     index->toString());
}

std::string ast::ExprUnary::dump(usize&) const
{
  return std::format("ExprUnary({}, {})", op->dump(), expr->dump(ZERO));
}

std::string ast::ExprBinary::dump(usize&) const
{
  return std::format("ExprBinary({}, {}, {})", op->dump(), lhs->dump(ZERO),
                     rhs->dump(ZERO));
}

std::string ast::ExprGroup::dump(usize&) const
{
  return std::format("ExprGroup({})", expr->dump(ZERO));
}

std::string ast::ExprCall::dump(usize&) const
{
  return std::format(
    "ExprCall(callee={}, args={})", lval->dump(ZERO),
    debug::dump(args, [](const auto& arg) { return arg->dump(ZERO); }));
}

std::string ast::ExprSubscript::dump(usize&) const
{
  return std::format("ExprSubscript({}, {})", lval->dump(ZERO),
                     idx->dump(ZERO));
}

std::string ast::ExprCast::dump(usize&) const
{
  return std::format("ExprCast({}, {})", expr->dump(ZERO), type->dump(ZERO));
}

std::string ast::ExprTernary::dump(usize&) const
{
  return std::format("ExprTernary(cnd={}, lhs={}, rhs={})", cnd->dump(ZERO),
                     lhs->dump(ZERO), rhs->dump(ZERO));
}

std::string ast::ExprArray::dump(usize&) const
{
  return std::format(
    "ExprArray(init={})",
    debug::dump(init, [](const auto& ini) { return ini->dump(ZERO); }));
}

std::string ast::ExprTuple::dump(usize&) const
{
  return std::format(
    "ExprTuple(vals={})",
    debug::dump(vals, [](const auto& ini) { return ini->dump(ZERO); }));
}

std::string ast::ExprLambda::dump(usize&) const
{
  return std::format("ExprLambda(<lambda>)");
}

std::string ast::StmtVarDecl::dump(usize& depth) const
{
  return INDENT + std::format("StmtVarDecl(lval={}, rval={}, type={})",
                              lval->dump(ZERO), rval->dump(ZERO),
                              type ? type->dump(ZERO) : "<infered>");
}

std::string ast::StmtScope::dump(usize& depth) const
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

std::string ast::StmtIf::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT << "StmtIf()\n";
  depth++;

  for (const auto& br : brs) {
    oss << INDENT
        << std::format("Branch({})\n", br.cnd ? br.cnd->dump(ZERO) : "");
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

std::string ast::StmtFor::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT
      << std::format("StmtFor(init={}, target={}, step={})\n", init->dump(ZERO),
                     target->dump(ZERO), step ? step->dump(ZERO) : "<infered>");
  depth++;

  for (const Stmt* stmt : br->stmts) {
    oss << stmt->dump(depth) << "\n";
  }

  depth--;
  oss << INDENT << "EndFor()";
  return oss.str();
}

std::string ast::StmtForEach::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT
      << std::format("StmtForEach(lval={}, iter={})\n", lval->dump(ZERO),
                     iter->dump(ZERO));
  depth++;

  for (const Stmt* stmt : br->stmts) {
    oss << stmt->dump(depth) << "\n";
  }

  depth--;
  oss << INDENT << "EndForEach()";
  return oss.str();
}

std::string ast::StmtWhile::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT << std::format("StmtWhile(cnd={})\n", cnd->dump(ZERO));
  depth++;

  for (const Stmt* stmt : br->stmts) {
    oss << stmt->dump(depth) << "\n";
  }

  depth--;
  oss << INDENT << "EndWhile()";
  return oss.str();
}

std::string ast::StmtAssign::dump(usize& depth) const
{
  return INDENT + std::format("StmtAssign(op={}, lval={}, rval={})", op->dump(),
                              lval->dump(ZERO), rval->dump(ZERO));
}

std::string ast::StmtReturn::dump(usize& depth) const
{
  return INDENT + std::format("StmtReturn({})", expr->dump(ZERO));
}

std::string ast::StmtEnum::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT
      << std::format("StmtEnum(sym={}, type={})\n", sym->dump(),
                     type->dump(ZERO));
  depth++;

  for (const auto& entry : pairs) {
    oss << std::format("EnumEntry(sym={}, expr={})\n", entry.sym->dump(),
                       entry.expr->dump(ZERO));
  }

  depth--;
  oss << INDENT << "EndEnum()";
  return oss.str();
}

std::string ast::StmtModule::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT << std::format("StmtModule({})\n", sym->dump());
  depth++;

  for (const auto& stmt : scp) {
    oss << stmt->dump(depth) << "\n";
  }

  depth--;
  oss << INDENT << "EndModule()";
  return oss.str();
}

std::string ast::StmtImport::dump(usize& depth) const
{
  return INDENT +
         std::format("StmtImport({})", debug::dump(path, [](const auto& node) {
                       return node->dump();
                     }));
}

std::string ast::StmtFunctionDecl::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT + std::format("StmtFunctionDecl(name={}, ret={}, parms={})\n",
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

std::string ast::StmtStructDecl::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT + std::format("StmtStructDecl({})\n", name->dump());
  depth++;

  for (const auto& stmt : scp) {
    oss << stmt->dump(depth) << "\n";
  }

  depth--;
  oss << INDENT << "EndStructDecl()";
  return oss.str();
}

std::string ast::StmtTypeDecl::dump(usize& depth) const
{
  return INDENT + std::format("StmtTypeDecl(sym={}, type={})", sym->dump(),
                              type->dump(ZERO));
}

std::string ast::StmtUsing::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT + std::format("StmtUsing({})\n", sp->dump());
  depth++;

  for (const auto& stmt : scp->stmts) {
    oss << stmt->dump(depth) << "\n";
  }

  depth--;
  oss << INDENT << "EndUsing()";
  return oss.str();
}

std::string ast::StmtEmpty::dump(usize& depth) const
{
  return INDENT + "StmtEmpty()";
}

std::string ast::StmtExpr::dump(usize& depth) const
{
  return INDENT + std::format("StmtExpr({})", expr->dump(ZERO));
}

std::string ast::TypeBuiltin::dump(usize&) const
{
  return std::format("TypeBuiltin({})", tok->dump());
}

std::string ast::TypeArray::dump(usize&) const
{
  return std::format("TypeArray({})", type->dump(ZERO));
}

std::string ast::TypeDict::dump(usize&) const
{
  return std::format("TypeDict(key={}, val={})", key->dump(ZERO),
                     val->dump(ZERO));
}

std::string ast::TypeFunc::dump(usize&) const
{
  return std::format(
    "TypeFunc(ret={}, parms={})", ret->dump(ZERO),
    debug::dump(params, [](const auto& parm) { return parm->dump(); }));
}

bool ast::isLValue(const Expr* expr) noexcept
{
  return TRY_IS(const ExprSymbol, expr) ||
         TRY_IS(const ExprStaticAccess, expr) ||
         TRY_IS(const ExprDynAccess, expr) ||
         TRY_IS(const ExprSubscript, expr) || TRY_IS(const ExprTuple, expr);
}

[[nodiscard]] std::string via::debug::dump(const via::SyntaxTree& ast)
{
  std::ostringstream oss;
  usize depth = 0;

  for (const auto* st : ast) {
    oss << st->dump(depth) << "\n";
  }

  return oss.str();
}
