// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "common_nodep.h"
#include "token.h"

namespace via {

struct Global {
    Token token;
    std::string symbol;
};

class GlobalTracker {
public:
    void declare_global(const Global &);

    bool was_declared(const Global &);
    bool was_declared(const std::string &);

    std::optional<U64> get_index(const std::string &);
    std::optional<U64> get_index(const Global &);

    std::optional<Global> get_global(const std::string &);
    std::optional<Global> get_global(U32);

    const std::vector<Global> &get();

    inline void declare_builtins()
    {
        static const std::vector<std::string> builtins = {
            "print",     "println", "error",  "exit",  "type",     "typeof", "to_string",
            "to_number", "to_bool", "assert", "pcall", "xpcall",   "math",   "table",
            "string",    "random",  "os",     "debug", "function",
        };

        for (const std::string &built_in : builtins) {
            globals.push_back(Global{
                .token = Token(TokenType::IDENTIFIER, built_in, 0, 0, 0),
                .symbol = built_in,
            });
        }
    }

private:
    std::vector<Global> globals;
};

} // namespace via
