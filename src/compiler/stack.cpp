//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================
#include "stack.h"

namespace via {

std::optional<StackVariable*> CompilerVariableStack::get_local_by_id(size_t pos) {
  if (pos > size()) {
    return std::nullopt;
  }

  return &m_array[pos];
}

std::optional<StackVariable*> CompilerVariableStack::get_local_by_symbol(const symbol_t& symbol) {
  for (int i = m_stack_pointer - 1; i >= 0; --i) {
    if (m_array[i].symbol == symbol) {
      return &m_array[i];
    }
  }

  return std::nullopt;
}

std::optional<operand_t> CompilerVariableStack::find_local_id(const symbol_t& symbol) {
  for (int i = m_stack_pointer - 1; i >= 0; --i) {
    if (m_array[i].symbol == symbol) {
      return i;
    }
  }

  return std::nullopt;
}

} // namespace via
