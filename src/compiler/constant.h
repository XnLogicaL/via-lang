// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_CONSTANT_H
#define VIA_HAS_HEADER_CONSTANT_H

#include "common.h"
#include "instruction.h"
#include "tvalue.h"

// ===========================================================================================
// constant.h
//
namespace via {

class ConstantHolder final {
public:
  // Type aliases
  using constant_type = const Value&&;
  using constant_vector = std::vector<Value>;

  // Returns the size_t or next index of the constant table.
  size_t size() const;

  // Pushes a constant to the holder and returns the index of which the constant now lives.
  operand_t push_constant(constant_type);

  // Returns the constant at a given index.
  constant_type& at(size_t) const;
  constant_type& at_s(size_t) const; // If the index is invalid, returns Nil.

  // Returns a reference to the constant table.
  const constant_vector& get() const;

private:
  std::vector<Value> constants;
};

} // namespace via

#endif
