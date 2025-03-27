// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_FILEIO_H
#define _VIA_FILEIO_H

#include "common.h"

VIA_NAMESPACE_UTIL_BEGIN

using ReadResult = tl::expected<std::string, std::string>;

// Writes the given content to the specified file path
VIA_NO_DISCARD bool write_to_file(const std::string& file_path, const std::string& content);
// Reads the content of the specified file path into a string
VIA_NO_DISCARD ReadResult read_from_file(const std::string& file_path);

VIA_NAMESPACE_END

#endif
