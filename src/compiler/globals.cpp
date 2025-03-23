// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "globals.h"

VIA_NAMESPACE_BEGIN

using index_query_result  = GlobalTracker::index_query_result;
using global_query_result = GlobalTracker::global_query_result;
using global_vector       = GlobalTracker::global_vector;

size_t GlobalTracker::size() {
    return globals.size();
}

void GlobalTracker::declare_global(const Global& global) {
    globals.emplace_back(global);
}

bool GlobalTracker::was_declared(const Global& global) {
    return was_declared(global.symbol);
}

bool GlobalTracker::was_declared(const std::string& symbol) {
    for (const Global& global : globals) {
        if (global.symbol == symbol) {
            return true;
        }
    }

    return false;
}

index_query_result GlobalTracker::get_index(const Global& global) {
    return get_index(global.symbol);
}

index_query_result GlobalTracker::get_index(const std::string& symbol) {
    uint64_t index = 0;
    for (const Global& global : globals) {
        if (global.symbol == symbol) {
            return index;
        }

        ++index;
    }

    return std::nullopt;
}

global_query_result GlobalTracker::get_global(const std::string& symbol) {
    for (const Global& global : globals) {
        if (global.symbol == symbol) {
            return global;
        }
    }

    return std::nullopt;
}

global_query_result GlobalTracker::get_global(size_t index) {
    return globals.at(index);
}

const global_vector& GlobalTracker::get() {
    return globals;
}

VIA_NAMESPACE_END
