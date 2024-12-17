/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include <stdexcept>
#include <string>
#include <vector>

namespace via::Tokenization::Preprocessing
{

class PreprocessorException : std::exception
{
private:
    std::string message;

public:
    PreprocessorException(const std::string &msg)
        : message(msg)
    {
    }

    const char *what() const throw()
    {
        return message.c_str();
    }
};

} // namespace via::Tokenization::Preprocessing
