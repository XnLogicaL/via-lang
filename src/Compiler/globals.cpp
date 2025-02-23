// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
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

} // namespace via
