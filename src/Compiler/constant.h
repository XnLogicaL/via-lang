// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GPL v3.           |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "types.h"

namespace via {

class ConstantHolder {
public:
    U32 push_constant(const TValue &);
    U32 size() const noexcept;

private:
    std::vector<TValue> constants;
};

} // namespace via
