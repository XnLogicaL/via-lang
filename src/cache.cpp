/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see
 * LICENSE for license information */

#include "cache.h"

#define VIA_CACHE_DIR_FS_PATH (dir.concat(VIA_CACHE_DIR_NAME))
#define VIA_CACHE_DIR_FS_FILE_PATH(file) (dir.concat(VIA_CACHE_DIR_NAME).concat(file))
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

namespace via {

namespace fs = std::filesystem;

bool CacheManager::dir_has_cache(fs::path dir)
{
    return fs::is_directory(VIA_CACHE_DIR_FS_PATH);
}

bool CacheManager::dir_has_cache_file(fs::path dir, std::string file)
{
    return fs::is_directory(VIA_CACHE_DIR_FS_FILE_PATH(file));
}

CacheResult CacheManager::make_cache(fs::path dir)
{
    bool success = fs::create_directory(VIA_CACHE_DIR_FS_PATH);
    return success ? CacheResult::SUCCESS : CacheResult::FAIL;
}

CacheResult CacheManager::write_cache(fs::path path, const CacheFile& file)
{
    if (!dir_has_cache(path)) {
        make_cache(path);
    }

    std::string file_name_hash = hash(file.file);
    auto        bin_path       = path / VIA_CACHE_DIR_NAME / (file_name_hash + VIA_BIN_EXT);
    auto        asm_path       = path / VIA_CACHE_DIR_NAME / (file_name_hash + VIA_ASM_EXT);

    bool          failed = false;
    std::ofstream ofs_bin(bin_path, std::ios::binary | std::ios::trunc);
    if (!ofs_bin.is_open()) {
        return CacheResult::FAIL;
    }

    std::ofstream ofs_asm(asm_path);
    if (!ofs_asm.is_open()) {
        return CacheResult::FAIL;
    }

    // Helper lambda to write data
    auto write_data = [&failed, &ofs_bin](const void* data, SIZE size) {
        ofs_bin.write(static_cast<const char*>(data), size);
        if (!ofs_bin) {
            failed = true;
        }
    };

    write_data(&file.magic_value, sizeof(file.magic_value));
    write_data(&file.version, sizeof(file.version));
    write_data(&file.compilation_date, sizeof(file.compilation_date));
    write_data(file.file_hash, ARRAY_SIZE(file.file_hash));
    write_data(file.platform_info, std::strlen(file.platform_info));
    write_data(file.runtime_flags, std::strlen(file.runtime_flags));
    write_data(&file.code_offset, sizeof(file.code_offset));
    write_data(&file.code_size, sizeof(file.code_size));
    write_data(&file.checksum_a, sizeof(file.checksum_a));
    write_data(file.bytecode.data(), file.bytecode.size());
    write_data(&file.checksum_b, sizeof(file.checksum_b));

    for (const Bytecode& pair : file.program.bytecode->get()) {
        std::string instr_str = via::to_string(pair) + "\n";
        ofs_asm.write(instr_str.data(), instr_str.size());
    }

    ofs_bin.close();
    ofs_asm.close();

    if (!ofs_bin || !ofs_asm || failed) {
        return CacheResult::FAIL;
    }

    return CacheResult::SUCCESS;
}

CacheFile CacheManager::read_cache(ProgramData& file)
{
    CacheFile cache_file{file};
    return cache_file;
}

} // namespace via