/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "bytecode.h"
#include "instruction.h"
#include "common.h"

namespace via {

// Add an instruction to the program
void BytecodeHolder::add_instruction(const Instruction &instruction)
{
    instructions.push_back(instruction);
}

// Remove instruction at <index> from the program
void BytecodeHolder::remove_instruction(size_t index)
{
    instructions.erase(instructions.begin() + index);
}

// Return the instruction program
BytecodeHolder::Instructions &BytecodeHolder::get()
{
    return instructions;
}

// Return the constant instruction program if applicable
const BytecodeHolder::Instructions &BytecodeHolder::get() const
{
    return instructions;
}

} // namespace via