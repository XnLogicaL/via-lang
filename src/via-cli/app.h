// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CLI_APP_H_
#define VIA_CLI_APP_H_

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
      .choices("", "ttree", "ast", "ir", "deftab", "symtab")
      .default_value("")
      .help("Dump the given tree");

  return cli;
}

}  // namespace cli

}  // namespace via

#endif
