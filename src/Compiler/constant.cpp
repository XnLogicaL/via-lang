// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GPL v3.           |
// =========================================================================================== |

#include "constant.h"
#include "api.h" // For `via::compare()`

namespace via {

U32 ConstantHolder::push_constant(const TValue &constant)
{
    for (U32 index = 0; index < constants.size(); index++) {
        const TValue &val = constants[index];
        if VIA_UNLIKELY (compare(val, constant)) {
            return index;
        }
    }

    constants.emplace_back(constant.clone());
    return constants.size() - 1;
}

U32 ConstantHolder::size() const noexcept
{
    return constants.size();
}

} // namespace via
