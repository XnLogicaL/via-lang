// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#pragma once

#include "instruction.h"

namespace via {

struct BytecodeHolder {
    using Instructions = std::vector<Instruction>;

    Instructions instructions;

    void add_instruction(const Instruction &);
    void remove_instruction(size_t index);
};

} // namespace via