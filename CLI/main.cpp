/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "common.h"
#include "cmdparser.hpp"
#include "fileio.hpp"
#include "repl.h"
#include "via.h"

using namespace viaCLI;
using namespace via;

int main(int argc, char **argv)
{
    static unsigned char USAGE[] = "Invalid command\n Usage: via <subcommand> <arguments>\n";

    CmdParser parser(argc, argv);

    if (!parser.is_valid())
    {
        std::cout << USAGE;
        std::abort();
    }

    if (parser.get_subcommand() == "--test-installation")
        std::cout << "Success\n";
    else if (parser.get_subcommand() == "compile")
    {
        auto args = parser.get_arguments();
        std::string input = args.at(0);
        std::string input_code = utils::read_from_file(input);

        Tokenization::Tokenizer lexer(input_code);
        viaSourceContainer container = lexer.tokenize();

        Parsing::Parser parser(container);
        Parsing::AST::AST *ast = parser.parse_program();

        Compilation::Compiler compiler(ast);
        compiler.add_default_passes();
        compiler.generate();

        std::string buffer;

        for (viaInstruction instr : compiler.get())
            buffer += Compilation::viaC_compileinstruction(instr) + "\n";

        std::cout << buffer;
    }
    else if (parser.get_subcommand() == "run")
    {
        auto args = parser.get_arguments();
        Interpreter interpreter;

        std::string file_path = args.at(0);
        std::string source_code = utils::read_from_file(file_path);

        interpreter.run(source_code);
    }
    else if (parser.get_subcommand() == "repl")
    {
        std::cout << "via-lang Copyright (C) 2024 @ github.com/XnLogical/via-lang, XnLogicaL\n";
        REPL repl;
        std::string code;

        while (true)
        {
            std::cout << ">> ";
            std::getline(std::cin, code);

            if (code == ";q")
                break;
            if (code == ";c")
                continue;

            repl.execute(code);
        }
    }
    else
    {
        std::cout << "Invalid subcommand\n";
        return 1;
    }

    return 0;
}
