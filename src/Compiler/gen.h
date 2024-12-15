/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "core.h"
#include "instruction.h"

namespace via::Compilation
{

class Generator
{
public:
    // Optimization stack
    std::stack<std::string> opt_stack;

    void pushinstr(viaInstruction);
    std::string compile();
    std::vector<viaInstruction> get();
    size_t get_available_register();
    void free_register(size_t);
    size_t uuid();

    Generator() {}
    ~Generator()
    {
        for (void *p : free_list)
            std::free(p);
    }

private:
    std::vector<bool> registers;
    std::vector<viaInstruction> instrs;
    std::vector<void *> free_list;
    size_t uuid_head;
};

} // namespace via::Compilation
