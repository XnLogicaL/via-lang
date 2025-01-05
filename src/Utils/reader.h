/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <format>

namespace FileReader
{

class BadFileException : std::exception
{

public:
    std::string file_path;

    BadFileException(std::string file_path)
        : file_path(file_path)
    {
    }

    const char *what() const throw()
    {
        return std::format("File '{}' does not exist", file_path).c_str();
    }
};

std::string read_file(std::string path);

} // namespace FileReader
