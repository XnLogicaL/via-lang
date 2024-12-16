/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "cleaner.h"

namespace via::Compilation
{

void Cleaner::clean()
{
    for (void *ptr : free_list)
        std::free(ptr);

    free_list.clear();
}

void Cleaner::add_malloc(const void *ptr)
{
    free_list.push_back(ptr);
}

} // namespace via::Compilation