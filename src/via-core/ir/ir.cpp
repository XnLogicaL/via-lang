/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "ir.h"

namespace ir = via::ir;

inline via::usize ZERO = 0;

#define INDENT std::string(depth, ' ')
#define SYMBOL(id) \
  (via::SymbolTable::instance().lookup(id).valueOr("<unknown-symbol>"))

std::string ir::TrReturn::dump(usize& depth) const
{
  return INDENT + std::format("return {}", val->dump(ZERO));
}

std::string ir::TrContinue::dump(usize& depth) const
{
  return INDENT + "continue";
}

std::string ir::TrBreak::dump(usize& depth) const
{
  return INDENT + "break";
}

std::string ir::TrBranch::dump(usize& depth) const
{
  return INDENT + std::format("br {}", lbl);
}

std::string ir::TrCondBranch::dump(usize& depth) const
{
  return INDENT +
         std::format("condbr {} ? {} : {}", cnd->dump(ZERO), iftrue, iffalse);
}

std::string ir::Parm::dump() const
{
  return std::format("{}: {}", SYMBOL(sym), type->dump());
}

std::string ir::ExprConstant::dump(usize&) const
{
  return value.dump();
}

std::string ir::ExprSymbol::dump(usize&) const
{
  return std::string(SYMBOL(symbol));
}

std::string ir::ExprAccess::dump(usize&) const
{
  return std::format("access {}{}{}", lval->dump(ZERO),
                     kind == Kind::STATIC ? "::" : ".", idx->dump(ZERO));
}

std::string ir::ExprUnary::dump(usize&) const
{
  return std::format("unop {} {}", magic_enum::enum_name(op), expr->dump(ZERO));
}

std::string ir::ExprBinary::dump(usize&) const
{
  return std::format("binop {} {} {}", lhs->dump(ZERO),
                     magic_enum::enum_name(op), rhs->dump(ZERO));
}

std::string ir::ExprCall::dump(usize&) const
{
  return std::format(
    "call {}, {}", callee->dump(ZERO),
    debug::dump<const Expr*, '[', ']'>(
      args, [](const auto& expr) { return expr->dump(ZERO); }));
}

std::string ir::ExprSubscript::dump(usize&) const
{
  return std::format("subscr {}, {}", expr->dump(ZERO), idx->dump(ZERO));
}

std::string ir::ExprCast::dump(usize&) const
{
  return std::format("cast {}, {}", expr->dump(ZERO), "");
}

std::string ir::ExprTuple::dump(usize&) const
{
  return "<tuple>";
}

std::string ir::ExprLambda::dump(usize&) const
{
  return "<lambda>";
}

std::string ir::StmtVarDecl::dump(usize& depth) const
{
  return INDENT + std::format("{} = {}", SYMBOL(sym), expr->dump(ZERO));
}

std::string ir::StmtFuncDecl::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT
      << std::format("fn {}{}:\n", SYMBOL(sym),
                     debug::dump<Parm, '(', ')'>(
                       parms, [](const auto& parm) { return parm.dump(); }));

  oss << INDENT << "{\n";
  depth++;

  oss << body->dump(depth) << "\n";

  depth--;
  oss << INDENT << '}';
  return oss.str();
}

std::string ir::StmtBlock::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT << "block " << SYMBOL(name) << ":\n";
  depth++;

  for (const Stmt* stmt : stmts) {
    oss << stmt->dump(depth) << "\n";
  }

  oss << INDENT << (term ? term->dump(ZERO) : "<no-terminator>");
  depth--;
  return oss.str();
}

[[nodiscard]] std::string via::debug::dump(const IRTree& ir)
{
  std::ostringstream oss;
  usize depth = 0;

  for (const auto& node : ir) {
    oss << node->dump(depth) << "\n";
  }

  return oss.str();
}
