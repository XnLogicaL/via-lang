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
#define SYMBOL(ID) (symtab->lookup(ID).valueOr("<unknown-symbol>"))
#define DUMP_IF(PTR, ...)                                               \
  (PTR ? PTR->dump(__VA_ARGS__)                                         \
       : [](const SymbolTable* symtab = nullptr, usize& depth = ZERO) { \
           return INDENT + "<null>";                                    \
         }(__VA_ARGS__))

std::string ir::TrReturn::dump(const SymbolTable* symtab, usize& depth) const
{
  return INDENT + std::format("return {} {}", DUMP_IF(val, symtab, ZERO),
                              implicit ? "(implicit)" : "");
}

std::string ir::TrContinue::dump(const SymbolTable* symtab, usize& depth) const
{
  return INDENT + "continue";
}

std::string ir::TrBreak::dump(const SymbolTable* symtab, usize& depth) const
{
  return INDENT + "break";
}

std::string ir::TrBranch::dump(const SymbolTable* symtab, usize& depth) const
{
  return INDENT + std::format("br {}", target->name);
}

std::string ir::TrCondBranch::dump(const SymbolTable* symtab,
                                   usize& depth) const
{
  return INDENT + std::format("cndbr {} ? {} : {}", DUMP_IF(cnd, symtab, ZERO),
                              trueTarget->name, falseTarget->name);
}

std::string ir::Parm::dump(const SymbolTable* symtab, usize& depth) const
{
  return std::format("{}: {}", SYMBOL(symbol), DUMP_IF(type));
}

std::string ir::ExprConstant::dump(const SymbolTable* symtab, usize&) const
{
  return value.dump();
}

std::string ir::ExprSymbol::dump(const SymbolTable* symtab, usize&) const
{
  return std::string(SYMBOL(symbol));
}

std::string ir::ExprAccess::dump(const SymbolTable* symtab, usize&) const
{
  return std::format("{}{}{}", DUMP_IF(root, symtab, ZERO),
                     kind == Kind::DYNAMIC ? "." : "::", SYMBOL(index));
}

std::string ir::ExprModuleAccess::dump(const SymbolTable* symtab, usize&) const
{
  return std::format("module<{}>::{} def@{}", module->getName(), SYMBOL(index),
                     reinterpret_cast<const void*>(def));
}

std::string ir::ExprUnary::dump(const SymbolTable* symtab, usize&) const
{
  return std::format("unop( {}: {} )", magic_enum::enum_name(op),
                     DUMP_IF(expr, symtab, ZERO));
}

std::string ir::ExprBinary::dump(const SymbolTable* symtab, usize&) const
{
  return std::format("binop( {}: {}, {})", magic_enum::enum_name(op),
                     DUMP_IF(lhs, symtab, ZERO), DUMP_IF(rhs, symtab, ZERO));
}

std::string ir::ExprCall::dump(const SymbolTable* symtab, usize&) const
{
  return std::format("call( {}, args: {} )", DUMP_IF(callee, symtab, ZERO),
                     debug::dump<const Expr*>(args, [&](const auto& expr) {
                       return DUMP_IF(expr, symtab, ZERO);
                     }));
}

std::string ir::ExprSubscript::dump(const SymbolTable* symtab, usize&) const
{
  return std::format("subscr( {}, {} )", DUMP_IF(expr, symtab, ZERO),
                     DUMP_IF(idx, symtab, ZERO));
}

std::string ir::ExprCast::dump(const SymbolTable* symtab, usize&) const
{
  return std::format("{} as {}", DUMP_IF(expr, symtab, ZERO), "");
}

std::string ir::ExprTuple::dump(const SymbolTable* symtab, usize&) const
{
  return "<tuple>";
}

std::string ir::ExprLambda::dump(const SymbolTable* symtab, usize&) const
{
  return "<lambda>";
}

std::string ir::StmtVarDecl::dump(const SymbolTable* symtab, usize& depth) const
{
  return INDENT + std::format("local {}: {} = {}", SYMBOL(symbol),
                              DUMP_IF(declType), DUMP_IF(expr, symtab, ZERO));
}

std::string ir::StmtFuncDecl::dump(const SymbolTable* symtab,
                                   usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT
      << std::format(
           "function {} {} -> {}:\n", SYMBOL(symbol),
           debug::dump<Parm, '(', ')'>(
             parms, [&](const auto& parm) { return parm.dump(symtab, ZERO); }),
           ret->dump());
  oss << INDENT << "{\n";
  depth++;

  for (const Stmt* stmt : body->stmts) {
    oss << DUMP_IF(stmt, symtab, depth) << "\n";
  }

  oss << INDENT << DUMP_IF(body->term, symtab, ZERO) << "\n";
  depth--;
  oss << INDENT << "}";
  return oss.str();
}

std::string ir::StmtBlock::dump(const SymbolTable* symtab, usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT << "block " << SYMBOL(name) << ":\n";
  oss << INDENT << "{\n";
  depth++;

  for (const Stmt* stmt : stmts) {
    oss << DUMP_IF(stmt, symtab, depth) << "\n";
  }

  oss << INDENT << (term ? DUMP_IF(term, symtab, ZERO) : "<no-terminator>")
      << "\n";
  depth--;
  oss << INDENT << "}";
  return oss.str();
}

std::string ir::StmtExpr::dump(const SymbolTable* symtab, usize& depth) const
{
  return INDENT + DUMP_IF(expr, symtab, ZERO);
}

[[nodiscard]] std::string via::debug::dump(const SymbolTable& symtab,
                                           const IRTree& ir)
{
  std::ostringstream oss;
  usize depth = 0;

  for (const auto& node : ir) {
    oss << DUMP_IF(node, &symtab, depth) << "\n";
  }

  return oss.str();
}
