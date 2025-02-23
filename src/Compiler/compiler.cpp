// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GPL v3.           |
// =========================================================================================== |

#include "compiler.h"
#include "types.h"

// ================================================================ |
// File compiler.cpp: Compiler class definitions.                   |
// ================================================================ |
// This file implements the Compiler class.
// ================================================================ |
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