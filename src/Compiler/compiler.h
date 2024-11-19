/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "common.h"
#include "instruction.h"

#include "Parser/ast.h"

namespace via::Compilation
{

class Compiler
{
public:
    Compiler(Parsing::AST::AST *ast)
        : ast(ast)
    {
    }
    std::vector<Instruction> compile();

private:
    Parsing::AST::AST *ast;
};

} // namespace via::Compilation