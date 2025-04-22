//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================

#ifndef VIA_HAS_HEADER_STACK_H
#define VIA_HAS_HEADER_STACK_H

#include "common.h"

#include <parse/ast-base.h>
#include <parse/ast.h>
#include <interpreter/instruction.h>

namespace via {

inline constexpr size_t COMPILER_STACK_SIZE = 200;

using symbol_t = std::string;

template<typename T>
class CompilerStackBase {
public:
  virtual ~CompilerStackBase() = default;

  // Returns the size of the stack.
  inline constexpr size_t size() const {
    return m_stack_pointer;
  }

  // Pushes a value onto the stack.
  inline void push(T&& val) {
    m_array[m_stack_pointer++] = std::move(val);
  }

  // Pops a value from the stack and returns it.
  inline T pop() {
    return std::move(m_array[m_stack_pointer--]);
  }

  // Returns the top element of the stack.
  inline T& top() {
    return m_array[m_stack_pointer - 1];
  }

  inline const T& top() const {
    return m_array[m_stack_pointer - 1];
  }

  inline T& at(size_t sp) {
    return m_array[sp];
  }

  inline void jump_to(size_t sp) {
    m_stack_pointer = sp;
  }

  inline T* begin() {
    return m_array;
  }

  inline T* end() {
    return m_array + m_stack_pointer + 1;
  }

protected:
  size_t m_stack_pointer = 0;
  T* m_array = new T[COMPILER_STACK_SIZE];
};

struct StackVariable {
  bool is_const = false;
  bool is_constexpr = false;
  symbol_t symbol;
  StmtNodeBase* decl;
  TypeNodeBase* type;
  ExprNodeBase* value;
};

class CompilerVariableStack : public CompilerStackBase<StackVariable> {
public:
  // Returns the stack object at a given index.
  std::optional<StackVariable*> get_local_by_id(size_t);

  // Returns the first stack object that holds the given symbol.
  std::optional<StackVariable*> get_local_by_symbol(const symbol_t&);

  // Returns the stack id of a given stack object.
  std::optional<operand_t> find_local_id(const symbol_t&);
};

struct StackFunction {
  size_t stack_pointer = 0;
  FuncDeclStmtNode* decl;
  CompilerVariableStack locals;
};

class CompilerFunctionStack : public CompilerStackBase<StackFunction> {
public:
  void push_main_function(TransUnitContext& unit_ctx);
};

} // namespace via

#endif
