// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "ast.h"
#include "arena.h"
#include "highlighter.h"
#include "token.h"

namespace via {

class Parser {
public:
    Parser(ProgramData *program)
        : program(program)
        , emitter(program)
    {
    }

    bool parse_program();

private:
    ProgramData *program;
    Emitter emitter; // Error emitter

private:
};

} // namespace via
