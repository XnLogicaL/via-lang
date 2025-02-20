// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#pragma once

#include "common.h"

namespace via::utils {

template<typename T>
class ModifiableOnce {
public:
    ModifiableOnce(T val)
        : value(val)
    {
    }

    void set(T new_value);
    const T &get() const;
    T get();

private:
    bool has_modified = false;
    T value;
};

} // namespace via::utils
