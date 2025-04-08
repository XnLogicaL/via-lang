//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================
#include "linenoise.hpp"
#include "argparse/argparse.hpp"
#include "file-io.h"
#include "color.h"
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
  command->add_argument("--dump-assembly", "-Dasm")
    .help("Dumps human-readable assembly to the console upon compilation of the given source file")
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
    .scan<'u', size_t>()
    .default_value(size_t(1));
  command->add_argument("--verbose", "-v").help("Enables verbosity").flag();
  command->add_argument("--sassy").help("Enables sassy mode 😉").flag();
  command->add_argument("--Bcapitalize-opcodes")
    .help("Whether to capitalize opcodes inside bytecode dumps")
    .flag();

  command->add_argument("--allow-direct-bin-execution")
    .help("Allows direct binary execution")
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

  std::string file = subcommand_parser.get<std::string>("target");

  rd_result_t source_result = read_from_file(file);
  trans_unit_context unit_ctx(file, *source_result);
  unit_ctx.optimization_level = subcommand_parser.get<size_t>("--optimize");

  // Record compilation start time
  SET_PROFILER_POINT(compilation_start)

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

  SET_PROFILER_POINT(preproc_start);
  preprocessor.declare_default();
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
      GET_PROFILER_DIFF_MS(codegen_start, codegen_end) / 1000
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

    if (get_flag("--dump-assembly")) {
      print_flag_label("--dump-assembly");

      std::stack<std::string> closure_disassembly_stack;
      std::stack<size_t>
        closure_bytecode_count_stack; // Stack to track the bytecode count of each closure

      std::cout << apply_color(
        "[disassembly of section text]", fg_color::yellow, bg_color::black, style::underline
      ) << "\n";

      for (size_t i = 0; i < unit_ctx.bytecode->get().size(); ++i) {
        const bytecode& bytecode = unit_ctx.bytecode->get()[i];
        std::string current_disassembly;

        if (bytecode.instruct.op == opcode::LBL) {
          std::cout << std::format(
            " L{}{}:\n", bytecode.meta_data.comment, bytecode.instruct.operand0
          );
          continue;
        }
        else if (bytecode.instruct.op == opcode::NEWCLSR) {
          // Push the closure name and bytecode count to the stack
          closure_disassembly_stack.push(bytecode.meta_data.comment);
          closure_bytecode_count_stack.push(
            i + bytecode.instruct.operand1
          ); // Operand1 is the bytecode count

          std::cout << " [disassembly of function " << bytecode.meta_data.comment << ' '
                    << std::format("<at register {}>", bytecode.instruct.operand0)
                    << ", <instruction count " << bytecode.instruct.operand1 << '>' << "]:\n";
          continue;
        }

        // Print disassembly of the bytecode instruction
        std::cout << "  " << via::to_string(bytecode, get_flag("--Bcapitalize-opcodes")) << "\n";

        if (bytecode.instruct.op == opcode::RET || bytecode.instruct.op == opcode::RETNIL) {
          // Check if we are at the last RET opcode for the current closure
          if (!closure_disassembly_stack.empty() && i >= closure_bytecode_count_stack.top()) {
            // Pop the function from the stack
            std::string disassembly_of = closure_disassembly_stack.top();
            closure_disassembly_stack.pop();
            closure_bytecode_count_stack.pop();

            std::cout << " [end of disassembly of function " << disassembly_of << "]\n";
          }
        }
      }

      std::cout << apply_color(
        "[disassembly of section data]", fg_color::yellow, bg_color::black, style::underline
      ) << "\n";

      // Platform info
      std::cout << apply_color("  platform_info ", fg_color::magenta, bg_color::black, style::bold)
                << unit_ctx.get_platform_info() << "\n";

      size_t const_position = 0;
      for (const value_obj& constant : unit_ctx.constants->get()) {
        std::cout << apply_color("  constant", fg_color::magenta, bg_color::black, style::bold)
                  << ' ' << const_position++ << ": '"
                  << apply_color(constant.to_literal_cxx_string(), fg_color::green) << "' "
                  << apply_color(
                       std::format("({})", magic_enum::enum_name(constant.type)), fg_color::red
                     )
                  << "\n";
      }
    }

    if (get_flag("--dump-machine-code")) {
      print_flag_label("--dump-machine-code");

      for (const bytecode& bytecode : unit_ctx.bytecode->get()) {
        const instruction& instr = bytecode.instruct;
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
  using namespace utils;
  using enum comp_err_lvl;

  const auto get_flag = [&subcommand_parser](const std::string& flag) constexpr -> bool {
    return subcommand_parser.get<bool>(flag);
  };

  std::string file = subcommand_parser.get<std::string>("target");
  rd_result_t source_result = read_from_file(file);

  // Binary file check
  if (source_result->starts_with("%viac%")) {
    trans_unit_context unit_ctx({});

    if (!get_flag("--allow-direct-bin-execution")) {
      err_bus.log({
        true,
        "Executing a viac binary file directly may result in crashes, undefined behavior, or "
        "execution of untrusted code. Ensure the file is valid and not malicious before "
        "proceeding. This warning can be suppressed using '--allow-direct-bin-execution'.",
        dummy_unit_ctx,
        WARNING,
        {},
      });
    }

    return {false, std::move(unit_ctx)};
  }

  comp_result result = handle_compile(subcommand_parser);
  trans_unit_context& unit_ctx = result.unit;

  bool verbosity_flag = get_flag("--verbose");

  if (!result.failed) {
    SET_PROFILER_POINT(runtime_begin);
    SET_PROFILER_POINT(state_init_begin);

    global_state gstate;
    stack_registers_t stk_registers;
    state state(&gstate, stk_registers, result.unit);

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
  using namespace via;
  using namespace utils;

  constexpr const char REPL_WELCOME[] =
    "via v" VIA_VERSION " Copyright (C) 2024-2025 XnLogicaL\nLicensed under GNU GPL v3.0 @ "
    "https://github.com/XnLogical/via-lang.\n"
    "Use ':help' to see a list of commands.\n";

  constexpr const char REPL_BYE[] = "Quitting.\n";

  constexpr const char REPL_HELP[] =
    "repl commands:\n"
    "  :quit, :q - Quits repl\n"
    "  :help, :h - Prints this \"menu\"\n"
    "  :exit-code, :ec - Displays the last exit code returned by the VM\n";

  constexpr const char REPL_UNKNOWN_CMD[] =
    "Unkown command. Use ':help' to see a list of commands.\n";

  constexpr const char REPL_HEAD[] = "$> ";

  trans_unit_context unit_ctx("<repl>", "");

  std::cout << REPL_WELCOME;

  while (true) {
    std::string line = linenoise::Readline(REPL_HEAD);

    if (line.starts_with(':')) {
      if (line == ":quit" || line == ":q") {
        break;
      }
      else if (line == ":help" || line == ":h") {
        std::cout << REPL_HELP;
      }
      else {
        std::cout << REPL_UNKNOWN_CMD;
      }

      continue;
    }
  }

  std::cout << REPL_BYE;

  return {false, std::move(unit_ctx)};
}

#ifdef __linux__

void linux_ub_sig_handler(int signum) {
  static std::unordered_map<int, const char*> sig_id_map = {
    {1, "SIGHUP"},     {2, "SIGINT"},   {3, "SIGQUIT"},   {4, "SIGILL"},   {5, "SIGTRAP"},
    {6, "SIGABRT"},    {7, "SIGBUS"},   {8, "SIGFPE"},    {9, "SIGKILL"},  {10, "SIGUSR1"},
    {11, "SIGSEGV"},   {12, "SIGUSR2"}, {13, "SIGPIPE"},  {14, "SIGALRM"}, {15, "SIGTERM"},
    {16, "SIGSTKFLT"}, {17, "SIGCHLD"}, {18, "SIGCONT"},  {19, "SIGSTOP"}, {20, "SIGTSTP"},
    {21, "SIGTTIN"},   {22, "SIGTTOU"}, {23, "SIGURG"},   {24, "SIGXCPU"}, {25, "SIGXFSZ"},
    {26, "SIGVTALRM"}, {27, "SIGPROF"}, {28, "SIGWINCH"}, {29, "SIGIO"},   {30, "SIGPWR"},
    {31, "SIGSYS"}
  };

  auto sig_id = "SIGUNKNOWN";
  auto it = sig_id_map.find(signum);
  if (it != sig_id_map.end()) {
    sig_id = it->second;
  }

  err_bus.log({
    true,
    std::format("Program recieved signal {} ({})", signum, sig_id),
    dummy_unit_ctx,
    comp_err_lvl::ERROR_,
    {},
  });

  err_bus.emit();
  std::_Exit(1);
}

#endif // __linux__

int main(int argc, char* argv[]) {
  using enum comp_err_lvl;

#ifdef __linux__
  std::signal(SIGSEGV, linux_ub_sig_handler);
  std::signal(SIGILL, linux_ub_sig_handler);
  std::signal(SIGABRT, linux_ub_sig_handler);
#endif

  // Argument parser entry point
  ArgumentParser argument_parser("via", VIA_VERSION);

  auto compile_parser = get_standard_parser("compile");
  compile_parser->add_description("Compiles the given source file.");

  auto run_parser = get_standard_parser("run");
  run_parser->add_description("Compiles and runs the given source file.");

  ArgumentParser repl_parser("repl");

  // Add subparsers
  argument_parser.add_subparser(*compile_parser);
  argument_parser.add_subparser(*run_parser);
  argument_parser.add_subparser(repl_parser);
  argument_parser.parse_args(argc, argv);

  if (argument_parser.is_subcommand_used(*compile_parser)) {
    handle_compile(*compile_parser);
  }
  else if (argument_parser.is_subcommand_used(*run_parser)) {
    handle_run(*run_parser);
  }
  else if (argument_parser.is_subcommand_used(repl_parser)) {
    handle_repl(repl_parser);
  }
  else {
    throw std::logic_error("Subcommand expected");
  }

  return 0;
}
