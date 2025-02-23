// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GPL v3.           |
// =========================================================================================== |

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
        emitter.out(pos, (message), OutputSeverity::Error); \
        failed = true; \
    }

namespace via {

class Preprocessor {
public:
    ~Preprocessor() = default;
    Preprocessor(ProgramData *program)
        : program(program)
        , emitter(program)
    {
    }

    bool preprocess();
    void declare_default();
    void declare_macro(Macro);
    void declare_definition(Definition);

private:
    ProgramData *program;
    Emitter emitter;
    std::unordered_map<std::string, Macro> macro_table;
    std::unordered_map<std::string, Definition> def_table;
    size_t pos = 0;
    bool failed = false;

private:
    Token consume(size_t ahead = 1);
    Token peek(int ahead = 0);

    Macro parse_macro();
    Definition parse_definition();
    void expand_macro(const Macro &);
    void expand_definition(const Definition &);
    void erase_from_stream(size_t, size_t);
};

} // namespace via
