// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_GC_H
#define _VIA_GC_H

#include "common.h"

VIA_NAMESPACE_BEGIN

struct State;
struct TValue;
struct GarbageCollector {
    using CleanupFunction = std::function<void(void)>;

    VIA_NON_COPYABLE(GarbageCollector);
    VIA_DEFAULT_CONSTRUCTOR(GarbageCollector);

    ~GarbageCollector();

    void collect();
    void add_periodic_callback(const CleanupFunction&);
    void add_defered_callback(const CleanupFunction&);

private:
    bool   terminating = false;
    size_t collections = 0;
    size_t size        = 0;

    std::vector<CleanupFunction> periodic_callback_list;
    std::vector<CleanupFunction> defered_callback_list;
};

VIA_NAMESPACE_END

#endif
