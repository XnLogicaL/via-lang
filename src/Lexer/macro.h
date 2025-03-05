// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "token.h"

namespace via {

struct Macro {
    std::string              name;   // Name of the macro
    std::vector<std::string> params; // Macro parameter names
    std::vector<Token>       body;   // Macro body as a list of tokens
    SIZE                     begin;
    SIZE                     end;
    SIZE                     line;
};

} // namespace via
