// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "common.h"

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

    U32 allocate_register();
    U32 allocate_temp();
    void free_register(U32);

private:
    std::unordered_map<U32, bool> registers;
};

} // namespace via
