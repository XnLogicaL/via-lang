/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"

namespace via {

using GCCleanupFunction = std::function<void(void)>;

struct State;
struct TValue;
struct GarbageCollector {
    ~GarbageCollector();

    void collect();
    void add_periodic_callback(const GCCleanupFunction &);
    void add_defered_callback(const GCCleanupFunction &);

private:
    bool terminating = false;
    size_t collections = 0;
    size_t size = 0;

    std::vector<GCCleanupFunction> periodic_callback_list;
    std::vector<GCCleanupFunction> defered_callback_list;
};

} // namespace via
