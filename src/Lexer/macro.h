/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "token.h"

namespace via::Tokenization
{

struct Macro
{
    std::string name;                // Name of the macro
    std::vector<std::string> params; // Macro parameter names
    std::vector<Token> body;         // Macro body as a list of tokens
};

} // namespace via::Tokenization
