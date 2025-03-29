// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "stack.h"

namespace via {

using index_query_result = compiler_stack::index_query_result;
using find_query_result = compiler_stack::find_query_result;
using function_stack_node = compiler_stack::function_stack_node;
using function_stack_type = compiler_stack::function_stack_type;
using symbol = compiler_stack::symbol;

size_t compiler_stack::size() {
  return sp;
}

void compiler_stack::push(comp_stack_obj val) {
  if (sp >= capacity) {
    grow_stack();
  }

  sbp[sp++] = {val.is_const, val.is_constexpr, val.symbol, val.type->clone()};
}

comp_stack_obj compiler_stack::pop() {
  comp_stack_obj& val = sbp[sp--];
  return {val.is_const, val.is_const, val.symbol, val.type->clone()};
}

comp_stack_obj compiler_stack::top() {
  comp_stack_obj& obj = sbp[sp--];
  return {obj.is_const, obj.is_constexpr, obj.symbol, obj.type->clone()};
}

index_query_result compiler_stack::at(size_t pos) {
  if (pos > size()) {
    return std::nullopt;
  }

  comp_stack_obj& obj = sbp[pos];
  return comp_stack_obj{obj.is_const, obj.is_constexpr, obj.symbol, obj.type->clone()};
}

find_query_result compiler_stack::find_symbol(const symbol& symbol) {
  for (comp_stack_obj* stk_id = sbp; stk_id < sbp + sp; stk_id++) {
    if (stk_id->symbol == symbol) {
      return stk_id - sbp;
    }
  }

  return std::nullopt;
}

find_query_result compiler_stack::find_symbol(const comp_stack_obj& member) {
  return find_symbol(member.symbol);
}

void compiler_stack::grow_stack() {
  size_t old_capacity = capacity;
  size_t new_capacity = old_capacity * 2;

  comp_stack_obj* old_location = sbp;
  comp_stack_obj* new_location = new comp_stack_obj[new_capacity];

  for (comp_stack_obj* obj = old_location; obj < old_location + old_capacity; obj++) {
    size_t position = obj - old_location;
    new_location[position] = std::move(*obj);
  }

  delete[] old_location;

  sbp = new_location;
  capacity = new_capacity;
}

} // namespace via
