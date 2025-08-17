// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CLI_APP_H_
#define VIA_CLI_APP_H_

#include <via/via.h>
#include <argparse/argparse.hpp>

namespace via {

namespace cli {

inline argparse::ArgumentParser& get_cli_app() {
  static argparse::ArgumentParser cli(
      "via",
      via::Convert<via::Version>::to_string(via::get_semantic_version()));

  cli.add_argument("input").default_value("").help("Target source file");
  cli.add_argument("--emit", "-e")
      .nargs(1)
      .choices("none", "list", "ttree", "ast")
      .default_value("none")
      .help("Emission type");

  return cli;
}

}  // namespace cli

}  // namespace via

#endif
