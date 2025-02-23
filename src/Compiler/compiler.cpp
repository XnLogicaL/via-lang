// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#include "compiler.h"
#include "types.h"

namespace via {

bool Compiler::generate()
{
    RegisterAllocator allocator(VIA_REGISTER_COUNT, true);
    Emitter emitter(program);
    StmtVisitor visitor(program, emitter, allocator);

    for (pStmtNode &stmt : program->ast->statements) {
        stmt->accept(visitor);
    }

    return visitor.failed();
}

} // namespace via