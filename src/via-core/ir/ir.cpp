/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "ir.h"
#include "module/module.h"

namespace ir = via::ir;

inline via::usize ZERO = 0;

#define INDENT std::string(depth * 2, ' ')
#define SYMBOL(ID) std::format("symbol({})", ID)
#define DUMP_IF(PTR, ...)       \
  (PTR ? PTR->dump(__VA_ARGS__) \
       : [](usize& depth = ZERO) { return INDENT + "<null>"; }(__VA_ARGS__))

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
  return INDENT + std::format("br {}", target->name);
}

std::string ir::TrCondBranch::dump(usize& depth) const
{
  return INDENT + std::format("cndbr {} ? {} : {}", DUMP_IF(cnd, ZERO),
                              trueTarget->name, falseTarget->name);
}

std::string ir::Parm::dump() const
{
  return std::format("{}: {}", sym, DUMP_IF(type));
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
  return std::format("{}{}{}", DUMP_IF(root, ZERO),
                     kind == Kind::DYNAMIC ? "." : "::", SYMBOL(index));
}

std::string ir::ExprModuleAccess::dump(usize&) const
{
  return std::format("module<{}>::{} def@{}", module->getName(), SYMBOL(index),
                     reinterpret_cast<const void*>(def));
}

std::string ir::ExprUnary::dump(usize&) const
{
  return std::format("unop( {}: {} )", magic_enum::enum_name(op),
                     DUMP_IF(expr, ZERO));
}

std::string ir::ExprBinary::dump(usize&) const
{
  return std::format("binop( {}: {}, {})", magic_enum::enum_name(op),
                     DUMP_IF(lhs, ZERO), DUMP_IF(rhs, ZERO));
}

std::string ir::ExprCall::dump(usize&) const
{
  return std::format("call( {}, args: {} )", DUMP_IF(callee, ZERO),
                     debug::dump<const Expr*>(args, [](const auto& expr) {
                       return DUMP_IF(expr, ZERO);
                     }));
}

std::string ir::ExprSubscript::dump(usize&) const
{
  return std::format("subscr( {}, {} )", DUMP_IF(expr, ZERO), idx->dump(ZERO));
}

std::string ir::ExprCast::dump(usize&) const
{
  return std::format("{} as {}", DUMP_IF(expr, ZERO), "");
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
  return INDENT + std::format("local {}: {} = {}", SYMBOL(sym),
                              DUMP_IF(declType), DUMP_IF(expr, ZERO));
}

std::string ir::StmtFuncDecl::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT
      << std::format("function {} {} -> {}:\n", SYMBOL(sym),
                     debug::dump<Parm, '(', ')'>(
                       parms, [](const auto& parm) { return parm.dump(); }),
                     ret->dump());
  oss << INDENT << "{\n";
  depth++;

  for (const Stmt* stmt : body->stmts) {
    oss << DUMP_IF(stmt, depth) << "\n";
  }

  oss << INDENT << DUMP_IF(body->term, ZERO) << "\n";
  depth--;
  oss << INDENT << "}";
  return oss.str();
}

std::string ir::StmtBlock::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT << "block " << SYMBOL(name) << ":\n";
  oss << INDENT << "{\n";
  depth++;

  for (const Stmt* stmt : stmts) {
    oss << DUMP_IF(stmt, depth) << "\n";
  }

  oss << INDENT << (term ? DUMP_IF(term, ZERO) : "<no-terminator>") << "\n";
  depth--;
  oss << INDENT << "}";
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
