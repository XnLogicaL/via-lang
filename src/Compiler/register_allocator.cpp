// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "register_allocator.h"

namespace via {

VIA_OPERAND RegisterAllocator::allocate_register()
{
    for (VIA_OPERAND reg = 0; reg < 128; reg++) {
        if (registers[reg]) {
            registers[reg] = false;
            return reg;
        }
    }

    return VIA_OPERAND_INVALID;
}

VIA_OPERAND RegisterAllocator::allocate_temp()
{
    VIA_OPERAND reg = allocate_register();
    free_register(reg);
    return reg;
}

void RegisterAllocator::free_register(VIA_OPERAND reg)
{
    registers.emplace(reg, true);
}

} // namespace via
