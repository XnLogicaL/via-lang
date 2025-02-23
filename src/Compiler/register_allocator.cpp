// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GPL v3.           |
// =========================================================================================== |

#include "register_allocator.h"

namespace via {

U32 RegisterAllocator::allocate_register()
{
    for (const auto &[reg, is_free] : registers) {
        if (is_free) {
            registers.try_emplace(reg, false);
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
