// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "instruction.h"

namespace via {

class RegisterAllocator {
public:
    RegisterAllocator(U32 size, bool default_value)
    {
        registers.reserve(size);
        for (U32 reg = 0; reg < size; reg++) {
            registers.emplace(reg, default_value);
        }
    }

    VIA_OPERAND allocate_register();
    VIA_OPERAND allocate_temp();
    void free_register(VIA_OPERAND);

private:
    std::unordered_map<VIA_OPERAND, bool> registers;
};

} // namespace via
