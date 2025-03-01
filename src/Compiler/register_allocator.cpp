// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "register_allocator.h"

namespace via {

U32 RegisterAllocator::allocate_register()
{
    for (U32 reg = 0; reg < 128; reg++) {
        if (registers[reg]) {
            registers[reg] = false;
            return reg;
        }
    }

    return 0xDEADBEEF; // REGISTER_INVALID
}

U32 RegisterAllocator::allocate_temp()
{
    U32 reg = allocate_register();
    free_register(reg);
    return reg;
}

void RegisterAllocator::free_register(U32 reg)
{
    registers.emplace(reg, true);
}

} // namespace via
