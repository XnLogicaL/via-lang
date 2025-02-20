// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

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

} // namespace via