// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "ir.h"

namespace via
{

namespace ir
{

inline usize ZERO = 0;

#define INDENT String(depth, ' ')
#define SYMBOL(id) \
  (SymbolTable::getInstance().lookup(id).value_or("<unknown-symbol>"))

String TrReturn::dump(usize& depth) const
{
  return INDENT + fmt::format("return {}", val->dump(ZERO));
}

String TrContinue::dump(usize& depth) const
{
  return INDENT + "continue";
}

String TrBreak::dump(usize& depth) const
{
  return INDENT + "break";
}

String TrBranch::dump(usize& depth) const
{
  return INDENT + fmt::format("br {}", lbl);
}

String TrCondBranch::dump(usize& depth) const
{
  return INDENT +
         fmt::format("condbr {} ? {} : {}", cnd->dump(ZERO), iftrue, iffalse);
}

String Parm::dump() const
{
  return fmt::format("{}: {}", SYMBOL(sym), type->dump());
}

String ExprConstant::dump(usize&) const
{
  return cv.dump();
}

String ExprSymbol::dump(usize&) const
{
  return String(SYMBOL(symbol));
}

String ExprAccess::dump(usize&) const
{
  return fmt::format("access {}{}{}", lval->dump(ZERO),
                     kind == Kind::STATIC ? "::" : ".", idx->dump(ZERO));
}

String ExprUnary::dump(usize&) const
{
  return fmt::format("unop {} {}", magic_enum::enum_name(op), expr->dump(ZERO));
}

String ExprBinary::dump(usize&) const
{
  return fmt::format("binop {} {} {}", lhs->dump(ZERO),
                     magic_enum::enum_name(op), rhs->dump(ZERO));
}

String ExprCall::dump(usize&) const
{
  return fmt::format("call {}, {}", callee->dump(ZERO),
                     debug::dump<Expr*, '[', ']'>(args, [](const auto& expr) {
                       return expr->dump(ZERO);
                     }));
}

String ExprSubscript::dump(usize&) const
{
  return fmt::format("subscr {}, {}", expr->dump(ZERO), idx->dump(ZERO));
}

String ExprCast::dump(usize&) const
{
  return fmt::format("cast {}, {}", expr->dump(ZERO), type->dump());
}

String ExprTuple::dump(usize&) const
{
  return "<tuple>";
}

String ExprLambda::dump(usize&) const
{
  return "<lambda>";
}

String StmtVarDecl::dump(usize& depth) const
{
  return INDENT + fmt::format("var {} = {}", SYMBOL(sym), expr->dump(ZERO));
}

String StmtFuncDecl::dump(usize& depth) const
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

String StmtBlock::dump(usize& depth) const
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

[[nodiscard]] String dump(const IrTree& ir)
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
