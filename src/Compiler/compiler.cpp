// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "compiler.h"
#include "strutils.h"

// ===========================================================================================
// compiler.cpp
//
VIA_NAMESPACE_BEGIN

bool Compiler::generate() {
    RegisterAllocator allocator(VIA_REGISTER_COUNT, true);
    Emitter           emitter(program);
    StmtVisitor       visitor(program, emitter, allocator);

    for (pStmtNode& stmt : program.ast->statements) {
        stmt->accept(visitor);
    }

    if (check_global_collisions()) {
        return true;
    }

    return visitor.failed();
}

bool Compiler::check_global_collisions() {
    Emitter emitter(program);

    bool                            failed = false;
    std::unordered_map<U32, Global> global_map;

    for (const Global& global : program.globals->get()) {
        U32  hash = hash_string_custom(global.symbol.c_str());
        auto it   = global_map.find(hash);

        if (it != global_map.end()) {
            failed = true;

            emitter.out(
                global.token.position,
                std::format(
                    "Global identifier '{}' collides with global identifier '{}'",
                    global.symbol,
                    it->second.symbol
                ),
                OutputSeverity::Error
            );

            emitter.out(
                it->second.token.position,
                std::format("Global '{}' declared here", it->second.symbol),
                OutputSeverity::Info
            );

            emitter.out_flat(
                "This limitation is due to a 32-bit bitspace used to identify "
                "globals during runtime. To fix it, try renaming either global to a non-related "
                "identifier.",
                OutputSeverity::Info
            );
        }

        global_map[hash] = global;
    }

    return failed;
}

VIA_NAMESPACE_END
