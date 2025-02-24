// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |
#include "cleaner.h"

// ================================================================ |
// File cleaner.cpp: Cleaner class definitions.                     |
// ================================================================ |
// This file defines the Cleaner class.
// ================================================================ |
namespace via {

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

} // namespace via