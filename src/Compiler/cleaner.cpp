/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "cleaner.h"

namespace via::Compilation
{

void Cleaner::clean()
{
    for (const void *ptr : free_list)
        std::free(const_cast<void *>(ptr));

    for (auto callback : callback_list)
        callback();

    free_list.clear();
    callback_list.clear();
}

void Cleaner::add_malloc(const void *ptr)
{
    free_list.push_back(ptr);
}

void Cleaner::add_callback(std::function<void(void)> callback)
{
    callback_list.push_back(callback);
}

} // namespace via::Compilation