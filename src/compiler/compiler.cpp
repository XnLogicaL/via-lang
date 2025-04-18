// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "compiler.h"
#include "compiler-types.h"
#include "state.h"
#include "String-utility.h"
#include <cmath>

// ===========================================================================================
// compiler.cpp
//
namespace via {

using enum Opcode;
using enum CErrorLevel;

// Helper function to return line and column number from a character offset
std::pair<size_t, size_t> get_line_and_column(const std::string& source, size_t offset) {
  size_t line = 1;
  size_t column = 1;

  // Iterate over the String until we reach the offset
  for (size_t i = 0; i < offset; ++i) {
    if (source[i] == '\n') {
      ++line;
      column = 1; // Reset column to 1 for a new line
    }
    else {
      ++column;
    }
  }

  return {line, column};
}

namespace compiler_util {

Value construct_constant(LitExprNode& literal_node) {
  using enum Value::Tag;
  return std::visit(
    [](auto&& val) -> Value {
      using T = std::decay_t<decltype(val)>;

      if constexpr (std::is_same_v<T, int>) {
        return Value(val);
      }
      else if constexpr (std::is_same_v<T, bool>) {
        return Value(val, true);
      }
      else if constexpr (std::is_same_v<T, float>) {
        return Value(val);
      }
      else if constexpr (std::is_same_v<T, std::string>) {
        return Value(val.data());
      }
      else if constexpr (std::is_same_v<T, std::monostate>) {
        return Value();
      }

      VIA_UNREACHABLE();
    },
    literal_node.value
  );
}

#if VIA_COMPILER == C_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winfinite-recursion"
#endif

LitExprNode fold_constant(VisitorContext& ctx, ExprNodeBase* expr, size_t fold_depth) {
  using enum TokenType;
  using evaluator_2i_t = std::function<int(int, int)>;
  using evaluator_2f_t = std::function<float(float, float)>;
  using evaluator_fi_t = std::function<float(float, int)>;

  static const std::unordered_map<TokenType, evaluator_2i_t> evaluators_2i = {
    {OP_ADD, [](int a, int b) { return a + b; }},
    {OP_SUB, [](int a, int b) { return a - b; }},
    {OP_MUL, [](int a, int b) { return a * b; }},
    {OP_DIV, [](int a, int b) { return a / b; }},
    {OP_EXP, [](int a, int b) { return std::pow(a, b); }},
    {OP_MOD, [](int a, int b) { return a % b; }},
  };

  static const std::unordered_map<TokenType, evaluator_2f_t> evaluators_2f = {
    {OP_ADD, [](float a, float b) { return a + b; }},
    {OP_SUB, [](float a, float b) { return a - b; }},
    {OP_MUL, [](float a, float b) { return a * b; }},
    {OP_DIV, [](float a, float b) { return a / b; }},
    {OP_EXP, [](float a, float b) { return std::pow(a, b); }},
    {OP_MOD, [](float a, float b) { return std::fmod(a, b); }},
  };

  static const std::unordered_map<TokenType, evaluator_fi_t> evaluators_fi = {
    {OP_ADD, [](float a, int b) { return a + b; }},
    {OP_SUB, [](float a, int b) { return a - b; }},
    {OP_MUL, [](float a, int b) { return a * b; }},
    {OP_DIV, [](float a, int b) { return a / b; }},
    {OP_EXP, [](float a, int b) { return std::pow(a, b); }},
    {OP_MOD, [](float a, int b) { return std::fmod(a, b); }},
  };

  if (LitExprNode* lit_expr = get_derived_instance<ExprNodeBase, LitExprNode>(expr)) {
    return *get_derived_instance<ExprNodeBase, LitExprNode>(lit_expr);
  }
  if (BinExprNode* bin_expr = get_derived_instance<ExprNodeBase, BinExprNode>(expr)) {
    LitExprNode left = fold_constant(ctx, bin_expr->lhs_expression, fold_depth + 1);
    LitExprNode right = fold_constant(ctx, bin_expr->rhs_expression, fold_depth + 1);

    if (int* int_left = std::get_if<int>(&left.value)) {
      if (int* int_right = std::get_if<int>(&right.value)) {
        evaluator_2i_t evaluator = evaluators_2i.at(bin_expr->op.type);
        return LitExprNode(Token(), evaluator(*int_left, *int_right));
      }
      else if (float* float_right = std::get_if<float>(&right.value)) {
        evaluator_fi_t evaluator = evaluators_fi.at(bin_expr->op.type);
        return LitExprNode(Token(), evaluator(*float_right, *int_left));
      }

      TypeNodeBase* left_type = left.infer_type(ctx.unit_ctx);
      TypeNodeBase* right_type = right.infer_type(ctx.unit_ctx);

      VIA_ASSERT(left_type && right_type, "!!tmp!! inference failed");

      compiler_error(
        ctx,
        expr->begin,
        expr->end,
        std::format(
          "Constant binary expression on incompatible types {} and {}",
          left_type->to_output_string(),
          right_type->to_output_string()
        )
      );
      compiler_output_end(ctx);
      goto bad_fold;
    }
    else if (float* float_left = std::get_if<float>(&left.value)) {
      if (float* float_right = std::get_if<float>(&right.value)) {
        evaluator_2f_t evaluator = evaluators_2f.at(bin_expr->op.type);
        return LitExprNode(Token(), evaluator(*float_left, *float_right));
      }
      else if (int* int_right = std::get_if<int>(&right.value)) {
        evaluator_fi_t evaluator = evaluators_fi.at(bin_expr->op.type);
        return LitExprNode(Token(), evaluator(*float_left, *int_right));
      }

      TypeNodeBase* left_type = left.infer_type(ctx.unit_ctx);
      TypeNodeBase* right_type = right.infer_type(ctx.unit_ctx);

      VIA_ASSERT(left_type && right_type, "!!tmp!! inference failed");

      compiler_error(
        ctx,
        expr->begin,
        expr->end,
        std::format(
          "Constant binary expression on incompatible types {} and {}",
          left_type->to_output_string(),
          right_type->to_output_string()
        )
      );
      compiler_output_end(ctx);
      goto bad_fold;
    }
  }
  else if (SymExprNode* sym_expr = get_derived_instance<ExprNodeBase, SymExprNode>(expr)) {
    auto closure = get_current_closure(ctx);
    auto stk_id = closure.locals.get_local_by_symbol(sym_expr->identifier.lexeme);
    if (!stk_id.has_value()) {
      goto bad_fold;
    }

    // Check if call exceeds variable depth limit
    if (fold_depth > 5) {
      compiler_error(ctx, expr->begin, expr->end, "Constant fold variable depth exceeded");
      compiler_info(
        ctx,
        "This error message likely indicates an internal compiler bug. Please create "
        "an issue at https://github.com/XnLogicaL/via-lang."
      );
      goto bad_fold;
    }

    return fold_constant(ctx, (*stk_id)->value, ++fold_depth);
  }

bad_fold:
  return LitExprNode(Token(), std::monostate());
}

operand_t push_constant(VisitorContext& ctx, const Value&& constant) {
  return ctx.unit_ctx.constants->push_constant(std::move(constant));
}

#if VIA_COMPILER == C_GCC
#pragma GCC diagnostic pop
#endif

void compiler_error(VisitorContext& ctx, size_t begin, size_t end, const std::string& message) {
  auto lc_info = get_line_and_column(ctx.unit_ctx.file_source, begin);
  ctx.errc++;
  ctx.failed = true;
  ctx.err_bus.log(
    {false, message, ctx.unit_ctx, ERROR_, {lc_info.first, lc_info.second, begin, end}}
  );
}

void compiler_error(VisitorContext& ctx, const Token& Token, const std::string& message) {
  ctx.errc++;
  ctx.failed = true;
  ctx.err_bus.log({false, message, ctx.unit_ctx, ERROR_, Token});
}

void compiler_error(VisitorContext& ctx, const std::string& message) {
  ctx.errc++;
  ctx.failed = true;
  ctx.err_bus.log({true, message, ctx.unit_ctx, ERROR_, {}});
}

void compiler_warning(VisitorContext& ctx, size_t begin, size_t end, const std::string& message) {
  auto lc_info = get_line_and_column(ctx.unit_ctx.file_source, begin);
  ctx.err_bus.log(
    {false, message, ctx.unit_ctx, WARNING, {lc_info.first, lc_info.second, begin, end}}
  );
}

void compiler_warning(VisitorContext& ctx, const Token& Token, const std::string& message) {
  ctx.err_bus.log({false, message, ctx.unit_ctx, WARNING, Token});
}

void compiler_warning(VisitorContext& ctx, const std::string& message) {
  ctx.err_bus.log({true, message, ctx.unit_ctx, WARNING, {}});
}

void compiler_info(VisitorContext& ctx, size_t begin, size_t end, const std::string& message) {
  auto lc_info = get_line_and_column(ctx.unit_ctx.file_source, begin);
  ctx.err_bus.log({false, message, ctx.unit_ctx, INFO, {lc_info.first, lc_info.second, begin, end}}
  );
}

void compiler_info(VisitorContext& ctx, const Token& Token, const std::string& message) {
  ctx.err_bus.log({false, message, ctx.unit_ctx, INFO, Token});
}

void compiler_info(VisitorContext& ctx, const std::string& message) {
  ctx.err_bus.log({true, message, ctx.unit_ctx, INFO, {}});
}

void compiler_output_end(VisitorContext& ctx) {
  ctx.err_bus.new_line();
}

StackFunction& get_current_closure(VisitorContext& ctx) {
  return ctx.unit_ctx.internal.function_stack->top();
}

bool resolve_lvalue(VisitorContext& ctx, ExprNodeBase* lvalue, operand_t dst) {
  auto& current_closure = get_current_closure(ctx);

  if (SymExprNode* sym_expr = get_derived_instance<ExprNodeBase, SymExprNode>(lvalue)) {
    Token var_id = sym_expr->identifier;
    symbol_t symbol = var_id.lexeme;

    auto stk_id = current_closure.locals.find_local_id(var_id.lexeme);
    if (stk_id.has_value()) {
      for (operand_t i = 0; StackVariable & param : current_closure.locals) {
        if (param.symbol == symbol) {
          bytecode_emit(ctx, GETLOCAL, {dst, i}, symbol);
          return false;
        }
        ++i;
      }
    }
    else if (ctx.unit_ctx.internal.globals->was_declared(symbol)) {
      LitExprNode lit_translation = LitExprNode(sym_expr->identifier, symbol);
      Value const_val = construct_constant(lit_translation);
      operand_t const_id = push_constant(ctx, std::move(const_val));
      operand_t tmp_reg = ctx.reg_alloc.allocate_temp();

      bytecode_emit(ctx, LOADK, {tmp_reg, const_id});
      bytecode_emit(ctx, GETGLOBAL, {dst, tmp_reg}, symbol);
      return false;
    }
    else if (ctx.unit_ctx.internal.function_stack->size() > 0) {
      operand_t index = 0;
      auto& top = ctx.unit_ctx.internal.function_stack->top();

      for (const auto& parameter : top.decl->parameters) {
        if (parameter.identifier.lexeme == symbol) {
          bytecode_emit(ctx, GETLOCAL, {dst, index}, symbol);
          return false;
        }

        ++index;
      }
    }
  }

  return true;
}

bool resolve_rvalue(NodeVisitorBase* visitor, ExprNodeBase* rvalue, operand_t dst) {
  rvalue->accept(*visitor, dst);
  return visitor->failed();
}

bool bind_lvalue(VisitorContext& ctx, ExprNodeBase* lvalue, operand_t src) {
  if (SymExprNode* sym_expr = get_derived_instance<ExprNodeBase, SymExprNode>(lvalue)) {
    Token symbol_token = sym_expr->identifier;
    symbol_t symbol = symbol_token.lexeme;
    auto& current_closure = get_current_closure(ctx);

    // Stack info
    auto stack_id = current_closure.locals.find_local_id(symbol);
    auto stack_obj = current_closure.locals.get_local_by_symbol(symbol);

    if (stack_id.has_value()) {
      if (stack_obj.has_value() && (*stack_obj)->is_const) {
        // Error: "constant-lvalue-assignment"
        auto message = std::format("Assignment to constant lvalue '{}'", symbol);
        compiler_error(ctx, symbol_token, message);
        compiler_output_end(ctx);
        return true;
      }

      bytecode_emit(ctx, SETLOCAL, {src, *stack_id}, symbol);
      return false;
    }
    else {
      // Error: "unknown-lvalue-assignment"
      auto message = std::format("Assignment to unknown lvalue '{}'", symbol);
      compiler_error(ctx, symbol_token, message);
      compiler_output_end(ctx);
    }
  }
  else {
    // Error: "invalid-lvalue-assignment"
    auto message = "Assignment to invalid lvalue";
    compiler_error(ctx, lvalue->begin, lvalue->end, message);
    compiler_output_end(ctx);
  }

  return true;
}

TypeNodeBase* resolve_type(VisitorContext& ctx, ExprNodeBase* expr) {
  return expr->infer_type(ctx.unit_ctx);
}

void bytecode_emit(VisitorContext& ctx, Opcode opc, operands_init_t&& ops, std::string com) {
  ctx.unit_ctx.bytecode->add({
    {
      opc,
      ops.data.at(0),
      ops.data.at(1),
      ops.data.at(2),
    },
    {com},
  });
}

void close_defer_statements(VisitorContext& ctx, NodeVisitorBase* visitor) {
  std::vector<StmtNodeBase*> defered_stmts = ctx.unit_ctx.internal.defered_stmts.top();
  ctx.unit_ctx.internal.defered_stmts.pop();

  // Emit defered statements
  for (StmtNodeBase* stmt : defered_stmts) {
    stmt->accept(*visitor);
  }
}

} // namespace compiler_util

void Compiler::codegen_prep() {
  ctx.unit_ctx.internal.globals->declare_builtins();
  ctx.unit_ctx.internal.function_stack->push_main_function(ctx.unit_ctx);
}

void Compiler::insert_exit0_instruction() {
  compiler_util::bytecode_emit(ctx, Opcode::RET0, {});
}

bool Compiler::generate() {
  StmtNodeVisitor visitor(ctx);

  codegen_prep();

  for (StmtNodeBase* stmt : ctx.unit_ctx.ast->statements) {
    if (DeferStmtNode* defer_stmt = get_derived_instance<StmtNodeBase, DeferStmtNode>(stmt)) {
      auto message = "Defer statements not allowed in global scope";
      compiler_util::compiler_error(ctx, defer_stmt->begin, defer_stmt->end, message);
      compiler_util::compiler_output_end(ctx);
      continue;
    }

    stmt->accept(visitor);
  }

  insert_exit0_instruction();

  if (ctx.errc > 0) {
    auto message = std::format("{} error(s) generated.", ctx.errc);
    compiler_util::compiler_error(ctx, message);
  }

  return visitor.failed();
}

} // namespace via
