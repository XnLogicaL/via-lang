// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_COMMON_H
#define _VIA_COMMON_H

#include <cstddef>
// External imports
#include "magic_enum.hpp"
#include "linenoise.hpp"
#include "arena.h"
// Internal imports
#include "token.h"
#include "common_nodep.h"
#include "program.h"

#define ASMJIT_STATIC

VIA_NAMESPACE_BEGIN

template<typename T, typename F>
    requires(std::invocable<F> && std::is_same_v<std::invoke_result_t<F>, T>)
VIA_FORCE_INLINE T safe_call(F func, T default_value) {
    try {
        return func();
    }
    catch (std::exception&) {
        return default_value;
    }
}

VIA_INLINE std::string memdump(const void* ptr, u64 size) {
    std::ostringstream oss;
    const uint8_t*     bytePtr = reinterpret_cast<const uint8_t*>(ptr);

    oss << "Memory dump at: " << ptr << " (size_t: " << size << " bytes)\n";

    for (size_t i = 0; i < size; i += 16) {
        oss << std::setw(6) << std::setfill('0') << std::hex << i << " | ";

        for (size_t j = 0; j < 16 && i + j < size; ++j) {
            oss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(bytePtr[i + j])
                << " ";
        }

        oss << " | ";

        for (size_t j = 0; j < 16 && i + j < size; ++j) {
            char c = bytePtr[i + j];
            oss << (c >= 32 && c <= 126 ? c : '.');
        }

        oss << '\n';
    }
    oss << std::dec; // Reset formatting

    return oss.str();
}

VIA_NAMESPACE_END

#endif
