// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "register.h"

namespace via {

register_t RegisterAllocator::allocate_register() {
  for (register_t reg = 0; reg < REGISTER_COUNT - 1; reg++) {
    if (registers[reg]) {
      registers[reg] = false;
      return reg;
    }
  }

  return 0xFFFF;
}

register_t RegisterAllocator::allocate_temp() {
  register_t reg = allocate_register();
  free_register(reg);
  return reg;
}

void RegisterAllocator::free_register(register_t reg) {
  registers.emplace(reg, true);
}

bool RegisterAllocator::is_used(register_t reg) {
  return registers[reg];
}

} // namespace via
