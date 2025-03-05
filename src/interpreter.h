// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "via.h"

namespace via {

// Minimal interpreter implementation for quick usage without touching the API
class Interpreter {
public:
    Interpreter(ProgramData &program)
        : program(program)
        , gstate(new GState())
        , state(new State(gstate, program))
    {
        lib::open_baselib(state);
    }

    ~Interpreter()
    {
        delete gstate;
        delete state;
    }

    int execute(ProgramData);

public:
    ProgramData &program;
    GState      *gstate;
    State       *state;

private:
    void tokenize();
    bool preprocess();
    bool parse();
    bool compile();
    void interpret();
};

} // namespace via