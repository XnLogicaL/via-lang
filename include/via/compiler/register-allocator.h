// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_REGISTER_ALLOCATOR_H
#define _VIA_REGISTER_ALLOCATOR_H

#include "common.h"
#include "instruction.h"

VIA_NAMESPACE_BEGIN

class RegisterAllocator final {
public:
  // Type aliases
  using register_type = Operand;
  using register_map  = std::unordered_map<register_type, bool>;

  // Constructor
  RegisterAllocator(size_t size, bool default_value) {
    registers.reserve(size);
    for (register_type reg = 0; reg < size; reg++) {
      registers.emplace(reg, default_value);
    }
  }

  // Returns a newly allocated register.
  register_type allocate_register();

  // Returns a temporary, non-allocated register.
  register_type allocate_temp();

  // Frees a given register.
  void free_register(register_type reg);

  // Returns wheter if a given register is used.
  bool is_used(register_type reg);

private:
  register_map registers;
};

VIA_NAMESPACE_END

#endif
