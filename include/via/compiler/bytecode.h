//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================

#ifndef VIA_HAS_HEADER_BYTECODE_H
#define VIA_HAS_HEADER_BYTECODE_H

#include "instruction.h"

// ==========================================================================================
// bytecode.h
//
// This file declares the bytecode_holder class.
//
// The bytecode_holder class serves as an abstraction over a container
//  (such as std::vector), providing methods like `add`,
//  `remove` and `emit`. The main reason behind it's
//  existence is to provide a more "fit-for-duty" interface for
//  interfacing with a bytecode array, specifically in the context of compilation.
//
// The `emit` method (the most important method) is used to emit an
//  instruction *without* actually constructing it on the spot,
//  but rather constructing it inside the method, and is widely used
//  inside the current compiler implementation.
//
// `emit` also provides the caller with an option to add a comment, which
//  is in turn saved to ProgramData::bytecode_info.
namespace via {

// std::array wrapper with custom initialization support
template<typename T, const size_t Size, const T Default>
struct operands_array {
  std::array<T, Size> data;

  constexpr operands_array() {
    data.fill(Default);
  }

  constexpr operands_array(std::initializer_list<T> init) {
    data.fill(Default);
    std::copy(init.begin(), init.end(), data.begin());
  }

  operator std::array<T, Size>&() {
    return data;
  }

  operator const std::array<T, Size>&() const {
    return data;
  }
};

class bytecode_holder final {
public:
  // Type aliases
  using comment_type = const std::string;
  using bytecode_vector = std::vector<bytecode>;

  // Returns the current size of the bytecode pair vector.
  size_t size() const;

  // Inserts a given bytecode pair to the bytecode vectors back.
  void add(const bytecode&);

  bytecode& front();
  bytecode& back();
  bytecode& at(size_t);

  // Removes the bytecode pair located in a given index.
  void remove(size_t);

  // Inserts a locally constructed instruction to a given index.
  void insert(
    size_t index = 0,
    opcode opcode = opcode::NOP,
    operands_array<operand_t, 3, VIA_OPERAND_INVALID>&& operands = {},
    comment_type& comment = ""
  );

  // Emits an instruction at the end of the vector.
  void emit(
    opcode opcode = opcode::NOP,
    operands_array<operand_t, 3, VIA_OPERAND_INVALID>&& operands = {},
    comment_type& comment = ""
  );

  // Returns a reference to the bytecode vector.
  const bytecode_vector& get() const;

private:
  bytecode_vector instructions;
};

} // namespace via

#endif
