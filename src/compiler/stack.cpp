// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "stack.h"

namespace via {

std::optional<variable_stack_obj> variable_stack::at(size_t pos) {
  if (pos > size()) {
    return std::nullopt;
  }

  variable_stack_obj& obj = m_array[pos];
  variable_stack_obj new_obj = {
    .is_const = false,
    .is_constexpr = false,
    .symbol = obj.symbol,
    .type = obj.type->clone(),
  };

  return new_obj;
}

std::optional<operand_t> variable_stack::find_symbol(const symbol_t& symbol) {
  for (variable_stack_obj* stk_id = m_array; stk_id < m_array + m_stack_pointer; stk_id++) {
    if (stk_id->symbol == symbol) {
      return stk_id - m_array;
    }
  }

  return std::nullopt;
}

std::optional<operand_t> variable_stack::find_symbol(const variable_stack_obj& member) {
  return find_symbol(member.symbol);
}

} // namespace via
