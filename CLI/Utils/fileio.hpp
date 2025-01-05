/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include <fstream>
#include <string>
#include <stdexcept>

namespace via::utils
{

// Writes the given content to the specified file path
inline void write_to_file(const std::string &file_path, const std::string &content)
{
    std::ofstream file(file_path, std::ios::out | std::ios::trunc); // Open in write mode
    if (!file.is_open())
    {
        throw std::runtime_error("Error: Unable to open file for writing: " + file_path);
    }
    file << content;
    file.close();
}

// Reads the content of the specified file path into a string
inline std::string read_from_file(const std::string &file_path)
{
    std::ifstream file(file_path, std::ios::in); // Open in read mode
    if (!file.is_open())
    {
        throw std::runtime_error("Error: Unable to open file for reading: " + file_path);
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return content;
}

} // namespace via::utils