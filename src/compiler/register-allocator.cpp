// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "register-allocator.h"

namespace via {

register_t register_allocator::allocate_register() {
  for (register_t reg = 0; reg < 128; reg++) {
    if (registers[reg]) {
      registers[reg] = false;
      return reg;
    }
  }

  return 0xFFFF;
}

register_t register_allocator::allocate_temp() {
  register_t reg = allocate_register();
  free_register(reg);
  return reg;
}

void register_allocator::free_register(register_t reg) {
  registers.emplace(reg, true);
}

bool register_allocator::is_used(register_t reg) {
  return registers[reg];
}

} // namespace via
