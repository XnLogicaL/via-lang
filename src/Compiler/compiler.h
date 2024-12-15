/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "common.h"
#include "gen.h"
#include "instruction.h"
#include "Parser/ast.h"

namespace via::Compilation
{

class Compiler
{
public:
    Compiler(Parsing::AST::AST *ast)
        : m_ast(ast)
        , m_gen(new Generator())
    {
    }
    std::vector<viaInstruction> compile();

private:
    Parsing::AST::AST *m_ast;
    Generator *m_gen;
};

} // namespace via::Compilation