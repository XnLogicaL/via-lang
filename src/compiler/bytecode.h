// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_BYTECODE_H
#define VIA_HAS_HEADER_BYTECODE_H

#include <interpreter/instruction.h>

// ==========================================================================================
// bytecode.h
//
// This file declares the BytecodeHolder class.
//
// The BytecodeHolder class serves as an abstraction over a container
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

class BytecodeHolder final {
public:
  // Type aliases
  using comment_type = const std::string;
  using bytecode_vector = std::vector<Bytecode>;

  // Returns the current size of the bytecode pair vector.
  size_t size() const;

  // Inserts a given bytecode pair to the bytecode vectors back.
  void add(const Bytecode&);

  Bytecode& front();
  Bytecode& back();
  Bytecode& at(size_t);

  // Removes the bytecode pair located in a given index.
  void remove(size_t);

  // Returns a reference to the bytecode vector.
  inline const bytecode_vector& get() const {
    return instructions;
  }

private:
  bytecode_vector instructions;
};

} // namespace via

#endif
