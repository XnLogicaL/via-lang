// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

/**
 * @file stack.h
 * @brief Declares stack structures used by the compiler
 */
#ifndef VIA_HAS_HEADER_STACK_H
#define VIA_HAS_HEADER_STACK_H

#include "common.h"

#include <parse/ast-base.h>
#include <parse/ast.h>
#include <interpreter/instruction.h>

/**
 * @namespace via
 * @ingroup via_namespace
 * @{
 */
namespace via {

/**
 * @typedef symbol_t std::string
 * @brief Alias for std::string
 */
using symbol_t = std::string;

/**
 * @struct StackVariable
 * @brief Pure-data structure that represents a variable on the stack
 */
struct StackVariable {
  /// @brief Whether if the variable is constant/immutable
  bool is_const = false;
  /// @brief Whether if the variable is a constant expression
  bool is_constexpr = false;
  /// @brief Symbol of the variable
  symbol_t symbol;
  /// @brief Declaration of the variable, is DeclStmtNode/FuncDeclStmtNode
  StmtNodeBase* decl;
  /// @brief Type of the variable
  TypeNodeBase* type;
  /// @brief Value of the variable
  ExprNodeBase* value;
};

/**
 * @class CompilerVariableStack
 * @brief Holds variables in a stack structure manner, instantiated per closure
 */
class CompilerVariableStack : public std::vector<StackVariable> {
public:
  /**
   * @brief Retrieves a local variable by its index in the stack.
   * @param id The index of the variable.
   * @return An optional pointer to the StackVariable if found.
   */
  std::optional<StackVariable*> get_local_by_id(size_t id);

  /**
   * @brief Retrieves a local variable by its symbol name.
   * @param symbol The symbol name of the variable.
   * @return An optional pointer to the StackVariable if found.
   */
  std::optional<StackVariable*> get_local_by_symbol(const symbol_t& symbol);

  /**
   * @brief Finds the index of a variable in the stack by its symbol.
   * @param symbol The symbol name of the variable.
   * @return An optional operand_t representing the index if found.
   */
  std::optional<operand_t> find_local_id(const symbol_t& symbol);

  /**
   * @brief Restores the variable stack to a given stack pointer.
   * @param sp The stack pointer to restore to (number of variables to retain).
   */
  void restore_stack_pointer(size_t sp);
};

/**
 * @struct StackFunction
 * @brief Represents a function on the stack with its own local variable stack.
 */
struct StackFunction {
  /// @brief Stack pointer value before entering the function.
  size_t stack_pointer = 0;
  /// @brief Function declaration associated with the stack frame.
  FuncDeclStmtNode* decl;
  /// @brief Local variables declared within the function.
  CompilerVariableStack locals;
};

/**
 * @class CompilerFunctionStack
 * @brief Stack of active functions used during compilation.
 */
class CompilerFunctionStack : public std::vector<StackFunction> {
public:
  /**
   * @brief Pushes the main function context into the function stack.
   * @param unit_ctx The translation unit context representing the main function scope.
   */
  void push_main_function(TransUnitContext& unit_ctx);
};

} // namespace via

/** @} */

#endif
