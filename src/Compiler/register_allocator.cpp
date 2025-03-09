// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "register_allocator.h"

VIA_NAMESPACE_BEGIN

Operand RegisterAllocator::allocate_register() {
    for (Operand reg = 0; reg < 128; reg++) {
        if (registers[reg]) {
            registers[reg] = false;
            return reg;
        }
    }

    return VIA_OPERAND_INVALID;
}

Operand RegisterAllocator::allocate_temp() {
    Operand reg = allocate_register();
    free_register(reg);
    return reg;
}

void RegisterAllocator::free_register(Operand reg) {
    registers.emplace(reg, true);
}

VIA_NAMESPACE_END
