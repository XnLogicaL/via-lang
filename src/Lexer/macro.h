/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <format>

#include "common.h"
#include "token.h"
#include "except.h"

namespace via::Tokenization::Preprocessing
{

struct Macro
{
    std::string name;                // Name of the macro
    std::vector<std::string> params; // Macro parameter names
    std::vector<Token> body;         // Macro body as a list of tokens
};

Macro parse_macro(std::vector<Token> *toks, size_t &pos);
void expand_macro(std::vector<Token> *toks, const Macro &macro);

} // namespace via::Tokenization::Preprocessing
