// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_MEMUTILS_H
#define _VIA_MEMUTILS_H

#include "common.h"

VIA_NAMESPACE_BEGIN

// Returns a raw memory dump.
std::string get_raw_memory_dump(const void* ptr, size_t size);

// Utility to dump raw bytes in hexadecimal format
void dump_raw_memory(const void* ptr, size_t size);

// General-purpose memory dump function
void dump_memory(const void* ptr, size_t size, const std::string& label = "");

// Utility to print a human-readable dump of a memory area
template<typename T>
void dump_struct(const T& obj) {
    std::cout << "Dumping object of type: " << typeid(T).name() << std::endl;

    if constexpr (std::is_integral<T>::value) {
        std::cout << "Integral value: " << obj << std::endl;
    }
    else if constexpr (std::is_floating_point<T>::value) {
        std::cout << "Floating-point value: " << obj << std::endl;
    }
    else if constexpr (std::is_pointer<T>::value) {
        std::cout << "Pointer value: " << obj << std::endl;
        std::cout << "Pointing to: ";
        if (obj) {
            dump_struct(*obj);
        }
        else {
            std::cout << "null" << std::endl;
        }
    }
    else {
        std::cout << "Object is of unknown type, printing raw bytes." << std::endl;
        dump_raw_memory(&obj, sizeof(T));
    }
}

VIA_NAMESPACE_END

#endif
