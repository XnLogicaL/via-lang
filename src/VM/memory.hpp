#pragma once

#ifndef via_memory_hpp
#define via_memory_hpp

#include "../common.hpp"

#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(_Ty, _Ptr, _Old, _New) \
    (_Ty*)reallocate(_Ptr, sizeof(_Ty) * (_Old), \
        sizeof(_Ty) * (_New))

#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type) * (oldCount), 0)

void* reallocate(void* _Ptr, size_t _Old, size_t _New) {
    if (_New == 0)
    {
        free(_Ptr);
        return nullptr;
    }

    void* result = realloc(_Ptr, _New);

    if (result == nullptr)
        exit(1);

    return result;
}

#endif