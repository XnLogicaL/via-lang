/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "bytecode.h"

namespace via::Compilation
{

// Add an instruction to the container
void Bytecode::add_instruction(const Instruction &instruction)
{
    instructions.push_back(instruction);
}

// Remove instruction at <index> from the container
void Bytecode::remove_instruction(size_t index)
{
    instructions.erase(instructions.begin() + index);
}

// Return the instruction container
Bytecode::Instructions_t &Bytecode::get()
{
    return instructions;
}

// Return the constant instruction container if applicable
const Bytecode::Instructions_t &Bytecode::get() const
{
    return instructions;
}

} // namespace via::Compilation