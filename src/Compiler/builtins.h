// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#pragma once

#include "common.h"

namespace via {

static const std::vector<std::string> built_in = {
    "print",   "println", "error", "exit",   "type", "typeof", "to_string", "to_number",
    "to_bool", "assert",  "pcall", "xpcall", "math", "table",  "string",    "random",
    "http",    "buffer",  "bit32", "utf8",   "fs",   "os",     "debug",     "function",
};

} // namespace via