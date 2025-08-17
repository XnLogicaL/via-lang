// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include <spdlog/spdlog.h>
#include <via/via.h>
#include "app.h"
#include "context.h"
#include "init.h"
#include "panic.h"
#include "process_file.h"

int main(int argc, char* argv[]) {
  try {
    via::cli::Context ctx;
    via::cli::init();

    auto& cli = via::cli::get_cli_app();

    try {
      cli.parse_args(argc, argv);
      ctx.path = cli.get("input");
      ctx.emit = cli.get("--emit");
    } catch (const std::bad_any_cast&) {
      via::cli::panic_assert(false, "bad argument");
    } catch (const std::exception& err) {
      via::cli::panic_assert(false, err.what());
    }

    via::cli::panic_assert(!ctx.path.empty(), "no input file");
    via::cli::process_file(ctx);
  } catch (int exit_code) {
    return exit_code;
  }

  return 0;
}
