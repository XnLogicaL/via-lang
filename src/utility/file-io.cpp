// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "file-io.h"

VIA_NAMESPACE_UTIL_BEGIN

void write_to_file(const std::string& file_path, const std::string& content) {
    std::ofstream file(file_path, std::ios::out | std::ios::trunc); // Open in write mode
    if (!file.is_open()) {
        return;
    }
    file << content;
    file.close();
}

std::string read_from_file(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::in); // Open in read mode
    if (!file.is_open()) {
        return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return content;
}

VIA_NAMESPACE_END
