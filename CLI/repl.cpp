/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "repl.h"

namespace viaCLI
{

using namespace via;

std::vector<viaInstruction> REPL::compile(std::string source)
{
    Tokenization::Tokenizer lexer(source);
    viaSourceContainer container = lexer.tokenize();

    Parsing::Parser parser(container);
    Parsing::AST::AST *ast = parser.parse_program();

    Compilation::Compiler compiler(ast);
    compiler.add_default_passes();
    compiler.generate();

    return compiler.get();
}

void REPL::execute(std::string code)
{
    auto bytecode = compile(code);

    if (!V)
        V = viaA_newstate(bytecode);
    else
    {
        V->ihp = new viaInstruction[bytecode.size()];
        V->ibp = V->ihp + bytecode.size();
        V->ip = V->ihp;

        std::copy(bytecode.begin(), bytecode.end(), V->ip);
    }

    lib::viaL_loadbaselib(V);
    lib::viaL_loadmathlib(V);

    via_execute(V);

    delete[] V->ihp;
}

} // namespace viaCLI