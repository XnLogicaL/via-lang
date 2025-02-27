// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "globals.h"

namespace via {

void GlobalTracker::declare_global(Global global)
{
    globals.emplace_back(global);
}

bool GlobalTracker::was_declared(const std::string &symbol)
{
    for (const Global &global : globals) {
        if (global.symbol == symbol) {
            return true;
        }
    }

    return false;
}

std::optional<Global> GlobalTracker::get_global(const std::string &symbol)
{
    for (const Global &global : globals) {
        if (global.symbol == symbol) {
            return global;
        }
    }

    return std::nullopt;
}

std::optional<Global> GlobalTracker::get_global(U32 index)
{
    try {
        return globals.at(index);
    }
    catch (const std::exception &) {
        return std::nullopt;
    }
}

} // namespace via
