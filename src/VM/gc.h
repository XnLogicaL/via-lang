/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"

namespace via
{

using GCCleanupFunction = std::function<void(void)>;

struct RTState;
struct TValue;
struct GCState
{
    bool terminating;
    size_t collections;
    size_t size; // Similar to LuaHeap
    std::vector<TValue *> periodic_list;
    std::vector<GCCleanupFunction> callback_list;

    GCState();
    ~GCState();
};

// Adds a dynamically allocated value to the free list
void gcadd(RTState *, TValue *);
// Invokes garbage collection
void gccollect(RTState *);
void gcaddcallback(RTState *, GCCleanupFunction);

} // namespace via
