/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "core.h"
#include "instruction.h"

static size_t _iota = std::numeric_limits<size_t>::max();

namespace via::Compilation
{

size_t iota()
{
    return _iota--;
}

class Generator
{
public:
    // Optimization stack
    std::stack<std::string> opt_stack;

    void pushinstr(viaInstruction instr);

    std::string compile();
    std::vector<viaInstruction> get();

    size_t get_available_register();
    void free_register(size_t offset);

private:
    std::vector<bool> registers;
    std::vector<viaInstruction> instrs;
};

} // namespace via::Compilation
