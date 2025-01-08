/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "Parser/ast.h"

namespace via
{

struct Instruction;
class BytecodeHolder
{
public:
    BytecodeHolder() = default;

    std::vector<Instruction> instructions;

    void add_instruction(const Instruction &);
    void remove_instruction(size_t index);
    std::vector<Instruction> &get();
    const std::vector<Instruction> &get() const;
};

} // namespace via