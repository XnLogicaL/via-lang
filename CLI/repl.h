/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "via.h"

namespace via
{

class REPLEngine
{
public:
    REPLEngine()
        : program("", "")                     // Initialize program data object with temporary data
        , gstate(new GState())                // Initialize global state
        , rtstate(new State(gstate, program)) // Initialize runtime state
    {
    }

    ~REPLEngine() {}

    void execute(const std::string &, bool);

public:
    ProgramData program;
    GState *gstate;
    State *rtstate;
    std::string stage;

private:
    void tokenize();
    void preprocess();
    void parse();
    void analyze();
    void compile();
    void interpret();
};

} // namespace via