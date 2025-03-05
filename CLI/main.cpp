// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "linenoise.hpp"
#include "argparse.hpp"
#include "fileio.h"
#include "via.h"

static constexpr const char REPL_WELCOME[] =
    "via-lang Copyright (C) 2024-2025 XnLogicaL @ www.github.com/XnLogicaL/via-lang\n"
    "Use ';help' to see a list of commands.\n";
static constexpr const char REPL_HELP[] =
    "repl commands:\n"
    "  ;quit - Quits repl\n"
    "  ;help - Prints this \"menu\"\n"
    "  ;exitinfo - Displays the last exit info returned by the VM\n";
static constexpr const char REPL_HEAD[] = ">> ";

via::ProgramData handle_compile(argparse::ArgumentParser &subcommand_parser)
{
    using namespace via;

    using enum via::OutputSeverity;
    using enum via::TokenType;

    const auto get_flag = [&subcommand_parser](const std::string &flag) constexpr -> bool {
        return subcommand_parser.get<bool>(flag);
    };

    const auto print_flag_label = [](const std::string &flag) constexpr -> void {
        std::cout << std::format("flag [{}]:\n", flag);
    };

    const bool verbosity_flag = get_flag("--verbose");

    std::string file              = subcommand_parser.get<std::string>("target");
    std::string source            = utils::read_from_file(file);
    const auto  compilation_start = std::chrono::steady_clock::now();

    ProgramData  program(file, source);
    Emitter      local_emitter(program);
    Tokenizer    lexer(program);
    Preprocessor preprocessor(program);
    Parser       parser(program);
    Compiler     compiler(program);

    if (verbosity_flag) {
        local_emitter.out_flat("Compilation started", Info);
    }

    lexer.tokenize();
    preprocessor.declare_default();

    bool failed = preprocessor.preprocess() || parser.parse() || compiler.generate();
    if (verbosity_flag) {
        if (failed) {
            local_emitter.out_flat("Compilation failed", Error);
            return program;
        }

        auto   compilation_end = std::chrono::steady_clock::now();
        double compilation_time =
            std::chrono::duration<double, std::milli>(compilation_end - compilation_start).count();

        local_emitter.out_flat(std::format("Compilation finished in {}s", compilation_time), Info);
    }

    if (get_flag("--dump-tokens")) {
        print_flag_label("--dump-tokens");

        for (const Token &token : program.tokens->tokens) {
            std::cout << token.to_string() << "\n";
        }
    }

    if (get_flag("--dump-ast")) {
        print_flag_label("--dump-ast");

        U32 depth = 0;

        for (const pStmtNode &pstmt : program.ast->statements) {
            std::cout << pstmt->to_string(depth) << "\n";
        }
    }

    if (get_flag("--dump-bytecode")) {
        print_flag_label("--dump-bytecode");

        for (const Bytecode &bytecode : program.bytecode->get()) {
            std::cout << via::to_string(bytecode) << "\n";
        }
    }

    return program;
}

via::ProgramData handle_repl(argparse::ArgumentParser &) {}

int main(int argc, char *argv[])
{
    using namespace via;
    using namespace argparse;

    // Argument parser entry point
    ArgumentParser argument_parser("via", VIA_VERSION);

    // Compile subcommand
    ArgumentParser compile_command("compile");
    compile_command.add_description("Compiles the given source file");
    compile_command.add_argument("target");
    compile_command.add_argument("--dump-ast", "-Da")
        .help("Dumps the abstract syntax tree representation of the program")
        .scan<'b', bool>()
        .default_value(false)
        .implicit_value(true);

    compile_command.add_argument("--dump-bytecode", "-Db")
        .help("Dumps human-readable bytecode to the console upon compilation of the give source "
              "file is completed")
        .scan<'b', bool>()
        .default_value(false)
        .implicit_value(true);

    compile_command.add_argument("--dump-tokens", "-Dt")
        .help("Dumps tokenized representation of the given source file upon tokenization of the "
              "given source file "
              "is completed")
        .scan<'b', bool>()
        .default_value(false)
        .implicit_value(true);

    compile_command.add_argument("--optimize", "-O")
        .help("Sets optimization level to the given integer")
        .scan<'u', U32>()
        .default_value(1);

    compile_command.add_argument("--verbose", "-v")
        .help("Enables verbosity")
        .scan<'b', bool>()
        .default_value(false)
        .implicit_value(true);

    // Argument parser initialization
    argument_parser.add_subparser(compile_command);

    try {
        argument_parser.parse_args(argc, argv);
    }
    catch (const std::exception &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << argument_parser;
        return 1;
    }

    if (argument_parser.is_subcommand_used(compile_command)) {
        handle_compile(compile_command);
    }
}
