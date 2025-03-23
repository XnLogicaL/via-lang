// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "file-io.h"

VIA_NAMESPACE_UTIL_BEGIN

VIA_NO_DISCARD bool write_to_file(const std::string& file_path, const std::string& content) {
    std::ofstream file(file_path, std::ios::out | std::ios::trunc); // Open in write mode
    if (!file.is_open()) {
        return false;
    }

    file << content;
    file.close();

    return true;
}

VIA_NO_DISCARD ReadResult read_from_file(const std::string& file_path) {
    auto get_error_string = [&file_path](const std::string& err) -> std::string {
        return std::format("Failed to read file '{}': {}", file_path, err);
    };

    std::ifstream file(file_path, std::ios::in); // Open in read mode
    if (!file.is_open()) {
        return tl::unexpected(get_error_string("No such file or directory"));
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    if (file.bad()) {
        return tl::unexpected(get_error_string("IO Error"));
    }
    else if (file.fail() && !file.eof()) {
        return tl::unexpected(get_error_string("Non-recoverable format"));
    }

    file.close();
    return content;
}

VIA_NAMESPACE_END
