// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef vl_has_header_stack_h
#define vl_has_header_stack_h

#include "ast.h"
#include "ast-base.h"
#include "instruction.h"
#include "common.h"

#define vl_tstacksize 2048

namespace via {

using symbol_t = std::string;

struct variable_stack_obj {
  bool is_const = false;
  bool is_constexpr = false;
  symbol_t symbol;
  p_type_node_t type;
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
  vl_implement constexpr size_t size() const {
    return m_stack_pointer;
  }

  // Pushes a value onto the stack.
  vl_implement void push(T&& val) {
    m_array[m_stack_pointer++] = std::move(val);
  }

  // Pops a value from the stack and returns it.
  vl_implement T pop() {
    return std::move(m_array[m_stack_pointer--]);
  }

  // Returns the top element of the stack.
  vl_implement T& top() {
    return m_array[m_stack_pointer - 1];
  }

  vl_implement const T& top() const {
    return m_array[m_stack_pointer - 1];
  }

protected:
  size_t m_stack_pointer = 0;
  T m_array[vl_tstacksize];
};

class variable_stack : public compiler_stack_base<variable_stack_obj> {
public:
  // Returns the stack object at a given index.
  std::optional<variable_stack_obj> at(size_t);

  // Returns the stack id of a given stack object.
  std::optional<operand_t> find_symbol(const variable_stack_obj&);
  std::optional<operand_t> find_symbol(const symbol_t&);
};

class function_stack : public compiler_stack_base<function_stack_obj> {};

} // namespace via

#endif
