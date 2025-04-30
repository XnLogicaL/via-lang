
#include "handlers.h"
#include "api-impl.h"

namespace handlers {

std::unique_ptr<ArgumentParser> get_standard_parser(std::string name) {
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
  command->add_argument("--Bcapitalize-opcodes")
    .help("Whether to capitalize opcodes inside bytecode dumps")
    .flag();

  command->add_argument("--allow-direct-bin-execution")
    .help("Allows direct binary execution")
    .flag();

  return command;
}

CompileResult handle_compile(argparse::ArgumentParser& subcommand_parser) {
  using enum TokenType;
  using enum CErrorLevel;

  const auto get_flag = [&subcommand_parser](const std::string& flag) -> bool {
    return subcommand_parser.get<bool>(flag);
  };

  const auto print_flag_label = [](const std::string& flag) -> void {
    std::cout << std::format("flag [{}]:\n", flag);
  };

  bool verbosity_flag = get_flag("--verbose");

  std::string file = subcommand_parser.get<std::string>("target");

  rd_result_t source_result = read_from_file(file);
  TransUnitContext unit_ctx(file, *source_result);
  unit_ctx.optimization_level = subcommand_parser.get<size_t>("--optimize");

  // Record compilation start time
  SET_PROFILER_POINT(compilation_start)

  if (!source_result.has_value()) {
    err_bus.log({true, source_result.error(), dummy_unit_ctx, ERROR_, {}});
    return {true, std::move(dummy_unit_ctx)};
  }

  Lexer Lexer(unit_ctx);
  Parser Parser(unit_ctx);
  BytecodeBuilder BytecodeBuilder(unit_ctx);

  SET_PROFILER_POINT(lex_start);
  Lexer.tokenize();

  if (verbosity_flag) {
    SET_PROFILER_POINT(lex_end);

    std::string message = std::format(
      "Tokenization completed in {:0.9f}s", GET_PROFILER_DIFF_MS(lex_start, lex_end) / 1000
    );

    err_bus.log({true, message, unit_ctx, INFO, {}});
  }

  SET_PROFILER_POINT(parser_start);
  bool parser_failed = Parser.parse();

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
  bool compiler_failed = BytecodeBuilder.generate();

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

  bool failed = parser_failed || compiler_failed;

  if (!failed) {
    if (get_flag("--dump-tokens")) {
      print_flag_label("--dump-tokens");

      for (const Token& Token : unit_ctx.tokens) {
        std::cout << Token.to_string() << "\n";
      }
    }

    if (get_flag("--dump-ast")) {
      print_flag_label("--dump-ast");
      uint32_t depth = 0;

      for (StmtNodeBase* pstmt : unit_ctx.ast) {
        std::cout << pstmt->to_string(depth) << "\n";
      }
    }

    if (get_flag("--dump-assembly")) {
      print_flag_label("--dump-assembly");

      std::stack<std::string> clsr_disas_stk;
      std::stack<size_t> clsr_bcc_stk; // Stack to track the bytecode count of each closure

      std::cout << apply_color(
        "[disassembly of function main (section<text>)]",
        fg_color::yellow,
        bg_color::black,
        style::underline
      ) << "\n";

      for (size_t i = 0; i < unit_ctx.bytecode.size(); ++i) {
        const Bytecode& bytecode = unit_ctx.bytecode[i];
        std::string curr_disas;

        if (bytecode.instruct.op == Opcode::LBL) {
          std::cout << std::format(" L{}{}:\n", bytecode.meta.comment, bytecode.instruct.a);
          continue;
        }
        else if (bytecode.instruct.op == Opcode::CLOSURE) {
          // Push the closure name and bytecode count to the stack
          clsr_disas_stk.push(bytecode.meta.comment);
          clsr_bcc_stk.push(i + bytecode.instruct.b); // Operand1 is the bytecode count

          std::cout << " [disassembly of function " << bytecode.meta.comment << ' '
                    << "<r=" << bytecode.instruct.a << ">, <ic=" << bytecode.instruct.b
                    << ">, <argc=" << bytecode.instruct.c << ">]:\n";
          continue;
        }

        // Print disassembly of the bytecode instruction
        std::cout << "  " << via::to_string(bytecode, get_flag("--Bcapitalize-opcodes")) << "\n";

        if (bytecode.instruct.op == Opcode::RET || bytecode.instruct.op == Opcode::RETNIL) {
          // Check if we are at the last RET Opcode for the current closure
          if (!clsr_disas_stk.empty() && i >= clsr_bcc_stk.top()) {
            // Pop the function from the stack
            std::string disassembly_of = clsr_disas_stk.top();
            clsr_disas_stk.pop();
            clsr_bcc_stk.pop();

            std::cout << " [end of disassembly of function " << disassembly_of << "]\n";
          }
        }
      }

      std::cout << apply_color(
        "[disassembly of section<data>]", fg_color::yellow, bg_color::black, style::underline
      ) << "\n";

      // Platform info
      std::cout << apply_color("  platform_info ", fg_color::magenta, bg_color::black, style::bold)
                << unit_ctx.get_platform_info() << "\n";

      size_t const_position = 0;
      for (const Value& constant : unit_ctx.constants) {
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

      for (const Bytecode& bytecode : unit_ctx.bytecode) {
        const Instruction& instr = bytecode.instruct;
        const size_t size = sizeof(Instruction);
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

CompileResult handle_run(argparse::ArgumentParser& subcommand_parser) {
  using namespace via;
  using enum CErrorLevel;

  const auto get_flag = [&subcommand_parser](const std::string& flag) -> bool {
    return subcommand_parser.get<bool>(flag);
  };

  std::string file = subcommand_parser.get<std::string>("target");
  rd_result_t source_result = read_from_file(file);

  // Binary file check
  if (source_result->starts_with("%viac%")) {
    TransUnitContext unit_ctx({});

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

  CompileResult result = handle_compile(subcommand_parser);
  TransUnitContext& unit_ctx = result.unit;

  bool verbosity_flag = get_flag("--verbose");

  if (!result.failed) {
    SET_PROFILER_POINT(runtime_begin);
    SET_PROFILER_POINT(state_init_begin);

    StkRegHolder stk_registers;
    State state(stk_registers, result.unit);

    if (verbosity_flag) {
      SET_PROFILER_POINT(state_init_end);

      std::string message = std::format(
        "State initialized in {:0.9f}s",
        GET_PROFILER_DIFF_MS(state_init_begin, state_init_end) / 1000
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

void handle_repl(argparse::ArgumentParser&) {
  using namespace via;

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

  TransUnitContext unit_ctx("<repl>", "");

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
}

void handle_debugger(argparse::ArgumentParser& parser) {
  using enum CErrorLevel;

  static constexpr const char* DBG_HELP = "Commands:\n"
                                          "  quit            - exit debugger\n"
                                          "  step            - step next instruction\n"
                                          "  continue        - run until break\n"
                                          "  regs            - show all registers\n"
                                          "  printr %<n>     - print register\n"
                                          "  locals          - show local variables\n"
                                          "  upvs            - show upvalues\n"
                                          "  callstack       - print call stack\n"
                                          "  exec <instr>    - manually run instruction\n"
                                          "  help            - show this help\n"
                                          "  pc              - print program counter\n";

  auto get_callable_string = [](const Callable& callee) -> std::string {
    return callee.type == Callable::Tag::Function
      ? std::string(callee.u.fn->id)
      : std::format("<nativefn@0x{:x}>", reinterpret_cast<uintptr_t>(callee.u.ntv));
  };

  CompileResult result = handle_compile(parser);
  if (result.failed) {
    err_bus.log({true, "Failed to launch debugger: compilation failed", result.unit, ERROR_, {}});
    return;
  }

  StkRegHolder regs;
  State state(regs, result.unit);

  while (true) {
    auto line = linenoise::Readline("(dbg) ");
    auto tokens = fast_tokenize(line);
    size_t cursor = 0;
    if (tokens.empty()) {
      continue;
    }

    if (tokens[0].lexeme == "exec") {
      cursor++;

      bool found_opcode = false;
      size_t operand_cursor = 0;
      std::string insn_str;
      Instruction insn;

      // 5 for:
      // exec <OPCODE> <A> <B> <C>
      if (tokens.size() < 5) {
        goto syntax_error;
      }

      while (cursor < tokens.size() - 1) {
        if (tokens[cursor].type == TokenType::IDENTIFIER) {
          auto op = magic_enum::enum_cast<Opcode>(tokens[cursor].lexeme);
          if (found_opcode || !op.has_value()) {
            goto syntax_error;
          }

          cursor++;
          found_opcode = true;
          insn.op = *op;
        }
        else if (tokens[cursor].type == TokenType::LIT_INT) {
          try {
            operand_t opi = std::stoul(tokens[cursor].lexeme);
            if (operand_cursor == 0) {
              insn.a = opi;
            }
            else if (operand_cursor == 1) {
              insn.b = opi;
            }
            else if (operand_cursor == 2) {
              insn.c = opi;
            }
            else {
              goto syntax_error;
            }
          }
          catch (const std::exception& e) {
            goto syntax_error;
          }

          cursor++;
          operand_cursor++;
        }
      }

      state.execute_step(insn);
      continue;
    }
    else if (tokens[0].lexeme == "quit") {
      break;
    }
    else if (tokens[0].lexeme == "step") {
      if (!state.pc) {
        std::cout << "no instruction\n";
        continue;
      }

      state.execute_step();
    }
    else if (tokens[0].lexeme == "continue") {
      if (!state.pc) {
        std::cout << "no instruction\n";
        continue;
      }

      state.execute();
    }
    else if (tokens[0].lexeme == "help") {
      std::cout << DBG_HELP;
    }
    else if (tokens[0].lexeme == "pc") {
      if (state.pc == nullptr) {
        std::cout << "no instruction\n";
        continue;
      }

      std::cout << "program counter: " << state.pc << "\n";
      std::cout << "disassembly    : " << magic_enum::enum_name(state.pc->op) << ' '
                << (state.pc->a != 0xFFFF ? std::to_string(state.pc->a) : "")
                << (state.pc->b != 0xFFFF ? std::to_string(state.pc->b) : "")
                << (state.pc->c != 0xFFFF ? std::to_string(state.pc->c) : "") << "\n";
    }
    else if (tokens[0].lexeme == "locals") {
      if (state.callstack->frames_count == 0) {
        std::cout << "no callframe\n";
        continue;
      }

      const CallFrame* frame = impl::__current_callframe(&state);
      std::cout << "in callframe #" << state.callstack->frames_count << " function "
                << get_callable_string(frame->closure->callee) << "\n";
      std::cout << "local count: " << frame->locals_size << "\n";
      for (size_t i = 0; i < frame->locals_size; i++) {
        std::cout << 'l' << i << ": " << magic_enum::enum_name(frame->locals[i].type) << ' '
                  << frame->locals[i].to_literal_cxx_string() << "\n";
      }
    }
    else if (tokens[0].lexeme == "regs") {
      std::cout << "disassembling 256 stack-allocated registers\n";
      for (uint16_t reg = 0; reg < 255; reg++) {
        Value& val = state.get_register(reg);
        std::cout << 'r' << reg << ": " << magic_enum::enum_name(val.type) << ' '
                  << val.to_literal_cxx_string() << "\n";
        if (val.is_nil()) {
          std::cout << "<nil-found>\n";
          break;
        }
      }
    }
    else if (tokens[0].lexeme == "callstack") {
      std::cout << "callframe count: " << state.callstack->frames_count << "\n";
      if (state.callstack->frames_count == 0) {
        continue;
      }

      for (size_t i = state.callstack->frames_count; i > 0; i--) {
        const CallFrame& visited_frame = state.callstack->frames[i - 1];
        const Callable& callee = visited_frame.closure->callee;
        std::cout << "#" << state.callstack->frames_count - i << " function "
                  << get_callable_string(callee) << "\n";
      }
    }
    else if (tokens[0].lexeme == "printr") {
      if (tokens.size() < 2) {
        goto syntax_error;
      }

      try {
        operand_t reg = std::stoul(tokens[1].lexeme);
        Value& atreg = state.get_register(reg);

        std::cout << 'r' << reg << ": " << magic_enum::enum_name(atreg.type) << ' '
                  << atreg.to_literal_cxx_string() << "\n";
      }
      catch (const std::exception&) {
        goto syntax_error;
      }
    }
    else {
    syntax_error:
      std::cout << "syntax error\n";
      continue;
    }
  };
}

} // namespace handlers
