// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "strutils.h"

VIA_NAMESPACE_BEGIN

char* duplicate_string(const std::string& str) {
    char* chars = new char[str.size() + 1];
    std::strcpy(chars, str.c_str());
    return chars;
}

char* duplicate_string(const char* str) {
    char* chars = new char[std::strlen(str) + 1];
    std::strcpy(chars, str);
    return chars;
}

u32 hash_string_custom(const char* str) {
    static const constexpr u32 BASE = 31;
    static const constexpr u32 MOD  = 0xFFFFFFFF;

    u32 hash = 0;
    while (char chr = *str++) {
        hash = (hash * BASE + static_cast<u32>(chr)) % MOD;
    }

    return hash;
}

VIA_NAMESPACE_END
