// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_SEMA_H
#define VIA_HAS_HEADER_SEMA_H

#include "common.h"
#include "sema_utils.h"
#include "sema_var.h"
#include "sema_glob.h"
#include "sema_reg.h"
#include "sema_types.h"

namespace via {

struct VisitorContext;
class NodeVisitorBase;

/**
 * @namespace sema
 * @brief Contains compiler utility functions.
 * @defgroup compiler_util_namespace
 * @{
 */
namespace sema {

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
 * @param loc Lex location
 * @param message Error message
 */
void error(VisitorContext& ctx, LexLocation loc, const std::string& message);

/**
 * @brief Emits a general compiler error without location info.
 * @param ctx Visitor context
 * @param message Error message
 */
void error(VisitorContext& ctx, const std::string& message);

/**
 * @brief Emits a compiler warning with a highlighted source range.
 * @param ctx Visitor context
 * @param loc Lex location
 * @param message Warning message
 */
void warning(VisitorContext& ctx, LexLocation loc, const std::string& message);

/**
 * @brief Emits a general compiler info without location info.
 * @param ctx Visitor context
 * @param message Info message
 */
void warning(VisitorContext& ctx, const std::string&);

/**
 * @brief Emits a compiler info with a highlighted source range.
 * @param ctx Visitor context
 * @param loc Lex location
 * @param message Info message
 */
void info(VisitorContext& ctx, LexLocation loc, const std::string& message);

/**
 * @brief Emits a general compiler info without location info.
 * @param ctx Visitor context
 * @param message Error info
 */
void info(VisitorContext& ctx, const std::string&);

/**
 * @brief Signifies the end of compiler output. Currently does nothing
 * @param ctx Visitor context
 */
void flush(VisitorContext& ctx);

/**
 * @brief Returns the top-most closure on the function stack.
 * @param ctx Visitor context
 * @return Reference to top-most closure on the stack.
 */
SemaFunc& get_current_closure(VisitorContext& ctx);

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
NodeLitExpr fold_constant(VisitorContext& ctx, AstNode* constant, size_t fold_depth = 0);

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
bool resolve_lvalue(VisitorContext& ctx, AstNode* lvalue, operand_t dst);

/**
 * @brief Resolves the given rvalue and loads the result into the given register.
 * @param visitor Visitor object
 * @param rvalue rvalue expression
 * @param dst Destination register
 * @return Fail status
 */
bool resolve_rvalue(NodeVisitorBase* visitor, AstNode* rvalue, operand_t dst);

/**
 * @brief Binds the value in the given register to the given lvalue.
 * @param ctx Visitor context
 * @param lvalue lvalue to bind to
 * @param src Register to the get the value to bind
 * @return Fail status
 */
bool bind_lvalue(VisitorContext& ctx, AstNode* lvalue, operand_t src);

/**
 * @brief Resolves the type of the given expression
 * @param ctx Visitor context
 * @param expr Expression to resolve type of
 * @return Resolved type, current can hold the value of nullptr because the type system is
 * incomplete
 */
AstNode* resolve_type(VisitorContext& ctx, AstNode* expr);

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
register_t alloc_register(VisitorContext& ctx);

/**
 * @brief Fress the given registers that was previously allocated with `alloc_register`
 * @see alloc_register
 * @param ctx Visitor context
 * @param reg Register id
 */
void free_register(VisitorContext& ctx, operand_t reg);

} // namespace sema

} // namespace via

#endif
