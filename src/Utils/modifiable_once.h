/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

namespace via::util
{

template<typename T>
class modifiable_once
{
    bool has_modified;
    T value;

public:
    modifiable_once(T value)
        : has_modified(false)
        , value(value)
    {
    }

    modifiable_once()
        : has_modified(false)
    {
    }

    void set(T new_value)
    {
        if (has_modified)
        {
            return;
        }

        has_modified = true;
        value = new_value;
    }

    T get()
    {
        return value;
    }
};

} // namespace via::util
