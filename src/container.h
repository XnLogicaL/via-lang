/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "token.h"

namespace via::Tokenization
{

struct viaSourceContainer
{
    std::vector<Token> tokens;
    std::string source;
    std::string file_name;
};

} // namespace via::Tokenization
