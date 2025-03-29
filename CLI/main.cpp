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

struct comp_result {
  bool failed;
  trans_unit_context unit;

  comp_result(bool failed, trans_unit_context unit)
    : failed(failed),
      unit(std::move(unit)) {}
};

compiler_context ctx;
error_bus err_bus;
trans_unit_context dummy_unit_ctx("<unavailable>", "");

std::unique_ptr<ArgumentParser> get_standard_parser(const std::string& name) {
  auto command = std::make_unique<ArgumentParser>(name);
  command->add_argument("target");
  command->add_argument("--dump-ast", "-Da")
    .help("Dumps the abstract syntax tree representation of the program")
    .flag();
  command->add_argument("--dump-bytecode", "-Db")
    .help("Dumps human-readable bytecode to the console upon compilation of the given source file")
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

comp_result handle_compile(argparse::ArgumentParser& subcommand_parser) {
  using enum token_type;
  using enum comp_err_lvl;
  using namespace utils;

  const auto get_flag = [&subcommand_parser](const std::string& flag) constexpr -> bool {
    return subcommand_parser.get<bool>(flag);
  };

  const auto print_flag_label = [](const std::string& flag) constexpr -> void {
    std::cout << std::format("flag [{}]:\n", flag);
  };

  bool verbosity_flag = get_flag("--verbose");
  bool sassy_flag = get_flag("--sassy");

  std::string file = subcommand_parser.get<std::string>("target");

  rd_result_t source_result = read_from_file(file);
  trans_unit_context unit_ctx(file, *source_result);

  // Record compilation start time
  SET_PROFILER_POINT(compilation_start)

  if (verbosity_flag) {
    ctx.flags |= vflag_verbose;
  }
  if (sassy_flag) {
    ctx.flags |= vflag_sassy;
  }

  if (!source_result.has_value()) {
    err_bus.log({true, source_result.error(), dummy_unit_ctx, ERROR_, {}});
    return {true, std::move(dummy_unit_ctx)};
  }

  lexer lexer(unit_ctx);
  preprocessor preprocessor(unit_ctx);
  parser parser(unit_ctx);
  compiler compiler(unit_ctx);

  SET_PROFILER_POINT(lex_start);
  lexer.tokenize();

  if (verbosity_flag) {
    SET_PROFILER_POINT(lex_end);

    std::string message = std::format(
      "Tokenization completed in {:0.9f}s", GET_PROFILER_DIFF_MS(lex_start, lex_end) / 1000
    );

    err_bus.log({true, message, unit_ctx, INFO, {}});
  }

  preprocessor.declare_default();

  SET_PROFILER_POINT(preproc_start);

  bool preproc_failed = preprocessor.preprocess();

  if (verbosity_flag) {
    if (preproc_failed) {
      err_bus.log({true, "Preprocess failed", unit_ctx, ERROR_, {}});
      return {true, std::move(unit_ctx)};
    }

    SET_PROFILER_POINT(preproc_end);

    std::string message = std::format(
      "Preprocessing completed in {:0.9f}s", GET_PROFILER_DIFF_MS(preproc_start, preproc_end) / 1000
    );

    err_bus.log({true, message, unit_ctx, INFO, {}});
  }

  SET_PROFILER_POINT(parser_start);
  bool parser_failed = parser.parse();

  if (verbosity_flag) {
    if (parser_failed) {
      err_bus.log({true, "Parsing failed", unit_ctx, ERROR_, {}});
      return {true, std::move(unit_ctx)};
    }

    SET_PROFILER_POINT(parser_end);

    std::string message = std::format(
      "Parsing completed in {:0.9f}s", GET_PROFILER_DIFF_MS(parser_start, parser_end) / 1000
    );

    err_bus.log({true, message, unit_ctx, INFO, {}});
  }

  SET_PROFILER_POINT(codegen_start);
  bool compiler_failed = compiler.generate();

  if (verbosity_flag) {
    if (compiler_failed) {
      err_bus.log({true, "Bytecode generation failed", unit_ctx, ERROR_, {}});
      return {true, std::move(unit_ctx)};
    }

    SET_PROFILER_POINT(codegen_end);

    std::string message = std::format(
      "Bytecode generation completed in {:0.9f}s",
      GET_PROFILER_DIFF_MS(codegen_end, codegen_end) / 1000
    );

    err_bus.log({true, message, unit_ctx, INFO, {}});
  }

  bool failed = preproc_failed || parser_failed || compiler_failed;

  if (!failed) {
    if (get_flag("--dump-tokens")) {
      print_flag_label("--dump-tokens");

      for (const token& token : unit_ctx.tokens->get()) {
        std::cout << token.to_string() << "\n";
      }
    }

    if (get_flag("--dump-ast")) {
      print_flag_label("--dump-ast");
      uint32_t depth = 0;

      for (const p_stmt_node_t& pstmt : unit_ctx.ast->statements) {
        std::cout << pstmt->to_string(depth) << "\n";
      }
    }

    if (get_flag("--dump-bytecode")) {
      print_flag_label("--dump-bytecode");
      std::cout << "main:\n";

      for (const bytecode& bytecode : unit_ctx.bytecode->get()) {
        if (bytecode.instruction.op == opcode::LABEL) {
          std::cout << std::format(
            "{}{}:\n", bytecode.meta_data.comment, bytecode.instruction.operand0
          );
          continue;
        }
        else if (bytecode.instruction.op == opcode::LOADFUNCTION) {
          std::cout << bytecode.meta_data.comment << ":\n";
          continue;
        }

        std::cout << "  " << via::to_string(bytecode, get_flag("--Bcapitalize-opcodes")) << "\n";
      }
    }

    if (get_flag("--dump-machine-code")) {
      print_flag_label("--dump-machine-code");

      for (const bytecode& bytecode : unit_ctx.bytecode->get()) {
        const instruction& instr = bytecode.instruction;
        const size_t size = sizeof(instruction);
        const uint8_t* data = reinterpret_cast<const uint8_t*>(&instr);

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
      err_bus.log({true, "Compilation failed", unit_ctx, ERROR_, {}});
      return {true, std::move(unit_ctx)};
    }

    SET_PROFILER_POINT(compilation_end);

    std::string message = std::format(
      "Compilation finished in {:0.9f}s",
      GET_PROFILER_DIFF_MS(compilation_start, compilation_end) / 1000
    );

    err_bus.log({true, message, unit_ctx, INFO, {}});
  }

  return {failed, std::move(unit_ctx)};
}

comp_result handle_run(argparse::ArgumentParser& subcommand_parser) {
  using namespace via;
  using enum comp_err_lvl;

  const auto get_flag = [&subcommand_parser](const std::string& flag) constexpr -> bool {
    return subcommand_parser.get<bool>(flag);
  };

  comp_result result = handle_compile(subcommand_parser);
  trans_unit_context& unit_ctx = result.unit;

  bool verbosity_flag = get_flag("--verbose");

  if (!result.failed) {
    SET_PROFILER_POINT(runtime_begin);
    SET_PROFILER_POINT(state_init_begin);

    GState gstate;
    State state(&gstate, result.unit);

    if (verbosity_flag) {
      SET_PROFILER_POINT(state_init_end);

      std::string message = std::format(
        "State initialized in {:0.9f}s",
        GET_PROFILER_DIFF_MS(state_init_begin, state_init_end) / 1000
      );

      err_bus.log({true, message, unit_ctx, INFO, {}});
    }

    SET_PROFILER_POINT(lib_load_begin);
    lib::open_baselib(&state);

    if (verbosity_flag) {
      SET_PROFILER_POINT(lib_load_end);

      std::string message = std::format(
        "C libraries loaded in {:0.9f}s", GET_PROFILER_DIFF_MS(lib_load_begin, lib_load_end) / 1000
      );

      err_bus.log({true, message, unit_ctx, INFO, {}});
    }

    SET_PROFILER_POINT(execution_begin);
    state.execute();

    if (verbosity_flag) {
      SET_PROFILER_POINT(execution_end);

      std::string message = std::format(
        "Execution completed in {:0.9f}s",
        GET_PROFILER_DIFF_MS(execution_begin, execution_end) / 1000
      );

      err_bus.log({true, message, unit_ctx, INFO, {}});
    }

    if (verbosity_flag) {
      SET_PROFILER_POINT(runtime_end);

      std::string message = std::format(
        "Runtime completed in {:0.9f}s", GET_PROFILER_DIFF_MS(runtime_begin, runtime_end) / 1000
      );

      err_bus.log({true, message, unit_ctx, INFO, {}});
    }
  }

  return result;
}

comp_result handle_repl(argparse::ArgumentParser&) {
  // REPL messages
  [[maybe_unused]]
  constexpr const char REPL_WELCOME[] =
    "via-lang Copyright (C) 2024-2025 XnLogicaL @ www.github.com/XnLogicaL/via-lang\n"
    "Use ';help' to see a list of commands.\n";

  [[maybe_unused]]
  constexpr const char REPL_HELP[] =
    "repl commands:\n"
    "  ;quit - Quits repl\n"
    "  ;help - Prints this \"menu\"\n"
    "  ;exitinfo - Displays the last exit info returned by the VM\n";

  [[maybe_unused]]
  constexpr const char REPL_HEAD[] = ">> ";

  using namespace via;

  trans_unit_context unit_ctx("<repl>", "");

  return {false, std::move(unit_ctx)};
}

int main(int argc, char* argv[]) {
  using enum comp_err_lvl;

  try {
    // Argument parser entry point
    ArgumentParser argument_parser("via", vl_version);

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
    err_bus.log({true, e.what(), dummy_unit_ctx, ERROR_, {}});
  }

  return 0;
}
