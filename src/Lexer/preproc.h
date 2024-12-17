/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include <vector>

#include "common.h"
#include "token.h"
#include "def.h"
#include "import.h"
#include "macro.h"

namespace via::Tokenization::Preprocessing
{

class Preprocessor
{
public:
    Preprocessor(std::vector<Token> *toks)
        : toks(toks)
    {
    }

    void preprocess();

private:
    std::vector<Token> *toks;
};

} // namespace via::Tokenization::Preprocessing
