// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "globals.h"

VIA_NAMESPACE_BEGIN

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

std::optional<u64> GlobalTracker::get_index(const Global& global) {
    return get_index(global.symbol);
}

std::optional<u64> GlobalTracker::get_index(const std::string& symbol) {
    u64 index = 0;
    for (const Global& global : globals) {
        if (global.symbol == symbol) {
            return index;
        }

        ++index;
    }

    return std::nullopt;
}

std::optional<Global> GlobalTracker::get_global(const std::string& symbol) {
    for (const Global& global : globals) {
        if (global.symbol == symbol) {
            return global;
        }
    }

    return std::nullopt;
}

std::optional<Global> GlobalTracker::get_global(u32 index) {
    try {
        return globals.at(index);
    }
    catch (const std::exception&) {
        return std::nullopt;
    }
}

const std::vector<Global>& GlobalTracker::get() {
    return globals;
}

VIA_NAMESPACE_END
