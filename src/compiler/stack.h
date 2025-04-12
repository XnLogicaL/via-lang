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

template<typename T>
class CompilerStackBase {
public:
  virtual ~CompilerStackBase() = default;

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

  VIA_IMPLEMENTATION T& at(size_t sp) {
    return m_array[sp];
  }

  VIA_IMPLEMENTATION void jump_to(size_t sp) {
    m_stack_pointer = sp;
  }

  VIA_IMPLEMENTATION T* begin() {
    return m_array;
  }

  VIA_IMPLEMENTATION T* end() {
    return m_array + m_stack_pointer + 1;
  }

protected:
  size_t m_stack_pointer = 0;
  T m_array[VIA_TSTACKSIZE];
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
  VIA_IMPLEMENTATION void push_main_function(TransUnitContext& unit_ctx) {
    ScopeStmtNode* scope = unit_ctx.ast->allocator.emplace<ScopeStmtNode>(
      size_t(0), size_t(0), std::vector<StmtNodeBase*>{}
    );

    PrimTypeNode* ret = unit_ctx.ast->allocator.emplace<PrimTypeNode>(
      Token(TokenType::IDENTIFIER, "nil", 0, 0, 0), IValueType::nil
    );

    FuncDeclStmtNode* func = unit_ctx.ast->allocator.emplace<FuncDeclStmtNode>(
      size_t(0),
      size_t(0),
      false,
      StmtModifiers{},
      Token(TokenType::IDENTIFIER, "main", 0, 0, 0),
      scope,
      ret,
      std::vector<ParamStmtNode>{}
    );

    push({
      .stack_pointer = 0,
      .decl = func,
      .locals = {},
    });
  }
};

} // namespace via

#endif
