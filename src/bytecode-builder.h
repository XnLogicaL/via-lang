// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

/**
 * @file bytecode-builder.h
 * @brief Declares the BytecodeBuilder class along with utility
 */
#ifndef VIA_HAS_HEADER_COMPILER_H
#define VIA_HAS_HEADER_COMPILER_H

#include "common.h"

#include <ast.h>
#include <visitor.h>
#include <types.h>
#include <instruction.h>
#include <state.h>

/**
 * @namespace via
 * @ingroup via_namespace
 * @{
 */
namespace via {

/**
 * @namespace compiler_util
 * @brief Contains compiler utility functions.
 * @defgroup compiler_util_namespace
 * @{
 */
namespace compiler_util {

/**
 * @class OperandsArray
 * @brief std::array wrapper with custom initialization support
 */
template<typename T, const size_t Size, const T Default>
struct OperandsArray {
  std::array<T, Size> data;

  constexpr OperandsArray() {
    data.fill(Default);
  }

  constexpr OperandsArray(std::initializer_list<T> init) {
    data.fill(Default);
    std::copy(init.begin(), init.end(), data.begin());
  }

  operator std::array<T, Size>&() {
    return data;
  }

  operator const std::array<T, Size>&() const {
    return data;
  }
};

/**
 * @brief Emits a compiler error with a highlighted source range.
 * @param ctx Visitor context
 * @param begin Start offset of source
 * @param end End offset of source
 * @param message Error message
 */
void compiler_error(VisitorContext& ctx, size_t begin, size_t end, const std::string& message);

/**
 * @brief Emits a compiler error associated with a specific token.
 * @param ctx Visitor context
 * @param token Token associated with the error
 * @param message Error message
 */
void compiler_error(VisitorContext& ctx, const Token& token, const std::string& message);

/**
 * @brief Emits a general compiler error without location info.
 * @param ctx Visitor context
 * @param message Error message
 */
void compiler_error(VisitorContext& ctx, const std::string& message);

/**
 * @brief Emits a compiler warning with a highlighted source range.
 * @param ctx Visitor context
 * @param begin Start offset of source
 * @param end End offset of source
 * @param message Warning message
 */
void compiler_warning(VisitorContext& ctx, size_t begin, size_t end, const std::string&);

/**
 * @brief Emits a compiler warning associated with a specific token.
 * @param ctx Visitor context
 * @param token Token associated with the warning
 * @param message Warning message
 */
void compiler_warning(VisitorContext& ctx, const Token&, const std::string&);

/**
 * @brief Emits a general compiler info without location info.
 * @param ctx Visitor context
 * @param message Info message
 */
void compiler_warning(VisitorContext& ctx, const std::string&);

/**
 * @brief Emits a compiler info with a highlighted source range.
 * @param ctx Visitor context
 * @param begin Start offset of source
 * @param end End offset of source
 * @param message Info message
 */
void compiler_info(VisitorContext& ctx, size_t begin, size_t end, const std::string&);

/**
 * @brief Emits a compiler info associated with a specific token.
 * @param ctx Visitor context
 * @param token Token associated with the info
 * @param message Error message
 */
void compiler_info(VisitorContext& ctx, const Token&, const std::string&);

/**
 * @brief Emits a general compiler info without location info.
 * @param ctx Visitor context
 * @param message Error info
 */
void compiler_info(VisitorContext& ctx, const std::string&);

/**
 * @brief Signifies the end of compiler output. Currently does nothing
 * @param ctx Visitor context
 */
void compiler_output_end(VisitorContext& ctx);

/**
 * @brief Returns the top-most closure on the function stack.
 * @param ctx Visitor context
 * @return Reference to top-most closure on the stack.
 */
StackFunction& get_current_closure(VisitorContext& ctx);

/**
 * @brief Constructs a constant value from the given literal expression node.
 * @param literal_node The literal node of which the constant value is generated from
 * @return Interpreter value
 */
Value construct_constant(NodeLitExpr& constant);

/**
 * @brief Folds an expression into a constant if possible.
 * @param ctx Visitor context
 * @param expr The expression to fold
 * @param fold_depth The maximum fold depth for symbolic expression
 * @return Literal expression node
 */
NodeLitExpr fold_constant(VisitorContext& ctx, ExprNode* constant, size_t fold_depth = 0);

/**
 * @brief Pushes a constant onto the constant table.
 * @param ctx Visitor context
 * @param constant Constant to push onto the constant table
 * @return Constant id
 */
operand_t push_constant(VisitorContext& ctx, Value&& constant);

/**
 * @brief Resolves the given lvalue and loads the result into the given register.
 * @param ctx Visitor context
 * @param lvalue lvalue expression; NodeSymExpr, NodeIndexExpr, etc.
 * @param dst Destination register
 * @return Fail status
 */
bool resolve_lvalue(VisitorContext& ctx, ExprNode* lvalue, operand_t dst);

/**
 * @brief Resolves the given rvalue and loads the result into the given register.
 * @param visitor Visitor object
 * @param rvalue rvalue expression
 * @param dst Destination register
 * @return Fail status
 */
bool resolve_rvalue(NodeVisitorBase* visitor, ExprNode* rvalue, operand_t dst);

/**
 * @brief Binds the value in the given register to the given lvalue.
 * @param ctx Visitor context
 * @param lvalue lvalue to bind to
 * @param src Register to the get the value to bind
 * @return Fail status
 */
bool bind_lvalue(VisitorContext& ctx, ExprNode* lvalue, operand_t src);

/**
 * @brief Resolves the type of the given expression
 * @param ctx Visitor context
 * @param expr Expression to resolve type of
 * @return Resolved type, current can hold the value of nullptr because the type system is
 * incomplete
 */
TypeNode* resolve_type(VisitorContext& ctx, ExprNode* expr);

/**
 * @typedef operand_init_t OperandsArrat<operand_t, 3, OPERAND_INVALID>
 * @brief Type alias for an instruction operand initializer list
 */
using operands_init_t = OperandsArray<operand_t, 3, OPERAND_INVALID>;

/**
 * @brief Emits a bytecode pair with the given data
 * @param ctx Visitor context
 * @param opcode Opcode of the instruction
 * @param operands Initializer list for instruction operands
 * @param comment A comment string
 */
void bytecode_emit(
  VisitorContext& ctx,
  Opcode opcode = Opcode::NOP,
  operands_init_t&& operands = {},
  std::string comment = ""
);

/**
 * @brief Closes and emits all defer statements found inside the given visitor
 * @param ctx Visitor context
 * @param visitor Visitor
 */
void close_defer_statements(VisitorContext& ctx, NodeVisitorBase* visitor);

/**
 * @brief Allocates a new register
 * @param ctx Visitor context
 * @return Register id
 */
inline register_t alloc_register(VisitorContext& ctx) {
  register_t reg = ctx.reg_alloc.allocate_register();
  if (reg == 0xFFFF) {
    compiler_error(ctx, "Register allocation failure");
    compiler_info(
      ctx,
      "This likely indicates an internal compiler bug. Please report this issue in the official "
      "via language github repository."
    );
    compiler_output_end(ctx);
  }

  return reg;
}

/**
 * @brief Fress the given registers that was previously allocated with `alloc_register`
 * @see alloc_register
 * @param ctx Visitor context
 * @param reg Register id
 */
inline void free_register(VisitorContext& ctx, operand_t reg) {
  ctx.reg_alloc.free_register(reg);
}

} // namespace compiler_util

/** @} */

/**
 * @class BytecodeBuilder
 * @brief Builds bytecode from the abstract syntax tree found inside the translation unit context.
 */
class BytecodeBuilder final {
public:
  BytecodeBuilder(Context& lctx)
    : ctx(lctx) {}

  /**
   * @brief Bytecode building entry point
   * @return Fail status
   */
  bool generate();

private:
  /**
   * @brief Prepares the builder and contexts for code generation
   */
  void codegen_prep();

  /**
   * @brief Emits a "successful exit" instruction to guarantee that the program exits safely
   */
  void insert_exit0_instruction();

private:
  VisitorContext ctx;
};

} // namespace via

/** @} */

#endif
