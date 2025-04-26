// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "handlers.h"

int main(int argc, char* argv[]) {
  try {
    // Argument Parser entry point
    argparse::ArgumentParser argument_parser("via", VIA_VERSION);

    auto compile_parser = handlers::get_standard_parser("compile");
    compile_parser->add_description("Compiles the given source file.");

    auto run_parser = handlers::get_standard_parser("run");
    run_parser->add_description("Compiles and runs the given source file.");

    auto dbg_parser = handlers::get_standard_parser("debug");
    dbg_parser->add_description("Opens interactive debugger");

    argparse::ArgumentParser repl_parser("repl");

    // Add subparsers
    argument_parser.add_subparser(*compile_parser);
    argument_parser.add_subparser(*run_parser);
    argument_parser.add_subparser(repl_parser);
    argument_parser.add_subparser(*dbg_parser);

    argument_parser.parse_args(argc, argv);

    if (argument_parser.is_subcommand_used(*compile_parser)) {
      handlers::handle_compile(*compile_parser);
    }
    else if (argument_parser.is_subcommand_used(*run_parser)) {
      handlers::handle_run(*run_parser);
    }
    else if (argument_parser.is_subcommand_used(repl_parser)) {
      handlers::handle_repl(repl_parser);
    }
    else if (argument_parser.is_subcommand_used(*dbg_parser)) {
      handlers::handle_debugger(*dbg_parser);
    }
    else {
      throw std::runtime_error("Subcommand expected");
    }
  }
  catch (const std::runtime_error& e) {
    handlers::err_bus.log({true, e.what(), handlers::dummy_unit_ctx, via::CErrorLevel::ERROR_, {}});
  }

  return 0;
}
