// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "bytecode.h"
#include "instruction.h"
#include "opcode.h"

namespace via {

class Encoder {
public:
    std::vector<char> encode(std::vector<Instruction>);
    std::vector<Instruction> decode(std::vector<char>);

private:
    char encode_opcode(OpCode);
    std::vector<char> encode_operand(Operand);
    OpCode decode_opcode(char);
    Operand decode_operand(std::vector<char>::const_iterator);
};

} // namespace via