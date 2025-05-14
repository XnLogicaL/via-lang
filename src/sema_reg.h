// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

/**
 * @file register.h
 * @brief Declares compile-time register semantics
 */
#ifndef VIA_HAS_HEADER_REGISTER_H
#define VIA_HAS_HEADER_REGISTER_H

#include "common.h"

#include <state.h>
#include <instruction.h>

/**
 * @namespace via
 * @ingroup via_namespace
 * @{
 */
namespace via {

/**
 * @typedef register_t operand_t
 * @brief Alias for operand type
 */
using register_t = operand_t;

/**
 * @class RegisterAllocator
 * @brief Handles compile-time register allocation and deallocation
 */
class RegisterAllocator final {
public:
  using register_map = std::unordered_map<register_t, bool>;

  inline explicit RegisterAllocator(size_t size, bool default_value) {
    registers.reserve(size);
    for (register_t reg = 0; reg < size - 1; reg++) {
      registers[reg] = default_value;
    }
  }

  /**
   * @brief Allocates and returns a register.
   * @return Register
   */
  register_t allocate_register();

  /**
   * @brief Returns a temporary register.
   * @return Register
   */
  register_t allocate_temp();

  /**
   * @brief Frees the given register
   * @param reg Register
   */
  void free_register(register_t reg);

  /**
   * @brief Returns whether if the given register is in use
   *
   * @param reg Register
   * @return Is used
   */
  bool is_used(register_t reg);

  /**
   * @brief Returns the amount of registers currently in-use
   * @return Count
   */
  size_t get_used_registers();

private:
  register_map registers;
};

} // namespace via

/** @} */

#endif
