/* This file is part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "cmdparser.h"
#include "interpreter.h"
#include "linenoise.hpp"
#include "fileio.h"
#include "via.h"

using namespace via;

const char USAGE[] = "Invalid command\n Usage: via <subcommand> <arguments>\n";
const char REPL_WELCOME[] = "via-lang Copyright (C) 2024 XnLogicaL @ www.github.com/XnLogicaL/via-lang\n"
                            "Use ';help' to see a list of commands.\n"
                            "WARNING: repl has not been fully implemented.\n";
const char REPL_HELP[] = "repl commands:\n"
                         "  ;quit - Quits repl\n"
                         "  ;help - Prints this \"menu\"\n"
                         "  ;exitinfo - Displays the last exit info returned by the VM\n";
const char REPL_HEAD[] = ">> ";

void handle_compile(const std::vector<std::string> &args)
{
    if (args.empty()) {
        std::cerr << "Invalid command\nNo input file provided.\n";
        std::exit(1);
    }

    std::string input = args.at(0);
    std::string input_code = via::utils::read_from_file(input);

    ProgramData program(input, input_code);

    Tokenizer lexer(program);
    lexer.tokenize();

    Preprocessor preprocessor(program);
    bool failed = preprocessor.preprocess();

    if (failed)
        return;

    Parser parser(program);
    parser.parse_program();

    Compiler compiler(program);
    compiler.add_default_passes();
    compiler.generate();

    for (Instruction instr : program.bytecode->get())
        std::cout << via::to_string(program, instr) << "\n";
}

void handle_run(const std::vector<std::string> &args)
{
    if (args.empty()) {
        std::cerr << "Invalid command\nNo input file provided.\n";
        std::exit(1);
    }

    std::string file_path = args.at(0);
    std::string source_code = via::utils::read_from_file(file_path);

    ProgramData program(file_path, source_code);

    Interpreter interpreter(program);
    interpreter.execute(program);
}

void handle_repl(const std::vector<std::string> &)
{
    std::cout << REPL_WELCOME;

    while (true) {
        std::string code;
        bool quit = linenoise::Readline(REPL_HEAD, code);

        if (quit)
            break;

        if (code.starts_with(';')) {
            std::string command = code.substr(1);
            if (command == "quit" || command == "q")
                break;
            if (command == "help" || command == "h")
                std::cout << REPL_HELP;
            else if (command == "exitinfo" || command == "ei") {
                if (false)
                    std::cout << "<none>\n";
                else
                    std::cout << "Exit code: \n"
                              << "At instruction: \n";
            }
            else
                std::cerr << "Unknown command: " << command << "\n";

            continue;
        }
    }
}

int main(int argc, char **argv)
{
    CmdParser parser(argc, argv);

    if (!parser.is_valid()) {
        std::cerr << USAGE;
        return 1;
    }

    const auto &args = parser.get_arguments();
    const auto &subcom = parser.get_subcommand();

    linenoise::SetCompletionCallback([](const char *editBuffer, std::vector<std::string> &completions) {
        if (editBuffer[0] == ';') {
            completions.push_back(";quit");
            completions.push_back(";help");
            completions.push_back(";exitinfo");
        }
    });

    if (subcom == "compile")
        handle_compile(args);
    else if (subcom == "run")
        handle_run(args);
    else if (subcom == "repl")
        handle_repl(args);
    else {
        std::cerr << "Invalid subcommand\n";
        return 1;
    }

    return 0;
}
