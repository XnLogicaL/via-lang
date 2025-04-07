//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================

#ifndef VIA_HAS_HEADER_STACK_H
#define VIA_HAS_HEADER_STACK_H

#include "ast.h"
#include "ast-base.h"
#include "instruction.h"
#include "common.h"

#define VIA_TSTACKSIZE 2048

namespace via {

using symbol_t = std::string;

struct variable_stack_obj {
  bool is_const = false;
  bool is_constexpr = false;
  symbol_t symbol;
  p_type_node_t type;
  p_expr_node_t value;
};

struct function_stack_obj {
  size_t stack_pointer = 0;
  func_stmt_node* func_stmt;
};

template<typename T>
class compiler_stack_base {
public:
  virtual ~compiler_stack_base() = default;

  // Returns the size of the stack.
  VIA_IMPLEMENTATION constexpr size_t size() const {
    return m_stack_pointer;
  }

  // Pushes a value onto the stack.
  VIA_IMPLEMENTATION void push(T&& val) {
    m_array[m_stack_pointer++] = std::move(val);
  }

  // Pops a value from the stack and returns it.
  VIA_IMPLEMENTATION T pop() {
    return std::move(m_array[m_stack_pointer--]);
  }

  // Returns the top element of the stack.
  VIA_IMPLEMENTATION T& top() {
    return m_array[m_stack_pointer - 1];
  }

  VIA_IMPLEMENTATION const T& top() const {
    return m_array[m_stack_pointer - 1];
  }

protected:
  size_t m_stack_pointer = 0;
  T m_array[VIA_TSTACKSIZE];
};

class compiler_variable_stack : public compiler_stack_base<variable_stack_obj> {
public:
  // Returns the stack object at a given index.
  std::optional<variable_stack_obj> at(size_t);

  // Returns the stack id of a given stack object.
  std::optional<operand_t> find_symbol(const variable_stack_obj&);
  std::optional<operand_t> find_symbol(const symbol_t&);
};

class compiler_function_stack : public compiler_stack_base<function_stack_obj> {};

} // namespace via

#endif
