/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"

namespace via {

using GCCleanupFunction = std::function<void(void)>;

struct State;
struct TValue;
struct GCState {
    bool terminating;
    size_t collections;
    size_t size;
    std::vector<GCCleanupFunction> callback_list;

    GCState();
    ~GCState();
};

// Invokes garbage collection
void gccollect(State *);
void gcaddcallback(State *, GCCleanupFunction);

} // namespace via
