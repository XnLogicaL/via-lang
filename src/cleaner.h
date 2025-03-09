// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "common.h"

// ================================================================ |
// File cleaner.h: Cleaner class declaration.                       |
// ================================================================ |
// This file declares the Cleaner class.
//
// The Cleaner class is used mainly to defer callbacks and de-allocate
//  allocations (by also defering them).
// ================================================================ |
namespace via {

class Cleaner {
public:
    void add_callback(std::function<void(void)>);
    void add_malloc(const void*);
    void clean();

private:
    std::vector<const void*>               free_list;
    std::vector<std::function<void(void)>> callback_list;
};

} // namespace via