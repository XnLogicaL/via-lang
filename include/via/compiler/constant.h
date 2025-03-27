// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_CONSTANT_H
#define _VIA_CONSTANT_H

#include "common.h"
#include "instruction.h"
#include "object.h"

// ===========================================================================================
// constant.h
//
VIA_NAMESPACE_BEGIN

class ConstantHolder final {
  public:
  // Type aliases
  using constant_type   = const TValue;
  using constant_vector = std::vector<TValue>;

  // Returns the size_t or next index of the constant table.
  size_t size() const;

  // Pushes a constant to the holder and returns the index of which the constant now lives.
  Operand push_constant(constant_type&);

  // Returns the constant at a given index.
  constant_type& at(size_t) const;
  constant_type& at_s(size_t) const; // If the index is invalid, returns nil.

  // Returns a reference to the constant table.
  const constant_vector& get() const;

  private:
  std::vector<TValue> constants;
};

VIA_NAMESPACE_END

#endif
