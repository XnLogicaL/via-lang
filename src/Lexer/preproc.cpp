/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "preproc.h"

using namespace via::Tokenization;
using namespace Preprocessing;

void Preprocessor::preprocess()
{
    size_t pos = 0;

    for (const auto &tok : *toks)
    {
        if (tok.type == TokenType::KW_MACRO)
        {
            Macro mac = parse_macro(toks, pos);
            expand_macro(toks, mac);
        }

        pos++;
    }
}