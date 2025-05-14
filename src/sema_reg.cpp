// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "register.h"

namespace via {

register_t RegisterAllocator::allocate_register() {
  for (size_t i = 0; i < (REGISTER_COUNT - 1024); i++) {
    if (registers[i] == true) {
      registers[i] = false;
      return i;
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
  registers[reg] = true;
}

bool RegisterAllocator::is_used(register_t reg) {
  return !registers[reg];
}

size_t RegisterAllocator::get_used_registers() {
  return std::count_if(registers.begin(), registers.end(), [](auto& it) {
    return it.second == false;
  });
}

} // namespace via
