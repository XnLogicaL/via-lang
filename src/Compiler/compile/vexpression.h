/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "gen.h"
#include "Parser/ast.h"

#include "instruction.h"
#include "optimize/bshift.h"
#include "optimize/constfold.h"

namespace via::Compilation
{

size_t compile_lit_expr(Generator *, Parsing::AST::LitExprNode);
size_t compile_un_expr(Generator *, Parsing::AST::UnExprNode);
size_t compile_binary_expr(Generator *, Parsing::AST::BinExprNode);
size_t compile_index_expr(Generator *, Parsing::AST::IndexExprNode);
size_t compile_call_expr(Generator *, Parsing::AST::CallExprNode);
size_t compile_ident_expr(Generator *, Parsing::AST::IdentExprNode);
size_t compile_expression(Generator *, Parsing::AST::ExprNode);

} // namespace via::Compilation
