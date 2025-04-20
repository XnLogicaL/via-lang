// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "bytecode.h"

// ===========================================================================================
// bytecode.cpp
//
namespace via {

using comment_type = BytecodeHolder::comment_type;
using bytecode_vector = BytecodeHolder::bytecode_vector;

size_t BytecodeHolder::size() const {
  return instructions.size();
}

Bytecode& BytecodeHolder::front() {
  return instructions.front();
}

Bytecode& BytecodeHolder::back() {
  return instructions.back();
}

Bytecode& BytecodeHolder::at(size_t pos) {
  return instructions.at(pos);
}

void BytecodeHolder::add(const Bytecode& bytecode) {
  instructions.push_back(bytecode);
}

void BytecodeHolder::remove(size_t index) {
  instructions.erase(instructions.begin() + index);
}

} // namespace via
