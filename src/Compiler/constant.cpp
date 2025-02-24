// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |
#include "constant.h"
#include "api.h" // For `via::compare()`

// ================================================================ |
// File constant.cpp: ConstantHolder definitions.                   |
// ================================================================ |
// This file defines ConstantHolder.
// ================================================================ |
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

const TValue &ConstantHolder::at(U64 index) const
{
    return constants.at(index);
}

} // namespace via
