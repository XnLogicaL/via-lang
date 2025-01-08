/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"

namespace via
{

static const std::vector<std::string> built_in = {
    "print", "println", "error",  "exit",   "type", "typeof", "tostring", "tonumber", "tobool", "assert", "pcall", "xpcall",
    "math",  "table",   "string", "random", "http", "buffer", "bit32",    "utf8",     "fs",     "os",     "debug", "function",
};

} // namespace via