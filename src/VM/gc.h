/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"

namespace via
{

struct viaState;
struct viaValue;
struct viaGCState
{
    bool terminating;
    size_t collections;
    size_t size;
    std::vector<viaValue *> dellist;
    std::vector<std::weak_ptr<viaValue>> reflist;
};

// Creates a new GCState object
viaGCState *viaGC_newstate();
// Adds a dynamically allocated value to the free list
void viaGC_add(viaState *, viaValue *);
// Invokes garbage collection
void viaGC_collect(viaState *);
// Adds a value reference to the gc
// These references are only cleaned up when the program is terminated
void viaGC_addref(viaState *, std::shared_ptr<viaValue>);
// Cleansup a GCState object
void viaGC_cleanup(viaGCState *S);

} // namespace via
