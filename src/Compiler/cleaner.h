/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

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