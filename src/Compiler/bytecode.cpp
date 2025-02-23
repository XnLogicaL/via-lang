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

void BytecodeHolder::emit(
    OpCode opcode,
    const std::vector<U32> &operands,
    const std::string &comment
)
{
    const std::vector<Instruction> &instructions = program->bytecode->get();
    const size_t instructions_size = instructions.size();
    program->bytecode->add_instruction({opcode, operands, nullptr, instructions_size});
    if (comment != "") {
        program->bytecode_info[instructions_size] = comment;
    }
}

std::vector<Instruction> &BytecodeHolder::get()
{
    return instructions;
}

} // namespace via