/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "repl.h"

namespace viaCLI
{

using namespace via;

std::vector<Instruction> REPLEngine::compile(std::string source)
{
    Tokenization::Tokenizer lexer(source);
    viaSourceContainer container = lexer.tokenize();
    container.file_name = "<repl>";

    /*Tokenization::SyntaxAnalyzer analyzer(container);
    bool fail = analyzer.analyze();

    VIA_ASSERT_SILENT(!fail, "Syntax analysis failed");*/

    Parsing::Parser parser(container);
    Parsing::AST::AST *ast = parser.parse_program();

    Compilation::Compiler compiler(ast);
    compiler.add_default_passes();
    compiler.generate();

    return compiler.get();
}

void REPLEngine::execute(std::string code, bool print)
{
    static bool libs_loaded = false;
    auto bytecode = compile(code);

    if (bytecode.empty())
        return;

    if (print)
    {
        std::cout << "Program bytecode:\n";
        for (Instruction instr : bytecode)
            std::cout << Compilation::viaC_compileinstruction(instr) << "\n";
    }

    if (!V)
        V = viaA_newstate(bytecode);
    else
    {
        V->ihp = new Instruction[bytecode.size()];
        V->ibp = V->ihp + bytecode.size();
        V->ip = V->ihp;

        std::copy(bytecode.begin(), bytecode.end(), V->ip);
    }

    if (!libs_loaded)
    {
        libs_loaded = true;
        lib::viaL_loadbaselib(V);
        lib::viaL_loadmathlib(V);
    }

    if (print)
        std::cout << "Program output:\n";

    via_execute(V);
    via_pausethread(V);

    delete[] V->ihp;

    VIA_ASSERT_SILENT(V->exitc == 0, V->exitm);
}

} // namespace viaCLI