/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "via.h"

namespace viaCLI
{

class REPL
{
public:
    REPL()
        : V(nullptr){};
    ~REPL() = default;

    void execute(std::string);

private:
    via::viaState *V;

private:
    std::vector<via::viaInstruction> compile(std::string);
};

} // namespace viaCLI