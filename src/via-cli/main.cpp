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

    std::string dump;
    fs::path path;

    try {
      cli.parse_args(argc, argv);
      path = cli.get("input");
      dump = cli.get("--dump");
    } catch (const std::bad_any_cast&) {
      assert(false, "bad argument");
    } catch (const std::exception& err) {
      assert(false, err.what());
    }

    uint32_t flags = 0;
    {
      if (dump == "ttree")
        flags |= Module::Flags::DUMP_TTREE;
      else if (dump == "ast")
        flags |= Module::Flags::DUMP_AST;
      else if (dump == "ir")
        flags |= Module::Flags::DUMP_IR;
    }

    assert(!path.empty(), "no input files");

    ModuleManager mm;
    auto module = Module::from_source(&mm, nullptr, path.stem().c_str(), path,
                                      Module::Perms::ALL, flags);

    assert(module.has_value(), module.error_or("<no-error>"));
  } catch (int code) {
    return code;
  }

  return 0;
}
