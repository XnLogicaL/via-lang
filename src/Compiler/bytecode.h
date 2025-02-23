// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#pragma once

#include "instruction.h"

namespace via {

class BytecodeHolder {
public:
    BytecodeHolder(ProgramData *program)
        : program(program)
    {
    }

    void add_instruction(const Instruction &);
    void remove_instruction(size_t index);
    void emit(
        OpCode opcode = OpCode::NOP,
        const std::vector<U32> &operands = {},
        const std::string &comment = ""
    );

    std::vector<Instruction> &get();

private:
    ProgramData *program;
    std::vector<Instruction> instructions;
};

} // namespace via