/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

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

    int execute(ProgramData &);

public:
    ProgramData &program;
    GState *gstate;
    State *state;

private:
    void tokenize();
    bool preprocess();
    bool parse();
    bool compile();
    void interpret();
};

} // namespace via