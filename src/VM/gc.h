/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"

namespace via::VM
{

struct GCState
{
    size_t collections;
    size_t size;
};
    
class GarbageCollector
{
    GCState state;
    std::vector<const void*> free_list;

public:

    inline void collect()
    {
        if (free_list.size() == 0)
        {
            return;
        }

        state.size = 0;
        state.collections++;

        for (const auto &p : free_list)
        {
            std::free(const_cast<void*>(p));
        }
    }

    template <typename T>
    inline void add(const T* p)
    {
        free_list.push_back(p);
        state.size += sizeof(p);
    }
};

} // namespace via::VM
