// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "token.h"

namespace via {

struct Definition {
    std::string identifier;
    std::vector<Token> replacement;
    size_t begin;
    size_t end;
    size_t line;
};

} // namespace via