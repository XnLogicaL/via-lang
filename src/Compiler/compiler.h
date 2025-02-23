// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "bytecode.h"
#include "cleaner.h"
#include "instruction.h"
#include "ast.h"
#include "visitor.h"

namespace via {

class Compiler {
public:
    Compiler(ProgramData *program)
        : program(program)
    {
    }

    ~Compiler()
    {
        cleaner.clean();
    }

    bool generate();

private:
    ProgramData *program;
    Cleaner cleaner;
};

} // namespace via