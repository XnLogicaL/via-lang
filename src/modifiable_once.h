/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

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
