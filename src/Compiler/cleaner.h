/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"

namespace via::Compilation
{

class Cleaner
{
public:
    Cleaner() = default;
    ~Cleaner() = default;

    void add_malloc(const void *);
    void clean();

private:
    std::vector<const void *> free_list;
};

} // namespace via::Compilation