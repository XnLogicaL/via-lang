// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "register-allocator.h"

VIA_NAMESPACE_BEGIN

using register_type = RegisterAllocator::register_type;

register_type RegisterAllocator::allocate_register() {
    for (register_type reg = 0; reg < 128; reg++) {
        if (registers[reg]) {
            registers[reg] = false;
            return reg;
        }
    }

    return VIA_OPERAND_INVALID;
}

register_type RegisterAllocator::allocate_temp() {
    register_type reg = allocate_register();
    free_register(reg);
    return reg;
}

void RegisterAllocator::free_register(register_type reg) {
    registers.emplace(reg, true);
}

bool RegisterAllocator::is_used(register_type reg) {
    return registers[reg];
}

VIA_NAMESPACE_END
