//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================
#include "stack.h"

namespace via {

std::optional<StackVariable> CompilerVariableStack::at(size_t pos) {
  if (pos > size()) {
    return std::nullopt;
  }

  StackVariable& obj = m_array[pos];
  StackVariable new_obj = {
    .is_const = false,
    .is_constexpr = false,
    .symbol = obj.symbol,
    .decl = obj.decl,
    .type = obj.type,
    .value = obj.value,
  };

  return new_obj;
}

std::optional<operand_t> CompilerVariableStack::find_symbol(const symbol_t& symbol) {
  for (int i = m_stack_pointer - 1; i >= 0; --i) {
    if (m_array[i].symbol == symbol) {
      return i;
    }
  }
  return std::nullopt;
}

std::optional<operand_t> CompilerVariableStack::find_symbol(const StackVariable& member) {
  return find_symbol(member.symbol);
}

} // namespace via
