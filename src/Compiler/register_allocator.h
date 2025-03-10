// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_REGISTER_ALLOCATOR_H
#define _VIA_REGISTER_ALLOCATOR_H

#include "common.h"
#include "instruction.h"

VIA_NAMESPACE_BEGIN

class RegisterAllocator {
public:
    RegisterAllocator(U32 size, bool default_value) {
        registers.reserve(size);
        for (U32 reg = 0; reg < size; reg++) {
            registers.emplace(reg, default_value);
        }
    }

    Operand allocate_register();
    Operand allocate_temp();
    void    free_register(Operand);

private:
    std::unordered_map<Operand, bool> registers;
};

VIA_NAMESPACE_END

#endif
