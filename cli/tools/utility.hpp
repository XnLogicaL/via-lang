/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <filesystem>

namespace via {
namespace cli {

std::filesystem::path get_home_dir();
std::filesystem::path get_lang_dir();

} // namespace cli
} // namespace via
