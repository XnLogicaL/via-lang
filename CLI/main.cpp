// ===========================================================================================
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0
// ===========================================================================================

#include "linenoise.hpp"
#include "argparse/argparse.hpp"
#include "file-io.h"
#include "via.h"

#define SET_PROFILER_POINT(id)     [[maybe_unused]] const auto id = std::chrono::steady_clock::now();
#define GET_PROFILER_DIFF_MS(l, r) std::chrono::duration<double, std::milli>(r - l).count()

using namespace argparse;
using namespace via;

// REPL messages
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

static ProgramData  dummy_data("<cli>", "");
static ErrorEmitter emitter((dummy_data));

struct CompilationResult {
    bool        failed;
    ProgramData program;

    CompilationResult(bool failed, via::ProgramData program)
        : failed(failed),
          program(std::move(program)) {}
};

std::unique_ptr<ArgumentParser> get_standard_parser(const std::string& name) {
    auto command = std::make_unique<ArgumentParser>(name);
    command->add_argument("target");
    command->add_argument("--dump-ast", "-Da")
        .help("Dumps the abstract syntax tree representation of the program")
        .flag();
    command->add_argument("--dump-bytecode", "-Db")
        .help(
            "Dumps human-readable bytecode to the console upon compilation of the given source file"
        )
        .flag();
    command->add_argument("--dump-machine-code", "-Dmc")
        .help("Dumps raw machine code to the console when compilation of the given source file is "
              "completed")
        .flag();
    command->add_argument("--dump-tokens", "-Dt")
        .help("Dumps tokenized representation of the given source file upon tokenization")
        .flag();
    command->add_argument("--optimize", "-O")
        .help("Sets optimization level to the given integer")
        .scan<'u', uint32_t>()
        .default_value(1);
    command->add_argument("--verbose", "-v").help("Enables verbosity").flag();
    command->add_argument("--sassy").help("Enables sassy mode 😉").flag();
    command->add_argument("--Bcapitalize-opcodes")
        .help("Whether to capitalize opcodes inside bytecode dumps")
        .flag();

    return command;
}

CompilationResult handle_compile(argparse::ArgumentParser& subcommand_parser) {
    using enum OutputSeverity;
    using enum TokenType;
    using namespace utils;

    const auto get_flag = [&subcommand_parser](const std::string& flag) constexpr -> bool {
        return subcommand_parser.get<bool>(flag);
    };

    const auto print_flag_label = [](const std::string& flag) constexpr -> void {
        std::cout << std::format("flag [{}]:\n", flag);
    };

    bool verbosity_flag = get_flag("--verbose");
    bool sassy_flag     = get_flag("--sassy");

    std::string file = subcommand_parser.get<std::string>("target");
    ProgramData program(file, "");

    // Record compilation start time
    SET_PROFILER_POINT(compilation_start)

    if (verbosity_flag) {
        program.flags |= VFLAG_VERBOSE;
    }
    if (sassy_flag) {
        program.flags |= VFLAG_SASSY;
    }

    ReadResult source_result = read_from_file(file);
    if (!source_result.has_value()) {
        emitter.out_flat(source_result.error(), Error);
        return CompilationResult(true, std::move(program));
    }
    program.source = *source_result;

    Tokenizer    lexer(program);
    Preprocessor preprocessor(program);
    Parser       parser(program);
    Compiler     compiler(program);

    SET_PROFILER_POINT(lex_start);
    lexer.tokenize();

    if (verbosity_flag) {
        SET_PROFILER_POINT(lex_end);
        emitter.out_flat(
            std::format(
                "Tokenization finished in {:0.9f}s", GET_PROFILER_DIFF_MS(lex_start, lex_end) / 1000
            ),
            Info
        );
    }

    preprocessor.declare_default();
    SET_PROFILER_POINT(preproc_start);
    bool preproc_failed = preprocessor.preprocess();

    if (verbosity_flag) {
        if (preproc_failed) {
            emitter.out_flat("Preprocessing failed", Error);
            return CompilationResult(true, std::move(program));
        }
        SET_PROFILER_POINT(preproc_end);
        emitter.out_flat(
            std::format(
                "Preprocessing finished in {:0.9f}s",
                GET_PROFILER_DIFF_MS(preproc_start, preproc_end) / 1000
            ),
            Info
        );
    }

    SET_PROFILER_POINT(parser_start);
    bool parser_failed = parser.parse();

    if (verbosity_flag) {
        if (parser_failed) {
            emitter.out_flat("Parsing failed", Error);
            return CompilationResult(true, std::move(program));
        }
        SET_PROFILER_POINT(parser_end);
        emitter.out_flat(
            std::format(
                "Parsing finished in {:0.9f}s",
                GET_PROFILER_DIFF_MS(parser_start, parser_end) / 1000
            ),
            Info
        );
    }

    SET_PROFILER_POINT(codegen_start);
    bool compiler_failed = compiler.generate();

    if (verbosity_flag) {
        if (compiler_failed) {
            emitter.out_flat("Bytecode generation failed", Error);
            return CompilationResult(true, std::move(program));
        }
        SET_PROFILER_POINT(codegen_end);
        emitter.out_flat(
            std::format(
                "Bytecode generation finished in {:0.9f}s",
                GET_PROFILER_DIFF_MS(codegen_start, codegen_end) / 1000
            ),
            Info
        );
    }

    bool failed = preproc_failed || parser_failed || compiler_failed;

    if (!failed) {
        if (get_flag("--dump-tokens")) {
            print_flag_label("--dump-tokens");
            for (const Token& token : program.token_stream->get()) {
                std::cout << token.to_string() << "\n";
            }
        }
        if (get_flag("--dump-ast")) {
            print_flag_label("--dump-ast");
            uint32_t depth = 0;
            for (const pStmtNode& pstmt : program.ast->statements) {
                std::cout << pstmt->to_string(depth) << "\n";
            }
        }
        if (get_flag("--dump-bytecode")) {
            print_flag_label("--dump-bytecode");
            std::cout << "main:\n";
            for (const Bytecode& bytecode : program.bytecode->get()) {
                if (bytecode.instruction.op == OpCode::LABEL) {
                    std::cout << std::format(
                        "{}{}:\n", bytecode.meta_data.comment, bytecode.instruction.operand0
                    );
                    continue;
                }
                else if (bytecode.instruction.op == OpCode::LOADFUNCTION) {
                    std::cout << bytecode.meta_data.comment << ":\n";
                    continue;
                }
                std::cout << "  " << via::to_string(bytecode, get_flag("--Bcapitalize-opcodes"))
                          << "\n";
            }
        }
        if (get_flag("--dump-machine-code")) {
            print_flag_label("--dump-machine-code");
            for (const Bytecode& bytecode : program.bytecode->get()) {
                const Instruction& instruction = bytecode.instruction;
                const size_t       size        = sizeof(Instruction);
                const uint8_t*     data        = reinterpret_cast<const uint8_t*>(&instruction);
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
            emitter.out_flat("Compilation failed", Error);
            return CompilationResult(true, std::move(program));
        }
        SET_PROFILER_POINT(compilation_end);
        double compilation_time = GET_PROFILER_DIFF_MS(compilation_start, compilation_end);
        emitter.out_flat(
            std::format("Compilation finished in {:0.9f}s", compilation_time / 1000), Info
        );
    }
    return CompilationResult(failed, std::move(program));
}

CompilationResult handle_run(argparse::ArgumentParser& subcommand_parser) {
    using namespace via;

    const auto get_flag = [&subcommand_parser](const std::string& flag) constexpr -> bool {
        return subcommand_parser.get<bool>(flag);
    };

    CompilationResult result         = handle_compile(subcommand_parser);
    bool              verbosity_flag = get_flag("--verbose");

    if (!result.failed) {
        SET_PROFILER_POINT(runtime_begin);
        SET_PROFILER_POINT(state_init_begin);

        GState gstate;
        State  state(&gstate, result.program);

        if (verbosity_flag) {
            SET_PROFILER_POINT(state_init_end);
            double state_init_time = GET_PROFILER_DIFF_MS(state_init_begin, state_init_end);
            emitter.out_flat(
                std::format("State initialized in {:0.9f}s", state_init_time / 1000),
                OutputSeverity::Info
            );
        }

        SET_PROFILER_POINT(lib_load_begin);
        lib::open_baselib(&state);

        if (verbosity_flag) {
            SET_PROFILER_POINT(lib_load_end);
            double lib_load_time = GET_PROFILER_DIFF_MS(lib_load_begin, lib_load_end);
            emitter.out_flat(
                std::format("C Libraries loaded in {:0.9f}s", lib_load_time / 1000),
                OutputSeverity::Info
            );
        }

        SET_PROFILER_POINT(execution_begin);
        state.execute();

        if (verbosity_flag) {
            SET_PROFILER_POINT(execution_end);
            double execution_time = GET_PROFILER_DIFF_MS(execution_begin, execution_end);
            emitter.out_flat(
                std::format("Execution completed in {:0.9f}s", execution_time / 1000),
                OutputSeverity::Info
            );
        }

        if (verbosity_flag) {
            SET_PROFILER_POINT(runtime_end);
            double runtime = GET_PROFILER_DIFF_MS(runtime_begin, runtime_end);
            emitter.out_flat(
                std::format("Runtime completed in {:0.9f}s", runtime / 1000), OutputSeverity::Info
            );
        }
    }

    return result;
}

CompilationResult handle_repl(argparse::ArgumentParser&) {
    using namespace via;
    std::string s;
    ProgramData program(s, s);
    return CompilationResult(false, std::move(program));
}

int main(int argc, char* argv[]) {
    try {
        // Argument parser entry point
        ArgumentParser argument_parser("via", VIA_VERSION);

        auto compile_parser = get_standard_parser("compile");
        compile_parser->add_description("Compiles the given source file.");

        auto run_parser = get_standard_parser("run");
        run_parser->add_description("Compiles and runs the given source file.");

        // Add subparsers
        argument_parser.add_subparser(*compile_parser);
        argument_parser.add_subparser(*run_parser);
        argument_parser.parse_args(argc, argv);

        if (argument_parser.is_subcommand_used(*compile_parser)) {
            handle_compile(*compile_parser);
        }
        else if (argument_parser.is_subcommand_used(*run_parser)) {
            handle_run(*run_parser);
        }
    }
    catch (const std::exception& e) {
        emitter.out_flat(e.what(), OutputSeverity::Error);
    }
    return 0;
}
