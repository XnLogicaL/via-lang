// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |
#include "constant.h"
#include "api.h" // For `via::compare()`

// ================================================================ |
// constant.cpp
//
VIA_NAMESPACE_BEGIN

Operand ConstantHolder::push_constant(const TValue& constant) {
    for (u32 index = 0; index < constants.size(); index++) {
        const TValue& val = constants[index];
        if VIA_UNLIKELY (compare(val, constant)) {
            return index;
        }
    }

    constants.emplace_back(constant.clone());
    return constants.size() - 1;
}

size_t ConstantHolder::size() const noexcept {
    return constants.size();
}

const TValue& ConstantHolder::at(size_t index) const {
    return constants.at(index);
}

const TValue& ConstantHolder::at_s(size_t index) const noexcept {
    static const TValue nil;
    if (index >= size()) {
        return nil;
    }

    return at(index);
}

const std::vector<TValue>& ConstantHolder::get() const noexcept {
    return constants;
}

VIA_NAMESPACE_END
