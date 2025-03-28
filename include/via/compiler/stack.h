// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_STACK_H
#define _VIA_STACK_H

#include "ast.h"
#include "ast-base.h"
#include "instruction.h"
#include "common.h"

#define VIA_TEST_STACK_SIZE 2048

VIA_NAMESPACE_BEGIN

struct StackObject {
  bool is_const = false;
  bool is_constexpr = false;

  std::string symbol = "<anonymous-symbol>";

  pTypeNode type;
};

class CompilerStack final {
public:
  // Type aliases
  using index_query_result = std::optional<StackObject>;
  using find_query_result = std::optional<Operand>;

  using function_stack_node = FunctionNode::StackNode;
  using function_stack_type = std::stack<function_stack_node>;

  using symbol = std::string;

  // Constructor
  CompilerStack()
      : capacity(VIA_TEST_STACK_SIZE),
        sbp(new StackObject[capacity]) {}

  // Destructor
  ~CompilerStack() {
    delete[] sbp;
  }

  // Returns the size of the stack.
  size_t size();

  // Pushes a given stack object onto the stack.
  void push(StackObject);

  // Returns the top stack object of the stack.
  StackObject top();

  // Pops and returns a clone of the top-most stack object of the stack.
  StackObject pop();

  // Returns the stack object at a given index.
  index_query_result at(size_t);

  // Returns the stack id of a given stack object.
  find_query_result find_symbol(const StackObject&);
  find_query_result find_symbol(const symbol&);

public:
  function_stack_type function_stack;

private:
  size_t sp = 0;
  size_t capacity;

  StackObject* sbp;

private:
  // Dynamically grows and relocates the stack.
  void grow_stack();
};

VIA_NAMESPACE_END

#endif
