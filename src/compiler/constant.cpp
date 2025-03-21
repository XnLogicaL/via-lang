// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |
#include "constant.h"

// ================================================================ |
// constant.cpp
//
VIA_NAMESPACE_BEGIN

using constant_type   = ConstantHolder::constant_type;
using constant_vector = ConstantHolder::constant_vector;

size_t ConstantHolder::size() const {
    return constants.size();
}

Operand ConstantHolder::push_constant(constant_type& constant) {
    for (size_t index = 0; index < constants.size(); index++) {
        const TValue& val = constants[index];
        if VIA_UNLIKELY (val.compare(constant)) {
            return index;
        }
    }

    constants.emplace_back(constant.clone());
    return constants.size() - 1;
}

constant_type& ConstantHolder::at(size_t index) const {
    return constants.at(index);
}

constant_type& ConstantHolder::at_s(size_t index) const {
    static const TValue nil;
    if (index >= size()) {
        return nil;
    }

    return at(index);
}

const constant_vector& ConstantHolder::get() const {
    return constants;
}

VIA_NAMESPACE_END
