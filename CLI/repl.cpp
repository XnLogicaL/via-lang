/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "repl.h"

namespace viaCLI
{

std::vector<via::Instruction> REPLEngine::compile(std::string source)
{
    via::Tokenizer lexer(source);
    via::ProgramData program = lexer.tokenize();
    program.file_name = "<repl>";

    /*Tokenization::SyntaxAnalyzer analyzer(program);
    bool fail = analyzer.analyze();

    VIA_ASSERT_SILENT(!fail, "Syntax analysis failed");*/

    via::Parser parser(program);
    via::AbstractSyntaxTree *ast = parser.parse_program();

    via::Compiler compiler(ast);
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
        for (via::Instruction instr : bytecode)
            std::cout << via::ccompileinstruction(instr) << "\n";
    }

    if (!V)
        V = stnewstate(bytecode);
    else
    {
        V->ihp = new via::Instruction[bytecode.size()];
        V->ibp = V->ihp + bytecode.size();
        V->ip = V->ihp;

        std::copy(bytecode.begin(), bytecode.end(), V->ip);
    }

    if (!libs_loaded)
    {
        libs_loaded = true;
        via::lib::loadbaselib(V);
        via::lib::loadmathlib(V);
    }

    if (print)
        std::cout << "Program output:\n";

    via::execute(V);
    via::pausethread(V);

    delete[] V->ihp;

    VIA_ASSERT_SILENT(V->exitc == 0, V->exitm);
}

} // namespace viaCLI