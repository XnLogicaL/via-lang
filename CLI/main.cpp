/* This file is part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "common.h"
#include "cmdparser.hpp"
#include "linenoise.hpp"
#include "fileio.hpp"
#include "repl.h"
#include "via.h"

using namespace viaCLI;
using namespace via;

namespace
{

const char USAGE[] = "Invalid command\n Usage: via <subcommand> <arguments>\n";
const char REPL_WELCOME[] = "via-lang Copyright (C) 2024 @ github.com/XnLogicaL/via-lang, XnLogicaL\n"
                            "Use ';help' to see a list of commands.\n";
const char REPL_HELP[] = "repl commands:\n"
                         "  ;quit - Quits repl\n"
                         "  ;help - Prints this \"menu\"\n"
                         "  ;exitinfo - Displays the last exit info returned by the VM\n";

void handle_test_installation()
{
    std::cout << "Success\n";
}

#ifdef VIA_DEBUG
void handle_debug_enabled()
{
    std::cout << "true\n";
}
#endif

void handle_compile(const std::vector<std::string> &args)
{
    if (args.empty())
    {
        std::cerr << "Invalid command\nNo input file provided.\n";
        exit(1);
    }

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
    for (Instruction &instr : compiler.get())
        buffer += Compilation::viaC_compileinstruction(instr) + "\n";

    std::cout << buffer;
}

void handle_run(const std::vector<std::string> &args)
{
    if (args.empty())
    {
        std::cerr << "Invalid command\nNo input file provided.\n";
        exit(1);
    }

    std::string file_path = args.at(0);
    std::string source_code = utils::read_from_file(file_path);

    Interpreter interpreter;
    interpreter.run(source_code);
}

void handle_repl(const std::vector<std::string> &args)
{
    std::cout << REPL_WELCOME;

    REPLEngine repl;
    bool print_bytecode = std::any_of(
        args.begin(),
        args.end(),
        [](const std::string &arg)
        {
            return arg == "-bc" || arg == "--bytecode";
        }
    );

    while (true)
    {
        std::string code;
        bool quit = linenoise::Readline(">> ", code);

        if (quit)
            break;

        if (code.starts_with(';'))
        {
            std::string command = code.substr(1);
            if (command == "quit" || command == "q")
                break;
            if (command == "help" || command == "h")
                std::cout << REPL_HELP;
            else if (command == "exitinfo" || command == "ei")
            {
                if (!repl.V)
                    std::cout << "<none>\n";
                else
                    std::cout << "Exit code:    " << repl.V->exitc << "\n"
                              << "Exit message: '" << repl.V->exitm << "'\n";
            }
            else
                std::cerr << "Unknown command: " << command << "\n";

            continue;
        }

        try
        {
            repl.execute(code, print_bytecode);
        }
        catch (const std::exception &e)
        {
            std::cout << "Error during execution: " << e.what() << "\n";
        }
    }
}

} // namespace

int main(int argc, char **argv)
{
    CmdParser parser(argc, argv);

    if (!parser.is_valid())
    {
        std::cerr << USAGE;
        return 1;
    }

    const auto &args = parser.get_arguments();
    const auto &subcom = parser.get_subcommand();

    linenoise::SetCompletionCallback(
        [](const char *editBuffer, std::vector<std::string> &completions)
        {
            if (editBuffer[0] == ';')
            {
                completions.push_back(";quit");
                completions.push_back(";help");
                completions.push_back(";exitinfo");
            }
        }
    );

    if (subcom == "--test-installation")
        handle_test_installation();
    else if (subcom == "compile")
        handle_compile(args);
    else if (subcom == "run")
        handle_run(args);
    else if (subcom == "repl")
        handle_repl(args);
    else
    {
        std::cerr << "Invalid subcommand\n";
        return 1;
    }

    return 0;
}