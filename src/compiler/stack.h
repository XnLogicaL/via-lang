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

struct StackVariable {
  bool is_const = false;
  bool is_constexpr = false;
  symbol_t symbol;
  StmtNodeBase* decl;
  TypeNodeBase* type;
  ExprNodeBase* value;
};

class CompilerVariableStack : public std::vector<StackVariable> {
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

class CompilerFunctionStack : public std::vector<StackFunction> {
public:
  void push_main_function(TransUnitContext& unit_ctx);
};

} // namespace via

#endif
