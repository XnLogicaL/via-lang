/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"

namespace via
{

struct RTState;
struct TValue;
struct GCState
{
    bool terminating;
    size_t collections;
    size_t size;
    std::vector<TValue *> dellist;
    std::vector<std::weak_ptr<TValue>> reflist;
};

// Creates a new GCState object
GCState *gcnewstate();
// Adds a dynamically allocated value to the free list
void gcadd(RTState *, TValue *);
// Invokes garbage collection
void gccollect(RTState *);
// Adds a value reference to the gc
// These references are only cleaned up when the program is terminated
void gcaddref(RTState *, std::shared_ptr<TValue>);
// Cleansup a GCState object
void gccleanup(GCState *S);

} // namespace via
