// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

/**
 * @file bytecode-builder.cpp
 * @brief BytecodeBuilder class imeplementation
 *
 * @details This file implements the BytecodeBuilder class, which builds serial bytecode from the
 * Abstract Syntax Tree. It uses the visitor pattern in order to visit each node and emit
 * appropriate bytecode.
 */
#include "bytecode-builder.h"
#include <tstring.h>
#include <cmath>

/**
 * @namespace via
 */
namespace via {

using enum Opcode;
using enum CErrorLevel;

/**
 * @brief Translates an absolute offset to line and column of a source string.
 *
 * @param source The source string
 * @param offset The absolute offset
 * @return The line and column as a pair object
 */
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

/**
 * @namespace sema
 * @brief Contains compiler utility functions.
 */
namespace sema {

/**
 * @brief Constructs a constant value from the given literal expression node.
 */
Value construct_constant(NodeLitExpr& literal_node) {
  using enum Value::Tag;
  return std::visit(
    [](auto&& val) -> Value {
      using T = std::decay_t<decltype(val)>;
      if constexpr (std::is_same_v<T, int> || std::is_same_v<T, bool> || std::is_same_v<T, float>)
        return Value(val);
      else if constexpr (std::is_same_v<T, std::string>)
        return Value(new struct String(val.c_str()));
      else if constexpr (std::is_same_v<T, std::monostate>)
        return Value();
      VIA_UNREACHABLE();
    },
    literal_node.value
  );
}

#if VIA_COMPILER == C_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winfinite-recursion"
#endif

/**
 * @brief Folds an expression into a constant if possible.
 */
NodeLitExpr fold_constant(VisitorContext& ctx, ExprNode* expr, size_t fold_depth) {
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
    {OP_EQ, [](int a, int b) { return a == b; }},
    {OP_NEQ, [](int a, int b) { return a != b; }},
    {KW_AND, [](int a, int b) { return a && b; }},
  };

  static const std::unordered_map<TokenType, evaluator_2f_t> evaluators_2f = {
    {OP_ADD, [](float a, float b) { return a + b; }},
    {OP_SUB, [](float a, float b) { return a - b; }},
    {OP_MUL, [](float a, float b) { return a * b; }},
    {OP_DIV, [](float a, float b) { return a / b; }},
    {OP_EXP, [](float a, float b) { return std::pow(a, b); }},
    {OP_MOD, [](float a, float b) { return std::fmod(a, b); }},
    {OP_EQ, [](float a, float b) { return a == b; }},
    {OP_NEQ, [](float a, float b) { return a != b; }},
    {KW_AND, [](float a, float b) { return a && b; }},
  };

  static const std::unordered_map<TokenType, evaluator_fi_t> evaluators_fi = {
    {OP_ADD, [](float a, int b) { return a + b; }},
    {OP_SUB, [](float a, int b) { return a - b; }},
    {OP_MUL, [](float a, int b) { return a * b; }},
    {OP_DIV, [](float a, int b) { return a / b; }},
    {OP_EXP, [](float a, int b) { return std::pow(a, b); }},
    {OP_MOD, [](float a, int b) { return std::fmod(a, b); }},
    {OP_EQ, [](float a, int b) { return a == b; }},
    {OP_NEQ, [](float a, int b) { return a != b; }},
    {KW_AND, [](float a, int b) { return a && b; }},
  };

  if (NodeLitExpr* lit_expr = dynamic_cast<NodeLitExpr*>(expr))
    return *lit_expr;
  else if (NodeBinExpr* bin_expr = dynamic_cast<NodeBinExpr*>(expr)) {
    bool is_cond = (int)bin_expr->op.type >= (int)OP_EQ && (int)bin_expr->op.type <= (int)OP_GEQ;
    NodeLitExpr left = fold_constant(ctx, bin_expr->lhs_expression, fold_depth + 1);
    NodeLitExpr right = fold_constant(ctx, bin_expr->rhs_expression, fold_depth + 1);

    if (int* int_left = std::get_if<int>(&left.value)) {
      if (int* int_right = std::get_if<int>(&right.value)) {
        evaluator_2i_t evaluator = evaluators_2i.at(bin_expr->op.type);
        if (is_cond)
          return NodeLitExpr(Token(), (bool)evaluator(*int_left, *int_right));
        else
          return NodeLitExpr(Token(), evaluator(*int_left, *int_right));
      }
      else if (float* float_right = std::get_if<float>(&right.value)) {
        evaluator_fi_t evaluator = evaluators_fi.at(bin_expr->op.type);
        if (is_cond)
          return NodeLitExpr(Token(), (bool)evaluator(*int_left, *float_right));
        else
          return NodeLitExpr(Token(), evaluator(*int_left, *float_right));
      }

      TypeNode* left_type = left.infer_type(ctx.lctx);
      TypeNode* right_type = right.infer_type(ctx.lctx);

      VIA_ASSERT(left_type && right_type, "!!tmp!! inference failed");

      error(
        ctx,
        expr->begin,
        expr->end,
        std::format(
          "Constant binary expression on incompatible types {} and {}",
          left_type->to_output_string(),
          right_type->to_output_string()
        )
      );
      flush(ctx);
      goto bad_fold;
    }
    else if (float* float_left = std::get_if<float>(&left.value)) {
      if (float* float_right = std::get_if<float>(&right.value)) {
        evaluator_2f_t evaluator = evaluators_2f.at(bin_expr->op.type);
        if (is_cond)
          return NodeLitExpr(Token(), (bool)evaluator(*float_left, *float_right));
        else
          return NodeLitExpr(Token(), evaluator(*float_left, *float_right));
      }
      else if (int* int_right = std::get_if<int>(&right.value)) {
        evaluator_fi_t evaluator = evaluators_fi.at(bin_expr->op.type);
        if (is_cond)
          return NodeLitExpr(Token(), (bool)evaluator(*float_left, *int_right));
        else
          return NodeLitExpr(Token(), evaluator(*float_left, *int_right));
      }

      TypeNode* left_type = left.infer_type(ctx.lctx);
      TypeNode* right_type = right.infer_type(ctx.lctx);

      VIA_ASSERT(left_type && right_type, "TODO: FOLD CONSTANT FAILURE");

      error(
        ctx,
        expr->begin,
        expr->end,
        std::format(
          "Constant binary expression on incompatible types {} and {}",
          left_type->to_output_string(),
          right_type->to_output_string()
        )
      );
      flush(ctx);
      goto bad_fold;
    }
  }
  else if (NodeSymExpr* sym_expr = dynamic_cast<NodeSymExpr*>(expr)) {
    auto closure = get_current_closure(ctx);
    auto stk_id = closure.locals.get_local_by_symbol(sym_expr->identifier.lexeme);
    if (!stk_id.has_value()) {
      goto bad_fold;
    }

    // Check if call exceeds variable depth limit
    if (fold_depth > 5) {
      error(ctx, expr->begin, expr->end, "Constant fold variable depth exceeded");
      info(
        ctx,
        "This error message likely indicates an compiler bug. Please create "
        "an issue at https://github.com/XnLogicaL/via-lang."
      );
      goto bad_fold;
    }

    return fold_constant(ctx, (*stk_id)->value, ++fold_depth);
  }

bad_fold:
  return NodeLitExpr(Token(), std::monostate());
}

/**
 * @brief Pushes a constant onto the constant table.
 */
operand_t push_constant(VisitorContext& ctx, Value&& constant) {
  for (size_t constant_id = 0; const Value& existing_constant : ctx.lctx.constants) {
    if (constant.deep_compare(existing_constant)) {
      return constant_id;
    }
    ++constant_id;
  }

  ctx.lctx.constants.emplace_back(std::move(constant));
  return ctx.lctx.constants.size() - 1;
}

#if VIA_COMPILER == C_GCC
#pragma GCC diagnostic pop
#endif

void error(VisitorContext& ctx, size_t begin, size_t end, const std::string& message) {
  auto lc_info = get_line_and_column(ctx.lctx.file_source, begin);
  ctx.errc++;
  ctx.failed = true;
  ctx.err_bus.log({false, message, ctx.lctx, ERROR_, {lc_info.first, lc_info.second, begin, end}});
}

void error(VisitorContext& ctx, const Token& Token, const std::string& message) {
  ctx.errc++;
  ctx.failed = true;
  ctx.err_bus.log({false, message, ctx.lctx, ERROR_, Token});
}

void error(VisitorContext& ctx, const std::string& message) {
  ctx.errc++;
  ctx.failed = true;
  ctx.err_bus.log({true, message, ctx.lctx, ERROR_, {}});
}

void warning(VisitorContext& ctx, size_t begin, size_t end, const std::string& message) {
  auto lc_info = get_line_and_column(ctx.lctx.file_source, begin);
  ctx.err_bus.log({false, message, ctx.lctx, WARNING, {lc_info.first, lc_info.second, begin, end}});
}

void warning(VisitorContext& ctx, const Token& Token, const std::string& message) {
  ctx.err_bus.log({false, message, ctx.lctx, WARNING, Token});
}

void warning(VisitorContext& ctx, const std::string& message) {
  ctx.err_bus.log({true, message, ctx.lctx, WARNING, {}});
}

void info(VisitorContext& ctx, size_t begin, size_t end, const std::string& message) {
  auto lc_info = get_line_and_column(ctx.lctx.file_source, begin);
  ctx.err_bus.log({false, message, ctx.lctx, INFO, {lc_info.first, lc_info.second, begin, end}});
}

void info(VisitorContext& ctx, const Token& Token, const std::string& message) {
  ctx.err_bus.log({false, message, ctx.lctx, INFO, Token});
}

void info(VisitorContext& ctx, const std::string& message) {
  ctx.err_bus.log({true, message, ctx.lctx, INFO, {}});
}

void flush(VisitorContext& ctx) {
  ctx.err_bus.new_line();
}

StackFunction& get_current_closure(VisitorContext& ctx) {
  return ctx.lctx.function_stack.back();
}

bool resolve_lvalue(VisitorContext& ctx, ExprNode* lvalue, operand_t dst) {
  auto& current_closure = get_current_closure(ctx);

  if (NodeSymExpr* sym_expr = dynamic_cast<NodeSymExpr*>(lvalue)) {
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
    else if (ctx.lctx.globals.was_declared(symbol)) {
      NodeLitExpr lit_translation = NodeLitExpr(sym_expr->identifier, symbol);
      Value const_val = construct_constant(lit_translation);
      operand_t const_id = push_constant(ctx, std::move(const_val));

      bytecode_emit(ctx, LOADK, {dst, const_id});
      bytecode_emit(ctx, GETGLOBAL, {dst, dst}, symbol);
      return false;
    }

    auto& top = ctx.lctx.function_stack.back();
    for (operand_t index = 0; const auto& parameter : top.decl->parameters) {
      if (parameter.identifier.lexeme == symbol) {
        bytecode_emit(ctx, GETARG, {dst, index});
        return false;
      }

      ++index;
    }
  }

  return true;
}

bool resolve_rvalue(NodeVisitorBase* visitor, ExprNode* rvalue, operand_t dst) {
  rvalue->accept(*visitor, dst);
  return visitor->failed();
}

bool bind_lvalue(VisitorContext& ctx, ExprNode* lvalue, operand_t src) {
  if (NodeSymExpr* sym_expr = dynamic_cast<NodeSymExpr*>(lvalue)) {
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
        error(ctx, symbol_token, message);
        flush(ctx);
        return true;
      }

      auto& var_repr = current_closure.locals[*stack_id];
      var_repr.value = lvalue;

      bytecode_emit(ctx, SETLOCAL, {src, *stack_id}, symbol);
      return false;
    }
    else {
      // Error: "unknown-lvalue-assignment"
      auto message = std::format("Assignment to unknown lvalue '{}'", symbol);
      error(ctx, symbol_token, message);
      flush(ctx);
    }
  }
  else {
    // Error: "invalid-lvalue-assignment"
    auto message = "Assignment to invalid lvalue";
    error(ctx, lvalue->begin, lvalue->end, message);
    flush(ctx);
  }

  return true;
}

TypeNode* resolve_type(VisitorContext& ctx, ExprNode* expr) {
  TypeNode* type = expr->infer_type(ctx.lctx);
  if (!type) {
    error(ctx, expr->begin, expr->end, "Expression type could not be infered");
    info(
      ctx,
      "This message indicates a likely compiler bug. Please report it at "
      "https://github.com/XnLogicaL/via-lang"
    );
    return nullptr;
  }

  return type;
}

void bytecode_emit(VisitorContext& ctx, Opcode opc, operands_init_t&& ops, std::string com) {
  ctx.lctx.bytecode.push_back({opc, ops.data.at(0), ops.data.at(1), ops.data.at(2)});
  ctx.lctx.bytecode_data.push_back({com});
}

void close_defer_statements(VisitorContext& ctx, NodeVisitorBase* visitor) {
  std::vector<StmtNode*> defered_stmts = ctx.lctx.defered_stmts.back();
  ctx.lctx.defered_stmts.pop_back();

  // Emit defered statements
  for (StmtNode* stmt : defered_stmts) {
    stmt->accept(*visitor);
  }
}

} // namespace sema

void BytecodeBuilder::codegen_prep() {
  ctx.lctx.globals.declare_builtins();
  ctx.lctx.function_stack.push_main_function(ctx.lctx);
}

void BytecodeBuilder::insert_exit0_instruction() {
  sema::bytecode_emit(ctx, Opcode::RETBF, {});
}

bool BytecodeBuilder::generate() {
  StmtNodeVisitor visitor(ctx);

  codegen_prep();

  for (StmtNode* stmt : ctx.lctx.ast) {
    if (NodeDeferStmt* defer_stmt = dynamic_cast<NodeDeferStmt*>(stmt)) {
      auto message = "Defer statements not allowed in global scope";
      sema::error(ctx, defer_stmt->begin, defer_stmt->end, message);
      sema::flush(ctx);
      continue;
    }

    stmt->accept(visitor);
  }

  insert_exit0_instruction();

  if (ctx.errc > 0) {
    auto message = std::format("{} error(s) generated.", ctx.errc);
    sema::error(ctx, message);
  }

  return visitor.failed();
}

} // namespace via
