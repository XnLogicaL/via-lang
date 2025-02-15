/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"

namespace via::utils {

// Writes the given content to the specified file path
void write_to_file(const std::string &file_path, const std::string &content);
// Reads the content of the specified file path into a string
std::string read_from_file(const std::string &file_path);

} // namespace via::utils