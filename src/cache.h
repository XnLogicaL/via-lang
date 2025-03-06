/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see
 * LICENSE for license information */

#pragma once

#include "SHA256.h"
#include "common.h"
#include "bytecode.h"
#include "fileio.h"

#define VIA_CACHE_DIR_NAME "__viacache__"
#define VIA_BIN_EXT ".viac"
#define VIA_ASM_EXT ".viac.s"

/* Binary file format:
    |===========|
    |8 bytes    | Magic value. (0xDEADBEEFULL)
    |4 bytes    | Version information for compatibility.
    |8 bytes    | Compilation date. (seconds since UNIX Epoch)
    |32 bytes   | File hash. (SHA-256)
    |16 bytes   | Platform info. (Arch, OS, etc.)
    |16 bytes   | Runtime flags. (-O3, -O2, etc.)
    |16 bytes   | Code section offset/size.
    |8 bytes    | Checksum A.
    |...bytes   | Bytecode.
    |8 bytes    | Checksum B.
    |=total=====|
    |116 bytes  |
*/
namespace via {

VIA_INLINE std::string hash(const std::string& src)
{
    SHA256 sha;
    sha.update(src);

    std::array<U8, 32> digest = sha.digest();
    std::string        hash   = sha.toString(digest);

    return hash;
}

VIA_INLINE U8* hash_file(const std::string& src)
{
    SHA256 sha;
    sha.update(src);

    std::array<U8, 32> digest     = sha.digest();
    std::string        hash       = sha.toString(digest);
    U8*                hash_final = new U8[32];

    for (SIZE i = 0; i < 32; ++i) {
        std::stringstream ss;
        ss << std::hex << hash.substr(i * 2, 2);
        int byte_value;
        ss >> byte_value;
        hash_final[i] = static_cast<U8>(byte_value);
    }

    return hash_final;
}

VIA_INLINE const char* get_platform_info()
{
    static char buffer[32]; // Sufficient size for "windows-x86-64" etc.
#ifndef _PLATFORM_INFO_FETCHED
    #define _PLATFORM_INFO_FETCHED
    #if defined(_WIN32)
    const char* os = "windows";
    #elif defined(__linux__)
    const char* os = "linux";
    #else
    const char* os = "other";
    #endif
    #if defined(__x86_64__)
    const char* arch = "x86-64";
    #elif defined(i386)
    const char* arch = "x86-32";
    #elif defined(__aarch64__)
    const char* arch = "arm-64";
    #else
    const char* arch = "other";
    #endif

    std::snprintf(buffer, sizeof(buffer), "%s-%s", os, arch);
#endif
    return buffer;
}

enum class CacheResult {
    SUCCESS,
    FAIL,
};

struct CacheFile {
    std::string       file;
    U64               magic_value = 0xDEADBEEFULL; // 8 bytes
    U32               version;                     // 4 bytes
    U64               compilation_date;            // 8 bytes
    U8                file_hash[32];               // 32 bytes (SHA-256)
    const char*       platform_info;               // 8 bytes (e.g., "x86_64-linux")
    const char*       runtime_flags;               // 8 bytes (e.g., "-O3")
    U64               code_offset = 0;             // 8 bytes
    U64               code_size   = 0;             // 8 bytes
    U64               checksum_a  = 0;             // 8 bytes
    U64               checksum_b  = 0;             // 8 bytes
    std::vector<char> bytecode;
    ProgramData&      program;

    explicit CacheFile(ProgramData& program)
        : file(program.file)
        , version(std::stoi(VIA_VERSION))
        , compilation_date(std::chrono::steady_clock::now().time_since_epoch().count())
        , platform_info(get_platform_info())
        , runtime_flags("\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0")
        , program(program)
    {
        U8* hash_data = hash_file(program.source);
        std::memcpy(file_hash, hash_data, 32);
        delete[] hash_data;
    }
};

class CacheManager {
public:
    CacheResult write_cache(std::filesystem::path, const CacheFile&);
    CacheFile   read_cache(ProgramData&);

private:
    CacheResult make_cache(std::filesystem::path);
    bool        dir_has_cache(std::filesystem::path);
    bool        dir_has_cache_file(std::filesystem::path, std::string);
};

} // namespace via