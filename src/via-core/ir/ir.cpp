// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "ir.h"

namespace via
{

namespace ir
{

inline usize ZERO = 0;

#define INDENT std::string(depth, ' ')
#define SYMBOL(id) \
  (SymbolTable::getInstance().lookup(id).value_or("<unknown-symbol>"))

std::string TrReturn::dump(usize& depth) const
{
  return INDENT + fmt::format("return {}", val->dump(ZERO));
}

std::string TrContinue::dump(usize& depth) const
{
  return INDENT + "continue";
}

std::string TrBreak::dump(usize& depth) const
{
  return INDENT + "break";
}

std::string TrBranch::dump(usize& depth) const
{
  return INDENT + fmt::format("br {}", lbl);
}

std::string TrCondBranch::dump(usize& depth) const
{
  return INDENT +
         fmt::format("condbr {} ? {} : {}", cnd->dump(ZERO), iftrue, iffalse);
}

std::string Parm::dump() const
{
  return fmt::format("{}: {}", SYMBOL(sym), type->dump());
}

std::string ExprConstant::dump(usize&) const
{
  return cv.dump();
}

std::string ExprSymbol::dump(usize&) const
{
  return std::string(SYMBOL(symbol));
}

std::string ExprAccess::dump(usize&) const
{
  return fmt::format("access {}{}{}", lval->dump(ZERO),
                     kind == Kind::STATIC ? "::" : ".", idx->dump(ZERO));
}

std::string ExprUnary::dump(usize&) const
{
  return fmt::format("unop {} {}", magic_enum::enum_name(op), expr->dump(ZERO));
}

std::string ExprBinary::dump(usize&) const
{
  return fmt::format("binop {} {} {}", lhs->dump(ZERO),
                     magic_enum::enum_name(op), rhs->dump(ZERO));
}

std::string ExprCall::dump(usize&) const
{
  return fmt::format("call {}, {}", callee->dump(ZERO),
                     debug::dump<Expr*, '[', ']'>(args, [](const auto& expr) {
                       return expr->dump(ZERO);
                     }));
}

std::string ExprSubscript::dump(usize&) const
{
  return fmt::format("subscr {}, {}", expr->dump(ZERO), idx->dump(ZERO));
}

std::string ExprCast::dump(usize&) const
{
  return fmt::format("cast {}, {}", expr->dump(ZERO), type->dump());
}

std::string ExprTuple::dump(usize&) const
{
  return "<tuple>";
}

std::string ExprLambda::dump(usize&) const
{
  return "<lambda>";
}

std::string StmtVarDecl::dump(usize& depth) const
{
  return INDENT + fmt::format("{} = {}", SYMBOL(sym), expr->dump(ZERO));
}

std::string StmtFuncDecl::dump(usize& depth) const
{
  std::ostringstream oss;
  oss << INDENT
      << fmt::format("fn {}{}:\n", SYMBOL(sym),
                     debug::dump<Parm, '(', ')'>(
                         parms, [](const auto& parm) { return parm.dump(); }));

  oss << INDENT << "{\n";
  depth++;

  oss << body->dump(depth) << "\n";

  depth--;
  oss << INDENT << '}';
  return oss.str();
}

std::string StmtBlock::dump(usize& depth) const
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

}  // namespace ir

namespace debug
{

[[nodiscard]] std::string dump(const IrTree& ir)
{
  std::ostringstream oss;
  usize depth = 0;

  for (const auto& node : ir) {
    oss << node->dump(depth) << "\n";
  }

  return oss.str();
}

}  // namespace debug

}  // namespace via
