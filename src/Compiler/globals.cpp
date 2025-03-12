// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "globals.h"

VIA_NAMESPACE_BEGIN

using index_query_result  = GlobalTracker::index_query_result;
using global_query_result = GlobalTracker::global_query_result;
using global_vector       = GlobalTracker::global_vector;

size_t GlobalTracker::size() noexcept {
    return globals.size();
}

void GlobalTracker::declare_global(const Global& global) noexcept {
    globals.emplace_back(global);
}

bool GlobalTracker::was_declared(const Global& global) noexcept {
    return was_declared(global.symbol);
}

bool GlobalTracker::was_declared(const std::string& symbol) noexcept {
    for (const Global& global : globals) {
        if (global.symbol == symbol) {
            return true;
        }
    }

    return false;
}

index_query_result GlobalTracker::get_index(const Global& global) noexcept {
    return get_index(global.symbol);
}

index_query_result GlobalTracker::get_index(const std::string& symbol) noexcept {
    u64 index = 0;
    for (const Global& global : globals) {
        if (global.symbol == symbol) {
            return index;
        }

        ++index;
    }

    return std::nullopt;
}

global_query_result GlobalTracker::get_global(const std::string& symbol) noexcept {
    for (const Global& global : globals) {
        if (global.symbol == symbol) {
            return global;
        }
    }

    return std::nullopt;
}

global_query_result GlobalTracker::get_global(size_t index) noexcept {
    try {
        return globals.at(index);
    }
    catch (const std::exception&) {
        return std::nullopt;
    }
}

const global_vector& GlobalTracker::get() noexcept {
    return globals;
}

VIA_NAMESPACE_END
