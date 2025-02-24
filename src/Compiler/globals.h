// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "common_nodep.h"

namespace via {

struct Global {
    std::string symbol;
    U64 declared_at;

    Global(const std::string &symbol, U64 declared_at)
        : symbol(symbol)
        , declared_at(declared_at)
    {
    }

    Global(const std::string &symbol)
        : symbol(symbol)
        , declared_at(std::numeric_limits<U64>::quiet_NaN())
    {
    }
};

class GlobalTracker {
public:
    void declare_global(Global);
    bool was_declared(const std::string &);
    std::optional<Global> get_global(const std::string &);

    inline void declare_builtins()
    {
        static const std::vector<std::string> builtins = {
            "print",   "println", "error", "exit",   "type", "typeof", "to_string", "to_number",
            "to_bool", "assert",  "pcall", "xpcall", "math", "table",  "string",    "random",
            "http",    "buffer",  "bit32", "utf8",   "fs",   "os",     "debug",     "function",
        };

        for (const std::string &built_in : builtins) {
            globals.emplace_back(Global(built_in));
        }
    }

private:
    std::vector<Global> globals;
};

} // namespace via
