// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef vl_has_header_fileio_h
#define vl_has_header_fileio_h

#include "common.h"

namespace via::utils {

using rd_result_t = tl::expected<std::string, std::string>;

// Writes the given content to the specified file path
vl_nodiscard bool write_to_file(const std::string& file_path, const std::string& content);
// Reads the content of the specified file path into a string
vl_nodiscard rd_result_t read_from_file(const std::string& file_path);

} // namespace via::utils

#endif
