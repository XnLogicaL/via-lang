/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "Parser/ast.h"

namespace via::Compilation
{

double _eval_binexpr(const Parsing::AST::BinExprNode &bin_expr);
double _eval_litexpr(const Parsing::AST::LitExprNode &lit_expr);
double _eval_expr(const Parsing::AST::ExprNode &expr);
bool _is_constexpr(const Parsing::AST::ExprNode &expr);

// Folds a constant expression into a single literal expression
// Only works with integer or float expressions for now
// ! This is a pre-compilation optimization
void optimize_constfold(Parsing::AST::ExprNode &expr);

} // namespace via::Compilation