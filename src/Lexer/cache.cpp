/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "cache.h"
#include "encoder.h"

#define VIA_CACHE_DIR_FS_PATH (dir.concat(VIA_CACHE_DIR_NAME))
#define VIA_CACHE_DIR_FS_FILE_PATH(file) (dir.concat(VIA_CACHE_DIR_NAME).concat(file))
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

namespace via
{

namespace fs = std::filesystem;

bool CacheManager::dir_has_cache(fs::path dir)
{
    return fs::is_directory(VIA_CACHE_DIR_FS_PATH);
}

bool CacheManager::dir_has_cache_file(fs::path dir, std::string file_name)
{
    return fs::is_directory(VIA_CACHE_DIR_FS_FILE_PATH(file_name));
}

CacheResult CacheManager::make_cache(fs::path dir)
{
    bool success = fs::create_directory(VIA_CACHE_DIR_FS_PATH);
    return success ? CacheResult::SUCCESS : CacheResult::FAIL;
}

CacheResult CacheManager::write_cache(fs::path path, const CacheFile &file)
{
    if (!dir_has_cache(path))
        make_cache(path);

    bool failed = false;
    // Open the file for binary writing
    std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    if (!ofs.is_open())
        return CacheResult::FAIL;

    // Helper lambda to write data
    auto write_data = [&failed, &ofs](const void *data, size_t size)
    {
        ofs.write(static_cast<const char *>(data), size);
        if (!ofs)
            failed = true;
    };

    // Write each field of CacheFile
    write_data(&file.magic_value, sizeof(file.magic_value));
    write_data(&file.version, sizeof(file.version));
    write_data(&file.compilation_date, sizeof(file.compilation_date));
    write_data(file.file_hash, ARRAY_SIZE(file.file_hash));
    write_data(file.platform_info, ARRAY_SIZE(file.platform_info));
    write_data(file.runtime_flags, ARRAY_SIZE(file.runtime_flags));
    write_data(&file.code_offset, sizeof(file.code_offset));
    write_data(&file.code_size, sizeof(file.code_size));
    write_data(&file.checksum_a, sizeof(file.checksum_a));
    // Write the encoded machine code into the cache file
    write_data(file.bytecode.data(), file.bytecode.size());
    // Write checksum B
    write_data(&file.checksum_b, sizeof(file.checksum_b));

    // Close the file
    ofs.close();
    if (!ofs || failed)
        return CacheResult::FAIL;

    return CacheResult::SUCCESS;
}

CacheFile CacheManager::read_cache(SrcContainer file)
{
    size_t offset = 0;
    CacheFile cache_file{file};
    const char *raw_source = dupstring(file.source);
    // Ensure the file is large enough to contain the metadata.
    if (file.source.size() < sizeof(CacheFile))
    { // Check if goto statements are supported in this compiler. (FUCK MSVC!)
#if defined(__GNUC__) || defined(__clang__)
        goto exit;
#else
        std::free(raw_source);
        return cache_file;
#endif
    }

    cache_file.file_name = file.file_name;

    // Read magic value
    std::memcpy(&cache_file.magic_value, raw_source + offset, sizeof(cache_file.magic_value));
    offset += sizeof(cache_file.magic_value);

    // Read version
    std::memcpy(&cache_file.version, raw_source + offset, sizeof(cache_file.version));
    offset += sizeof(cache_file.version);

    // Read compilation date
    std::memcpy(&cache_file.compilation_date, raw_source + offset, sizeof(cache_file.compilation_date));
    offset += sizeof(cache_file.compilation_date);

    // Read file hash
    std::memcpy(cache_file.file_hash, raw_source + offset, ARRAY_SIZE(cache_file.file_hash));
    offset += ARRAY_SIZE(cache_file.file_hash);

    // Read platform info
    std::memcpy(cache_file.platform_info, raw_source + offset, ARRAY_SIZE(cache_file.platform_info));
    offset += ARRAY_SIZE(cache_file.platform_info);

    // Read runtime flags
    std::memcpy(cache_file.runtime_flags, raw_source + offset, ARRAY_SIZE(cache_file.runtime_flags));
    offset += ARRAY_SIZE(cache_file.runtime_flags);

    // Read code offset
    std::memcpy(&cache_file.code_offset, raw_source + offset, sizeof(cache_file.code_offset));
    offset += sizeof(cache_file.code_offset);

    // Read code size
    std::memcpy(&cache_file.code_size, raw_source + offset, sizeof(cache_file.code_size));
    offset += sizeof(cache_file.code_size);

    // Read checksum A
    std::memcpy(&cache_file.checksum_a, raw_source + offset, sizeof(cache_file.checksum_a));
    offset += sizeof(cache_file.checksum_a);

    // Read the bytecode if code_size > 0
    if (cache_file.code_size > 0)
    {
        cache_file.bytecode.resize(cache_file.code_size);
        std::memcpy(cache_file.bytecode.data(), raw_source + offset, cache_file.code_size);
        offset += cache_file.code_size;
    }

    // Read checksum B
    std::memcpy(&cache_file.checksum_b, raw_source + offset, sizeof(cache_file.checksum_b));
    offset += sizeof(cache_file.checksum_b);

#if defined(__GNUC__) || defined(__clang__)
    goto exit;
exit:
#endif

    std::free(const_cast<char *>(raw_source));
    return cache_file;
}

} // namespace via