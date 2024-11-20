/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include <exception>
#include <string>
#include "common.h"

namespace via
{

/*
 * We don't have a LexerException because;
 * 1 - The lexer should NEVER fail
 * 2 - The lexers job is non-volatile
 * 3 - We don't want abstract exceptions to reach the end user
 * (same with ParserException)
 */

class CompilationException
{
public:
    std::string message;

    CompilationException(const std::string &message)
        : message(message)
    {
    }

    const char *what() const throw()
    {
        return message.c_str();
    }
};

class VMException
{
public:
    std::string message;

    VMException(const std::string &message)
        : message(message)
    {
    }

    const char *what() const throw()
    {
        return message.c_str();
    }
};

} // namespace via
