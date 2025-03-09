// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "bytecode.h"
#include "instruction.h"
#include "common.h"

// ===========================================================================================
// bytecode.cpp
//
VIA_NAMESPACE_BEGIN

void BytecodeHolder::add(const Bytecode& bytecode) {
    instructions.push_back(bytecode);
}

SIZE BytecodeHolder::size() const noexcept {
    return instructions.size();
}

void BytecodeHolder::remove(U64 index) {
    instructions.erase(instructions.begin() + index);
}

void BytecodeHolder::insert(
    U64 index, OpCode opcode, const std::array<Operand, 3>& operands, const std::string& comment
) {
    // Insert the instruction at the specified index
    instructions.insert(
        instructions.begin() + index,
        {
            .instruction =
                {
                    .op       = opcode,
                    .operand0 = operands.at(0),
                    .operand1 = operands.at(1),
                    .operand2 = operands.at(2),
                },
            .meta_data =
                {
                    .chunk   = nullptr,
                    .comment = comment,
                },
        }
    );
}

void BytecodeHolder::emit(
    OpCode opcode, const std::array<Operand, 3>& operands, const std::string& comment
) {
    add({
        .instruction =
            {
                .op       = opcode,
                .operand0 = operands.at(0),
                .operand1 = operands.at(1),
                .operand2 = operands.at(2),
            },
        .meta_data =
            {
                .chunk   = nullptr,
                .comment = comment,
            },
    });
}

const std::vector<Bytecode>& BytecodeHolder::get() const noexcept {
    return instructions;
}

VIA_NAMESPACE_END
