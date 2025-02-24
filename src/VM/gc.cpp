// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "gc.h"
#include "types.h"

namespace via {

GarbageCollector::~GarbageCollector()
{
    terminating = true;

    for (const GCCleanupFunction &fn : defered_callback_list) {
        fn();
    }
}

void GarbageCollector::collect()
{
    // TODO: Implement non-callback collection mechanism

    for (const GCCleanupFunction &fn : periodic_callback_list) {
        fn();
    }

    periodic_callback_list.clear();
    this->size = 0;
    this->collections++;
}

void GarbageCollector::add_periodic_callback(const GCCleanupFunction &fn)
{
    periodic_callback_list.push_back(fn);
}

void GarbageCollector::add_defered_callback(const GCCleanupFunction &fn)
{
    defered_callback_list.push_back(fn);
}

} // namespace via