/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <via/via.h>
#include <argparse/argparse.hpp>

namespace via
{

namespace cli
{

inline auto& getApp()
{
  static argparse::ArgumentParser cli("via",
                                      via::toString(getSemanticVersion()));

  cli.add_argument("input").default_value("").help("Target source file");

  cli.add_argument("--include-dirs", "-I")
    .default_value("")
    .help("Comma seperated custom include directory paths");

  cli.add_argument("--dump", "-D")
    .nargs(1)
    .choices("", "ttree", "ast", "ir", "exe", "deftab", "symtab")
    .default_value("")
    .help("Dump the given tree");

  return cli;
}

}  // namespace cli

}  // namespace via
