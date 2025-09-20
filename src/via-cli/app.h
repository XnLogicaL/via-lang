/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <argparse/argparse.hpp>
#include <via/via.h>

namespace via {
namespace cli {

inline auto& app_instance() noexcept
{
    static argparse::ArgumentParser cli("via", "0.1.0");

    cli.add_argument("input").default_value("").help("Target source file");

    cli.add_argument("--no-execute").flag().help("Disable code execution");
    cli.add_argument("--debug").flag();

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

} // namespace cli
} // namespace via
