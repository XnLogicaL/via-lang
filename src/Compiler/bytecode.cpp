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

using comment_type    = BytecodeHolder::comment_type;
using operands_array  = BytecodeHolder::operands_array;
using bytecode_vector = BytecodeHolder::bytecode_vector;

void BytecodeHolder::add(const Bytecode& bytecode) {
    instructions.push_back(bytecode);
}

size_t BytecodeHolder::size() const noexcept {
    return instructions.size();
}

void BytecodeHolder::remove(size_t index) {
    instructions.erase(instructions.begin() + index);
}

void BytecodeHolder::insert(
    size_t index, OpCode opcode, operands_array& operands, comment_type& comment
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

void BytecodeHolder::emit(OpCode opcode, operands_array& operands, comment_type& comment) {
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

const bytecode_vector& BytecodeHolder::get() const noexcept {
    return instructions;
}

VIA_NAMESPACE_END
