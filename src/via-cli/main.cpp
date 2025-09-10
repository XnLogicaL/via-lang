/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include <spdlog/spdlog.h>
#include <via/via.h>
#include <csv2/reader.hpp>
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

// Expand $HOME on Unix, %USERPROFILE% on Windows.
static fs::path getHomeDir()
{
#ifdef _WIN32
  if (const char* profile = std::getenv("USERPROFILE")) {
    return fs::path(profile);
  }
  // Fallback: construct from HOMEDRIVE + HOMEPATH
  const char* drive = std::getenv("HOMEDRIVE");
  const char* path = std::getenv("HOMEPATH");
  if (drive && path) {
    return fs::path(std::string(drive) + path);
  }
  return fs::current_path();  // last resort
#else
  if (const char* home = std::getenv("HOME")) {
    return fs::path(home);
  }
  return fs::current_path();  // last resort
#endif
}

// Gets the base directory where via stores core stuff
static fs::path getCoreLangBaseDir()
{
#ifdef _WIN32
  if (const char* local = std::getenv("LOCALAPPDATA")) {
    return fs::path(local) / "via";
  }
  return getHomeDir() / "AppData" / "Local" / "via";
#else
  if (const char* xdg = std::getenv("XDG_DATA_HOME")) {
    return fs::path(xdg) / "via";
  }
  return getHomeDir() / ".local" / "share" / "via";
#endif
}

int main(int argc, char* argv[])
{
  spdlog::set_pattern("%^%l:%$ %v");

  const fs::path corePath = getCoreLangBaseDir();

  try {
    auto& cli = via::cli::getApp();

    csv2::Reader<csv2::delimiter<','>, csv2::quote_character<'\''>,
                 csv2::first_row_is_header<false>,
                 csv2::trim_policy::trim_whitespace>
      csvReader;

    std::string rawDumpMode, rawIncludeDirs;
    fs::path path;

    try {
      cli.parse_args(argc, argv);
      path = cli.get("input");
      rawDumpMode = cli.get("--dump");
      rawIncludeDirs = cli.get("--include-dirs");
    } catch (const std::bad_any_cast&) {
      assert(false, "bad argument");
    } catch (const std::exception& err) {
      assert(false, err.what());
    }

    assert(!path.empty(), "no input files");

    uint32_t flags = 0;
    {
      if (rawDumpMode == "ttree")
        flags |= Module::Flags::DUMP_TTREE;
      else if (rawDumpMode == "ast")
        flags |= Module::Flags::DUMP_AST;
      else if (rawDumpMode == "ir")
        flags |= Module::Flags::DUMP_IR;
      else if (rawDumpMode == "deftab")
        flags |= via::Module::DUMP_DEFTABLE;
    }

    if (!fs::exists(corePath)) {
      spdlog::warn("Could not find language core directory ({})",
                   corePath.string());
    }

    ModuleManager mgr;
    mgr.addImportPath(path.parent_path());

    if (!rawIncludeDirs.empty()) {
      assert(csvReader.parse(rawIncludeDirs), "bad import path list");

      for (const auto row : csvReader) {
        for (const auto cell : row) {
          std::string path;
          cell.read_value(path);
          mgr.addImportPath(path);
        }
      }
    }

    auto module = Module::loadSourceFile(&mgr, nullptr, path.stem().c_str(),
                                         path, Module::Perms::ALL, flags);

    assert(module.hasValue(),
           module.errorOr(via::Error::fail("<no-error>")).toString());

    if (rawDumpMode == "symtab") {
      std::println(std::cout, "{}",
                   via::ansi("[global symbol table]", via::Fg::Yellow,
                             via::Bg::Black, via::Style::Bold));

      for (const auto& sym : via::SymbolTable::instance().getSymbols()) {
        std::println(std::cout, "  {}: {}", sym.second, sym.first);
      }
    }
  } catch (int code) {
    return code;
  }

  return 0;
}
