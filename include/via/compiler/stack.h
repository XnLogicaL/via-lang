// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _vl_stack_h
#define _vl_stack_h

#include "ast.h"
#include "ast-base.h"
#include "instruction.h"
#include "common.h"

#define vl_tstacksize 2048

namespace via {

using symbol_t = std::string;

struct comp_stack_obj {
  bool is_const = false;
  bool is_constexpr = false;

  std::string symbol = "<anonymous-symbol>";

  p_type_node_t type;
};

class compiler_stack final {
public:
  // Type aliases
  using index_query_result = std::optional<comp_stack_obj>;
  using find_query_result = std::optional<operand_t>;

  using function_stack_node = func_stmt_node::stack_node;
  using function_stack_type = std::stack<function_stack_node>;

  // Constructor
  compiler_stack()
    : capacity(vl_tstacksize),
      sbp(new comp_stack_obj[capacity]) {}

  // Destructor
  ~compiler_stack() {
    delete[] sbp;
  }

  // Returns the size of the stack.
  size_t size();

  // Pushes a given stack object onto the stack.
  void push(comp_stack_obj);

  // Returns the top stack object of the stack.
  comp_stack_obj top();

  // Pops and returns a clone of the top-most stack object of the stack.
  comp_stack_obj pop();

  // Returns the stack object at a given index.
  index_query_result at(size_t);

  // Returns the stack id of a given stack object.
  find_query_result find_symbol(const comp_stack_obj&);
  find_query_result find_symbol(const symbol_t&);

public:
  function_stack_type function_stack;

private:
  size_t sp = 0;
  size_t capacity;

  comp_stack_obj* sbp;

private:
  // Dynamically grows and relocates the stack.
  void grow_stack();
};

} // namespace via

#endif
