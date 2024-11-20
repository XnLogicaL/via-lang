/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "types.h"

namespace via::VM
{

class Global
{
private:
    std::unordered_map<std::string_view, viaValue> consts;

public:
    inline bool set_global(const char *k, viaValue &v)
    {
        auto it = consts.find(std::string_view(k));

        if (it != consts.end())
            return 1;

        consts[std::string_view(k)] = v;

        return 0;
    }

    inline viaValue get_global(const char *k)
    {
        auto it = consts.find(std::string_view(k));

        if (it != consts.end())
            return viaValue();

        return consts[std::string_view(k)];
    }
};

} // namespace via::VM
