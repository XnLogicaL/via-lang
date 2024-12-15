/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "compiler.h"
#include "vexpression.h"
#include "vstatement.h"

namespace via::Compilation
{

using namespace Parsing;
using namespace AST;

std::vector<viaInstruction> Compiler::compile()
{
    for (StmtNode stmt : m_ast->statements)
        compile_statement(m_gen, stmt);

    return m_gen->get();
}

} // namespace via::Compilation