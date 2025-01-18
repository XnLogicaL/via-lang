/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "Parser/ast.h"
#include "instruction.h"

namespace via
{

struct BytecodeHolder
{
    using Instructions = std::vector<Instruction>;

    BytecodeHolder() = default;
    Instructions instructions;

    void add_instruction(const Instruction &);
    void remove_instruction(size_t index);
    Instructions &get();
    const Instructions &get() const;
};

} // namespace via