// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_CACHE_H
#define _VIA_CACHE_H

#include "SHA256.h"
#include "common.h"
#include "bytecode.h"
#include "fileio.h"

/* Binary file format:
    |===========|
    |8 bytes    | Magic value. (0xDEADBEEFULL)
    |4 bytes    | Version information for compatibility.
    |8 bytes    | Compilation date. (seconds since UNIX Epoch)
    |32 bytes   | File hash. (SHA-256)
    |16 bytes   | Platform info. (Arch, OS, etc.)
    |16 bytes   | Runtime flags. (-O3, -O2, etc.)
    |16 bytes   | Code section offset/size_t.
    |8 bytes    | Checksum A.
    |...bytes   | Bytecode.
    |8 bytes    | Checksum B.
    |=total=====|
    |116 bytes  |
*/

VIA_NAMESPACE_BEGIN

VIA_INLINE std::string hash_string_sha256(const std::string& src) {
    SHA256 sha;
    sha.update(src);

    std::array<u8, 32> digest = sha.digest();
    std::string        hash   = sha.toString(digest);

    return hash;
}

VIA_INLINE u8* hash_file_sha256(const std::string& src) {
    SHA256 sha;
    sha.update(src);

    std::array<u8, 32> digest     = sha.digest();
    std::string        hash       = sha.toString(digest);
    u8*                hash_final = new u8[32];

    for (size_t i = 0; i < 32; ++i) {
        std::stringstream ss;
        ss << std::hex << hash.substr(i * 2, 2);
        int byte_value;
        ss >> byte_value;
        hash_final[i] = static_cast<u8>(byte_value);
    }

    return hash_final;
}

VIA_INLINE const char* get_platform_info() {
    static char buffer[32]; // Sufficient size_t for "windows-x86-64" etc.

#ifndef _VIA_PLATFORM_INFO_FETCHED
#define _VIA_PLATFORM_INFO_FETCHED

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

    std::snprintf(buffer, sizeof(buffer), "%s-%s", os, arch);
#endif

    return buffer;
}

VIA_NAMESPACE_END

#endif
