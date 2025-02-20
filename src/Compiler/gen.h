// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#pragma once

#include "Parser/ast.h"
#include "arena.h"
#include "bytecode.h"
#include "chunk.h"
#include "cleaner.h"
#include "common.h"
#include "instruction.h"
#include "register.h"
#include "stack.h"


// Unlikely Register Id that serves as a non-masked value that carries
// information about register validity
#define VIA_REGISTER_INVALID (0xDEADBEEF)
// Quick way to check if the `target_register` parameter is a valid U32
// by comparing it with `VIA_REGISTER_INVALID`
#define LOAD_TO_REGISTER (target_register != VIA_REGISTER_INVALID)

#ifndef VIA_GENERATOR_ALLOC_SIZE
    #define VIA_GENERATOR_ALLOC_SIZE 8 * 1024 * 1024 // 8 MB
#endif

namespace via {

class Generator {
public:
    Generator(ProgramData *program)
        : program(program)
    {
        for (U32 gpr = 0; gpr < VIA_REGISTER_COUNT; gpr++) {
            register_pool[gpr] = true;
        }
    }

    ~Generator()
    {
        // Clean up resources using the cleaner
        // Bytecode is managed by the generator object, no need to delete manually
        cleaner.clean();
    }

    // Main function to generate the bytecode
    bool generate();
    // Register management functions
    U32 allocate_temp_register();
    U32 allocate_register();
    void free_register(U32);
    void add_bc_info(std::string);

public:
    ProgramData *program;
    std::vector<TValue> constants;

private:
    Cleaner cleaner;
    std::unordered_map<U32, bool> register_pool;
};

} // namespace via
