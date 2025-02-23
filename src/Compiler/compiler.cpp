// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#include "compiler.h"
#include "types.h"

namespace via {

bool Compiler::generate()
{
    bool failed = false;

    for (pStmtNode &stmt : program->ast->statements) {
    }

    return failed;
}

} // namespace via