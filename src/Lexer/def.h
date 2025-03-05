// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "token.h"

namespace via {

struct Definition {
    std::string        identifier;
    std::vector<Token> replacement;
    SIZE               begin;
    SIZE               end;
    SIZE               line;
};

} // namespace via