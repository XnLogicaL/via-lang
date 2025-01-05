/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

namespace via::util
{

class debounce
{
    bool val;

public:
    debounce(bool v)
        : val(v)
    {
    }

    bool get()
    {
        bool vclone = val;
        val = !val;
        return vclone;
    }
};

} // namespace via::util
