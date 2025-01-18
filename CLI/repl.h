/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "via.h"

namespace via
{

class REPLEngine
{
public:
    REPLEngine()
        : program("", "")       // Initialize program data object with temporary data
        , gstate(stnewgstate()) // Initialize global state
        , rtstate(nullptr)      // Initialize runtime state
    {
    }

    ~REPLEngine() {}

    void execute(const std::string &, bool);

private:
    ProgramData program;
    GState *gstate;
    RTState *rtstate;
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