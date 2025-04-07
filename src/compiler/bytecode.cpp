//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================

#include "bytecode.h"
#include "instruction.h"
#include "common.h"

// ===========================================================================================
// bytecode.cpp
//
namespace via {

using comment_type = bytecode_holder::comment_type;
using bytecode_vector = bytecode_holder::bytecode_vector;

size_t bytecode_holder::size() const {
  return instructions.size();
}

bytecode& bytecode_holder::front() {
  return instructions.front();
}

bytecode& bytecode_holder::back() {
  return instructions.back();
}

bytecode& bytecode_holder::at(size_t pos) {
  return instructions.at(pos);
}

void bytecode_holder::add(const bytecode& bytecode) {
  instructions.push_back(bytecode);
}

void bytecode_holder::remove(size_t index) {
  instructions.erase(instructions.begin() + index);
}

void bytecode_holder::insert(
  size_t index,
  opcode opcode,
  operands_array<operand_t, 3, VIA_OPERAND_INVALID>&& operands,
  comment_type& comment
) {
  // Insert the instruction at the specified index
  instructions.insert(
    instructions.begin() + index,
    {
      .instruct =
        {
          .op = opcode,
          .operand0 = operands.data.at(0),
          .operand1 = operands.data.at(1),
          .operand2 = operands.data.at(2),
        },
      .meta_data =
        {
          .comment = comment,
        },
    }
  );
}

void bytecode_holder::emit(
  opcode opcode, operands_array<operand_t, 3, VIA_OPERAND_INVALID>&& operands, comment_type& comment
) {
  add({
    .instruct =
      {
        .op = opcode,
        .operand0 = operands.data.at(0),
        .operand1 = operands.data.at(1),
        .operand2 = operands.data.at(2),
      },
    .meta_data =
      {
        .comment = comment,
      },
  });
}

const bytecode_vector& bytecode_holder::get() const {
  return instructions;
}

} // namespace via
