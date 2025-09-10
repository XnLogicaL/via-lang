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
#define DUMP_IF(PTR, ...) (PTR ? PTR->dump(__VA_ARGS__) : "<null>")

std::string ir::TrReturn::dump(usize& depth) const
{
  return INDENT + std::format("return {}", DUMP_IF(val, ZERO));
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
  return INDENT + std::format("condbr {} ? {} : {}", DUMP_IF(cnd, ZERO), iftrue,
                              iffalse);
}

std::string ir::Parm::dump() const
{
  return std::format("{}: {}", SYMBOL(sym), DUMP_IF(type));
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
  return std::format("access {}{}{}", DUMP_IF(root, ZERO),
                     kind == Kind::STATIC ? "::" : ".", SYMBOL(index));
}

std::string ir::ExprUnary::dump(usize&) const
{
  return std::format("unop {} {}", magic_enum::enum_name(op),
                     DUMP_IF(expr, ZERO));
}

std::string ir::ExprBinary::dump(usize&) const
{
  return std::format("binop {} {} {}", DUMP_IF(lhs, ZERO),
                     magic_enum::enum_name(op), DUMP_IF(rhs, ZERO));
}

std::string ir::ExprCall::dump(usize&) const
{
  return std::format(
    "call {}, {}", DUMP_IF(callee, ZERO),
    debug::dump<const Expr*, '[', ']'>(
      args, [](const auto& expr) { return DUMP_IF(expr, ZERO); }));
}

std::string ir::ExprSubscript::dump(usize&) const
{
  return std::format("subscr {}, {}", DUMP_IF(expr, ZERO), idx->dump(ZERO));
}

std::string ir::ExprCast::dump(usize&) const
{
  return std::format("cast {}, {}", DUMP_IF(expr, ZERO), "");
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
  return INDENT + std::format("{} = {}", SYMBOL(sym), DUMP_IF(expr, ZERO));
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

  oss << DUMP_IF(body, depth) << "\n";

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
    oss << DUMP_IF(stmt, depth) << "\n";
  }

  oss << INDENT << (term ? DUMP_IF(term, ZERO) : "<no-terminator>");
  depth--;
  return oss.str();
}

std::string ir::StmtExpr::dump(usize& depth) const
{
  return INDENT + DUMP_IF(expr, ZERO);
}

[[nodiscard]] std::string via::debug::dump(const IRTree& ir)
{
  std::ostringstream oss;
  usize depth = 0;

  for (const auto& node : ir) {
    oss << DUMP_IF(node, depth) << "\n";
  }

  return oss.str();
}
