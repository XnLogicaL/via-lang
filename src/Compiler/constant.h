// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |
#pragma once

#include "common.h"
#include "types.h"

// ================================================================ |
// File constant.h: This file declares the ConstantHolder class.    |
// ================================================================ |
//
// ================================================================ |
namespace via {

class ConstantHolder {
public:
    U32 push_constant(const TValue &);
    U32 size() const noexcept;
    const TValue &at(U64) const;

private:
    std::vector<TValue> constants;
};

} // namespace via
