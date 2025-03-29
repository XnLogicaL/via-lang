// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef vl_has_header_register_allocator_h
#define vl_has_header_register_allocator_h

#include "common.h"
#include "instruction.h"

namespace via {

using register_t = operand_t;

class register_allocator final {
public:
  // Type aliases
  using register_map = std::unordered_map<register_t, bool>;

  // Constructor
  register_allocator(size_t size, bool default_value) {
    registers.reserve(size);
    for (register_t reg = 0; reg < size; reg++) {
      registers.emplace(reg, default_value);
    }
  }

  // Returns a newly allocated register.
  register_t allocate_register();

  // Returns a temporary, non-allocated register.
  register_t allocate_temp();

  // Frees a given register.
  void free_register(register_t reg);

  // Returns wheter if a given register is used.
  bool is_used(register_t reg);

private:
  register_map registers;
};

} // namespace via

#endif
