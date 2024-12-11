/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "common.h"
#include "Parser/ast.h"

namespace via::Compilation
{

void optimize_inline_const(Parsing::AST::AST *, std::string, Parsing::AST::ExprNode);
void optimize_inline_func(Parsing::AST::AST *, std::string, Parsing::AST::FunctionDeclStmtNode);

} // namespace via::Compilation