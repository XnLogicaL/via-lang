/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "via.h"

namespace viaCLI
{

class REPLEngine
{
public:
    REPLEngine()
        : V(nullptr){};
    ~REPLEngine() = default;

    void execute(std::string, bool print);
    via::viaState *V;

private:
    std::vector<via::Instruction> compile(std::string);
};

} // namespace viaCLI