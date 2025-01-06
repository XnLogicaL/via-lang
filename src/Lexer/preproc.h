/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include <vector>

#include "common.h"
#include "highlighter.h"
#include "token.h"
#include "def.h"
#include "import.h"
#include "macro.h"

#define PREPROCESSOR_ERROR(message) \
    { \
        emitter.out(pos, (message), Emitter::Severity::ERROR); \
        failed = true; \
    }

namespace via::Tokenization
{

class Preprocessor
{
public:
    ~Preprocessor() = default;
    Preprocessor(SrcContainer &container)
        : pos(0)
        , failed(false)
        , container(container)
        , emitter(container)
    {
    }

    bool preprocess();

private:
    size_t pos;
    bool failed;
    SrcContainer &container;
    std::unordered_map<std::string, Macro> macro_table;
    std::unordered_map<std::string, Definition> def_table;
    Emitter emitter;

private:
    Token consume(size_t ahead = 1);
    Token peek(int ahead = 0);

    Macro parse_macro();
    Definition parse_definition();
    void expand_macro(const Macro &);
    void expand_definition(const Definition &);
    void erase_from_stream(size_t, size_t);
};

} // namespace via::Tokenization
