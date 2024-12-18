/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "token.h"

namespace via::Tokenization
{

struct Definition
{
    std::string identifier;
    std::vector<Token> replacement;
};

} // namespace via::Tokenization