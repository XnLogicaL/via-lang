// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include <spdlog/spdlog.h>
#include <via/via.h>
#include "app.h"

#undef assert

namespace fs = std::filesystem;

using via::Module;
using via::ModuleManager;

static void assert(bool cond, std::string msg)
{
  if (!cond) {
    spdlog::error(msg);
    throw 1;
  }
}

int main(int argc, char* argv[])
{
  spdlog::set_pattern("%^%l:%$ %v");

  try {
    auto& cli = via::cli::get_cli_app();

    std::string name, emit;
    fs::path path;

    try {
      cli.parse_args(argc, argv);
      path = cli.get("input");
      emit = cli.get("--emit");
    } catch (const std::bad_any_cast&) {
      assert(false, "bad argument");
    } catch (const std::exception& err) {
      assert(false, err.what());
    }

    via::u32 flags = 0;
    {
      if (emit == "ttree")
        flags |= Module::Flags::DUMP_TTREE;
      else if (emit == "ast")
        flags |= Module::Flags::DUMP_AST;
      else if (emit == "ir")
        flags |= Module::Flags::DUMP_IR;
    }

    ModuleManager mm;
    Module* module =
        Module::from_source(&mm, name.c_str(), path, Module::Perms::ALL, flags);
  } catch (int code) {
    return code;
  }

  return 0;
}
