/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"

namespace via::utils {

template<typename T>
class ModifiableOnce {
    bool has_modified;
    T value;

public:
    // Initialized constructor; value is set by user
    ModifiableOnce(T value)
        : has_modified(false)
        , value(value)
    {
    }

    // Uninitialized constructor; value is junk data
    ModifiableOnce()
        : has_modified(false)
    {
    }

    void set(T new_value)
    {
        if (has_modified)
            return;

        has_modified = true;
        value = new_value;
    }

    T get()
    {
        return value;
    }

    const T &get() const
    {
        return value;
    }
};

} // namespace via::utils
