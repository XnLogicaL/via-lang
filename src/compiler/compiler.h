// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_COMPILER_H
#define VIA_HAS_HEADER_COMPILER_H

#include "common.h"

#include <parse/ast.h>
#include <compiler/bytecode.h>
#include <compiler/visitor.h>
#include <compiler/types.h>
#include <interpreter/instruction.h>
#include <interpreter/state.h>

// ===========================================================================================
// compiler.h
// This file declares the compiler class.
//
// The compiler class serves as an abstract compilation interface,
//  taking away the complexity of node visitation, optimization,
//  global tracking, stack tracking, etc.
//
// The `generate` method is the main entry point for performing compilation,
//  it returns a Bool indicating if the program failed or not, of which
//  a value of `true` represents failure. The method could theoretically be called
//  multiple times, but it is not recommended to do so.
namespace via {

namespace compiler_util {

// stdArray wrapper with custom initialization support
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

void compiler_error(VisitorContext& ctx, size_t begin, size_t end, const std::string&);
void compiler_error(VisitorContext& ctx, const Token&, const std::string&);
void compiler_error(VisitorContext& ctx, const std::string&);

void compiler_warning(VisitorContext& ctx, size_t begin, size_t end, const std::string&);
void compiler_warning(VisitorContext& ctx, const Token&, const std::string&);
void compiler_warning(VisitorContext& ctx, const std::string&);

void compiler_info(VisitorContext& ctx, size_t begin, size_t end, const std::string&);
void compiler_info(VisitorContext& ctx, const Token&, const std::string&);
void compiler_info(VisitorContext& ctx, const std::string&);

void compiler_output_end(VisitorContext& ctx);

// Returns the top function in the function stack.
StackFunction& get_current_closure(VisitorContext& ctx);

//  ==================
// [ Constant utility ]
//  ==================

Value construct_constant(LitExprNode& constant);
LitExprNode fold_constant(VisitorContext& ctx, ExprNodeBase* constant, size_t fold_depth = 0);

// Shortcut
// Forwards arguments to std::unit_ctx::constants::push
operand_t push_constant(VisitorContext& ctx, const Value&& constant);

//  =======================
// [ lvalue/rvalue utility ]
//  =======================

// Resolves and emits the given lvalue into register dst. Returns fail status.
bool resolve_lvalue(VisitorContext& ctx, ExprNodeBase* lvalue, operand_t dst);

// Resolves and emits the given rvalue into register dst. Returns fail status.
bool resolve_rvalue(NodeVisitorBase* visitor, ExprNodeBase* rvalue, operand_t dst);

// Binds the given rvalue to the given lvalue. Returns fail status.
bool bind_lvalue(VisitorContext& ctx, ExprNodeBase* lvalue, operand_t src);

// Resolves the type of an expression. Returns fail status.
TypeNodeBase* resolve_type(VisitorContext& ctx, ExprNodeBase* expr);

//  ==================
// [ Bytecode utility ]
//  ==================

// Alias for operands instruction operands initializer list
using operands_init_t = OperandsArray<operand_t, 3, OPERAND_INVALID>;

// Shortcut
// Forwards arguments to ctx::unit_ctx::bytecode::add
void bytecode_emit(
  VisitorContext& ctx,
  Opcode opcode = Opcode::NOP,
  operands_init_t&& operands = {},
  std::string comment = ""
);

//  ===================
// [ Statement utility ]
//  ===================

// Closes accumulated defer statements.
void close_defer_statements(VisitorContext& ctx, NodeVisitorBase* visitor);

//  ==================
// [ Register utility ]
//  ==================

inline operand_t alloc_register(VisitorContext& ctx) {
  return ctx.reg_alloc.allocate_register();
}

inline void free_register(VisitorContext& ctx, operand_t reg) {
  ctx.reg_alloc.free_register(reg);
}

} // namespace compiler_util

class Compiler final {
public:
  Compiler(TransUnitContext& unit_ctx)
    : ctx(unit_ctx) {}

  // Compiler entry point.
  bool generate();

  // Adds a custom unused expression handler to the statement visitor.
  void add_unused_expression_handler(const unused_expression_handler_t& handler);

private:
  void codegen_prep();
  void insert_exit0_instruction();

private:
  VisitorContext ctx;
};

} // namespace via

#endif
