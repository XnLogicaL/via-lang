// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "cache.h"

VIA_NAMESPACE_BEGIN

const char* get_platform_info() {
    static char buffer[32];
    static bool fetched = false;

#ifdef _WIN32
    const char* os = "windows";
#elifdef __linux__
    const char* os = "linux";
#else
    const char* os = "other";
#endif

#ifdef __x86_64__
    const char* arch = "x86-64";
#elifdef i386
    const char* arch = "x86-32";
#elifdef __aarch64__
    const char* arch = "arm-64";
#else
    const char* arch = "other";
#endif

    if (!fetched) {
        fetched = true;
        std::snprintf(buffer, sizeof(buffer), "%s-%s", os, arch);
    }

    return buffer;
}

VIA_NAMESPACE_END
