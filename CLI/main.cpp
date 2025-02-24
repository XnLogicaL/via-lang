// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "cmdparser.h"
#include "interpreter.h"
#include "linenoise.hpp"
#include "fileio.h"
#include "via.h"

using namespace via;

static constexpr const char USAGE[] = "Invalid command\n Usage: via <subcommand> <arguments>\n";
static constexpr const char REPL_WELCOME[] =
    "via-lang Copyright (C) 2024 XnLogicaL @ www.github.com/XnLogicaL/via-lang\n"
    "Use ';help' to see a list of commands.\n"
    "WARNING: repl has not been fully implemented.\n";
static constexpr const char REPL_HELP[] =
    "repl commands:\n"
    "  ;quit - Quits repl\n"
    "  ;help - Prints this \"menu\"\n"
    "  ;exitinfo - Displays the last exit info returned by the VM\n";
static constexpr const char REPL_HEAD[] = ">> ";

static Emitter cli_emitter(nullptr);

void handle_compile(const std::vector<std::string> &args)
{
#define HAS_FLAG(flag) \
    ({ \
        auto it = std::ranges::find(args, flag); \
        it != args.end(); \
    })

#define CHECK_SUBPROC_FAIL \
    if (failed) { \
        if (flag_verbose) { \
            cli_emitter.out_flat("Compilation failed", OutputSeverity::Error); \
        } \
        return; \
    }

    if (args.empty()) {
        cli_emitter.out_flat("no input found", OutputSeverity::Error);
        std::exit(1);
    }

    bool failed;

    bool flag_verbose = HAS_FLAG("--verbose") || HAS_FLAG("-v");
    bool flag_cache = HAS_FLAG("--cache") || HAS_FLAG("-c");
    bool flag_dump_tokens = HAS_FLAG("--dump-tokens");
    bool flag_dump_ast = HAS_FLAG("--dump-ast");
    bool flag_dump_bytecode = HAS_FLAG("--dump-bytecode");

    std::string input = args.at(0);
    std::string input_code = via::utils::read_from_file(input);

    ProgramData program(input, input_code);

    Tokenizer lexer(&program);
    lexer.tokenize();

    if (flag_dump_tokens) {
        std::cout << "\nflag [--dump-tokens]:\n";
        for (const Token &tok : program.tokens->tokens) {
            std::cout << tok.to_string() << "\n";
        }
    }

    Preprocessor preprocessor(&program);
    failed = preprocessor.preprocess();

    CHECK_SUBPROC_FAIL;

    Parser parser(&program);
    failed = parser.parse_program();

    CHECK_SUBPROC_FAIL;

    if (flag_dump_ast) {
        U32 depth = 0;

        std::cout << "\nflag [--dump-ast]:\n";
        for (const pStmtNode &pstmt : program.ast->statements) {
            std::cout << pstmt->to_string(depth) << "\n";
        }
    }

    Compiler compiler(&program);
    failed = compiler.generate();

    CHECK_SUBPROC_FAIL;

    if (flag_dump_bytecode) {
        std::cout << "\nflag [--dump-bytecode]:\n";
        for (Instruction instr : program.bytecode->get()) {
            std::cout << via::to_string(&program, instr) << "\n";
        }
    }

    if (flag_cache) {
        CacheFile file(&program);
        CacheManager manager;
        manager.write_cache("./", file);
    }

    if (flag_verbose) {
        cli_emitter.out_flat("Compilation completed", OutputSeverity::Info);
    }

#undef HAS_FLAG
#undef CHECK_SUBPROC_FAIL
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

    Interpreter interpreter(&program);
    interpreter.execute(&program);
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

    linenoise::SetCompletionCallback([](const char *editBuffer,
                                        std::vector<std::string> &completions) {
        if (editBuffer[0] == ';') {
            completions.push_back(";quit");
            completions.push_back(";help");
            completions.push_back(";exitinfo");
        }
    });

    if (subcom == "compile") {
        handle_compile(args);
    }
    else if (subcom == "run") {
        handle_run(args);
    }
    else if (subcom == "repl") {
        handle_repl(args);
    }
    else {
        std::cerr << "Invalid subcommand\n";
        return 1;
    }

    return 0;
}
