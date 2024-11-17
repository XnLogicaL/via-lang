/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "gen.h"

#include "Parser/ast.h"
#include "Lexer/token.h"

namespace via::Compilation
{

size_t compile_lit_expr(Generator *gen, Parsing::AST::LitExprNode lit);
size_t compile_un_expr(Generator *gen, Parsing::AST::UnExprNode un);
size_t compile_binary_expr(Generator *gen, Parsing::AST::BinExprNode bin_expr);
size_t compile_index_expr(Generator *gen, Parsing::AST::IndexExprNode idx_expr);
size_t compile_expression(Generator *gen, Parsing::AST::ExprNode expr);

} // namespace via::Compilation
