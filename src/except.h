/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include <exception>
#include <string>
#include "common.h"

namespace via
{

class viaException : public std::exception
{
public:
    std::string message;

    viaException(const std::string &message)
        : message(message)
    {
    }

    const char *what() const throw()
    {
        return message.c_str();
    }
};

} // namespace via
