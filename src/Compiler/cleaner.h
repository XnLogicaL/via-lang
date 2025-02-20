// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#pragma once

#include "common.h"

namespace via {

class Cleaner {
public:
    void add_callback(std::function<void(void)>);
    void add_malloc(const void *);
    void clean();

private:
    std::vector<const void *> free_list;
    std::vector<std::function<void(void)>> callback_list;
};

} // namespace via