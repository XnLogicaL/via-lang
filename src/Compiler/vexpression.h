/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "gen.h"
#include "Parser/ast.h"
#include "builtin.h"
#include "instruction.h"
#include "optimize/bshift.h"
#include "optimize/constfold.h"

namespace via::Compilation
{

viaRegister compile_lit_expr(Generator *, Parsing::AST::LiteralExprNode);
viaRegister compile_un_expr(Generator *, Parsing::AST::UnaryExprNode);
viaRegister compile_binary_expr(Generator *, Parsing::AST::BinaryExprNode);
viaRegister compile_index_expr(Generator *, Parsing::AST::IndexExprNode);
viaRegister compile_call_expr(Generator *, Parsing::AST::CallExprNode);
viaRegister compile_var_expr(Generator *, Parsing::AST::VarExprNode);
viaRegister compile_expression(Generator *, Parsing::AST::ExprNode);

} // namespace via::Compilation
