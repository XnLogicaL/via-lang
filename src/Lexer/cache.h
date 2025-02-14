/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "SHA256.h"
#include "common.h"
#include "bytecode.h"
#include "fileio.h"

#define VIA_CACHE_DIR_NAME ("_viac")
#define VIA_BIN_EXT ("viac")
#define VIA_ASM_EXT ("viac.s")

/* Binary file format (VBFF):
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

VIA_FORCEINLINE uint8_t *hash_file(const std::string &src)
{
    SHA256 sha;
    sha.update(src);

    std::array<uint8_t, 32> digest = sha.digest();
    std::string hash = sha.toString(digest);

    // Assuming the hash is a 64-character hex string for SHA-256
    uint8_t *hash_final = new uint8_t[32]; // Allocate memory for 32 bytes (SHA-256)

    // Copy the hash into the uint8_t array (hex string to bytes)
    for (size_t i = 0; i < 32; ++i) {
        std::stringstream ss;
        ss << std::hex << hash.substr(i * 2, 2); // Each byte is 2 hex digits
        int byte_value;
        ss >> byte_value;
        hash_final[i] = static_cast<uint8_t>(byte_value);
    }

    return hash_final; // You should manage memory properly to avoid leaks
}

enum class CacheResult { SUCCESS, FAIL };

struct CacheFile {
    std::string file_name;
    uint64_t magic_value;      // 8 bytes
    uint32_t version;          // 4 bytes
    uint64_t compilation_date; // 8 bytes
    uint8_t file_hash[32];     // 32 bytes (SHA-256)
    char platform_info[16];    // 16 bytes (e.g., "x86_64-linux")
    char runtime_flags[16];    // 16 bytes (e.g., "-O3")
    uint64_t code_offset;      // 8 bytes
    uint64_t code_size;        // 8 bytes
    uint64_t checksum_a;       // 8 bytes
    uint64_t checksum_b;       // 8 bytes
    std::vector<char> bytecode;

    explicit CacheFile(ProgramData &program)
        : file_name(program.file_name)
        , magic_value(0xDEADBEEFULL)
        , version(std::stoi(VIA_VERSION))
        , compilation_date(std::chrono::steady_clock::now().time_since_epoch().count())
        , platform_info("\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0")
        , runtime_flags("\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0")
        , code_offset(0)
        , code_size(0)
        , checksum_a(0)
        , checksum_b(0)
        , bytecode({})
    {
        // Assuming the hash is 32 bytes
        uint8_t *hash_data = hash_file(program.source);
        std::memcpy(file_hash, hash_data, 32);
        delete[] hash_data; // Remember to free the dynamically allocated memory
    }
};

class CacheManager {
public:
    CacheResult write_cache(std::filesystem::path, const CacheFile &);
    CacheFile read_cache(ProgramData);

private:
    CacheResult make_cache(std::filesystem::path);
    bool dir_has_cache(std::filesystem::path);
    bool dir_has_cache_file(std::filesystem::path, std::string);
};

} // namespace via