/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <cstdint>
#include <filesystem>
#include <set>
#include <string>
#include <vector>

namespace via {
namespace cli {

struct ProgramOptions
{
    uint8_t verbosity = 0;
    bool no_execute = false;
    bool debugger = false;
    bool supress_missing_core_warning = false;
    std::filesystem::path input;
    std::set<std::string> dump;
    std::vector<std::string> imports;

    std::string to_string() const;
};

} // namespace cli
} // namespace via
