// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_CONSTANT_H
#define _VIA_CONSTANT_H

#include "common.h"
#include "rttypes.h"

// ===========================================================================================
// constant.h
//
VIA_NAMESPACE_BEGIN

class ConstantHolder final {
public:
    // Pushes a constant to the holder and returns the index of which the constant now lives.
    Operand push_constant(const TValue&);
    // Returns the size_t or next index of the constant table.
    size_t size() const noexcept;
    // Returns the constant at a given index.
    const TValue& at(size_t) const;
    // Returns the constant at a given index.
    // If the index is invalid, returns nil.
    const TValue& at_s(size_t) const noexcept;
    // Returns a reference to the constant table.
    const std::vector<TValue>& get() const noexcept;

private:
    std::vector<TValue> constants;
};

VIA_NAMESPACE_END

#endif
