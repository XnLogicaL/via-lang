// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

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
    SIZE collections = 0;
    SIZE size        = 0;

    std::vector<GCCleanupFunction> periodic_callback_list;
    std::vector<GCCleanupFunction> defered_callback_list;
};

} // namespace via
