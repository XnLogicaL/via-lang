// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "gc.h"
#include "rt-types.h"

VIA_NAMESPACE_BEGIN

GarbageCollector::~GarbageCollector() {
    terminating = true;

    for (const CleanupFunction& fn : defered_callback_list) {
        fn();
    }
}

void GarbageCollector::collect() {
    for (const CleanupFunction& fn : periodic_callback_list) {
        fn();
    }

    periodic_callback_list.clear();
    this->size = 0;
    this->collections++;
}

void GarbageCollector::add_periodic_callback(const CleanupFunction& fn) {
    periodic_callback_list.push_back(fn);
}

void GarbageCollector::add_defered_callback(const CleanupFunction& fn) {
    defered_callback_list.push_back(fn);
}

VIA_NAMESPACE_END
