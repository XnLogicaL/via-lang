// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "linenoise.hpp"
#include "argparse.hpp"
#include "fileio.h"
#include "via.h"

[[maybe_unused]]
static constexpr const char REPL_WELCOME[] =
    "via-lang Copyright (C) 2024-2025 XnLogicaL @ www.github.com/XnLogicaL/via-lang\n"
    "Use ';help' to see a list of commands.\n";

[[maybe_unused]]
static constexpr const char REPL_HELP[] =
    "repl commands:\n"
    "  ;quit - Quits repl\n"
    "  ;help - Prints this \"menu\"\n"
    "  ;exitinfo - Displays the last exit info returned by the VM\n";

[[maybe_unused]]
static constexpr const char REPL_HEAD[] = ">> ";

via::ProgramData handle_compile(argparse::ArgumentParser& subcommand_parser) {
    using namespace via;

    using enum via::OutputSeverity;
    using enum via::TokenType;

    const auto get_flag = [&subcommand_parser](const std::string& flag) constexpr -> bool {
        return subcommand_parser.get<bool>(flag);
    };

    const auto print_flag_label = [](const std::string& flag) constexpr -> void {
        std::cout << std::format("flag [{}]:\n", flag);
    };

    const bool verbosity_flag = get_flag("--verbose");

    std::string file = subcommand_parser.get<std::string>("target");
    std::string source;
    const auto  compilation_start = std::chrono::steady_clock::now();

    ProgramData program(file, source);
    Emitter     local_emitter(program);

    try {
        source = utils::read_from_file(file);
    }
    catch (const std::exception& e) {
        local_emitter.out_flat(std::format("Failed to read file '{}': {}", file, e.what()), Error);
        return program;
    }

    program.source = source;

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

    if (!failed) {
        if (get_flag("--dump-tokens")) {
            print_flag_label("--dump-tokens");

            for (const Token& token : program.tokens->tokens) {
                std::cout << token.to_string() << "\n";
            }
        }

        if (get_flag("--dump-ast")) {
            print_flag_label("--dump-ast");

            U32 depth = 0;

            for (const pStmtNode& pstmt : program.ast->statements) {
                std::cout << pstmt->to_string(depth) << "\n";
            }
        }

        if (get_flag("--dump-bytecode")) {
            print_flag_label("--dump-bytecode");

            U32 counter = 0;

            for (const Bytecode& bytecode : program.bytecode->get()) {
                std::cout << std::format("{:0>3} ", counter++)
                          << via::to_string(bytecode, get_flag("--Bcapitalize-opcodes")) << "\n";
            }
        }

        if (get_flag("--dump-machine-code")) {
            print_flag_label("--dump-machine-code");

            for (const Bytecode& bytecode : program.bytecode->get()) {
                const Instruction& instruction = bytecode.instruction;

                const size_t   size = sizeof(Instruction);
                const uint8_t* data = reinterpret_cast<const U8*>(&instruction);

                for (size_t i = 0; i < size; i++) {
                    std::cout << "0x" << std::setw(2) << std::setfill('0') << std::hex
                              << static_cast<int>(data[i]) << std::dec << " ";
                }

                std::cout << std::endl;
            }
        }
    }

    if (verbosity_flag) {
        if (failed) {
            local_emitter.out_flat("Compilation failed", Error);
            return program;
        }

        auto   compilation_end = std::chrono::steady_clock::now();
        double compilation_time =
            std::chrono::duration<double, std::milli>(compilation_end - compilation_start).count();

        local_emitter.out_flat(
            std::format("Compilation finished in {}s", compilation_time / 1000), Info);
    }

    return program;
}

via::ProgramData handle_repl(argparse::ArgumentParser&) {
    using namespace via;

    using enum via::OutputSeverity;
    using enum via::TokenType;

    std::string s;

    ProgramData program(s, s);

    return program;
}

int main(int argc, char* argv[]) {
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
        .flag();

    compile_command.add_argument("--dump-bytecode", "-Db")
        .help("Dumps human-readable bytecode to the console upon compilation of the given source "
              "file is completed")
        .flag();

    compile_command.add_argument("--dump-machine-code", "-Dmc")
        .help("Dumps raw machine code to the console when compilation of the given source file is "
              "completed")
        .flag();

    compile_command.add_argument("--dump-tokens", "-Dt")
        .help("Dumps tokenized representation of the given source file upon tokenization of the "
              "given source file "
              "is completed")
        .flag();

    compile_command.add_argument("--optimize", "-O")
        .help("Sets optimization level to the given integer")
        .scan<'u', U32>()
        .default_value(1);

    compile_command.add_argument("--verbose", "-v").help("Enables verbosity").flag();

    compile_command.add_argument("--Bcapitalize-opcodes")
        .help("Whether to captialize opcodes inside bytecode dumps")
        .flag();

    // Argument parser initialization
    argument_parser.add_subparser(compile_command);

    try {
        argument_parser.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << argument_parser;
        return 1;
    }

    if (argument_parser.is_subcommand_used(compile_command)) {
        handle_compile(compile_command);
    }
}
