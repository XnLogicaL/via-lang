// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef VIA_HAS_HEADER_FILEIO_H
#define VIA_HAS_HEADER_FILEIO_H

#include "common.h"

namespace via::utils {

// Read result.
using rd_result_t = tl::expected<std::string, std::string>;

// Writes the given content to the specified file path
VIA_NODISCARD bool write_to_file(const std::string& file_path, const std::string& content);
// Reads the content of the specified file path into a string
VIA_NODISCARD rd_result_t read_from_file(const std::string& file_path);

} // namespace via::utils

#endif
