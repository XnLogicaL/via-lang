// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "state.h"
#include "bit-utility.h"
#include "stack.h"
#include "String-utility.h"
#include "compiler.h"
#include "compiler-types.h"
#include "visitor.h"
#include <cmath>

// ==========================================================================================
// visitor_expr.cpp
//
// This file is a part of the first compiler stage (0), and is used to compile expressions.
// It defines the `ExpressionVisitor::visit` function overloads.
//
// All visitor functions have common parameters: (NodeType& node, operand_t dst).
// - NodeType& node: AST node object.
// - operand_t dst: The destination register in which the expression lives until externally free'd.
//  - Note: This also implies that `dst` is not owned.
//
// Visitor functions compile each type of expression node by first converting it into
// corresponding IOpCode(s), and then determining the operands via the built-in node parameters.
//
// - LitExprNode compilation:
//  This node only emits `LOAD` opcodes, and is considered a constant expression.
//  It first checks for primitive data types within the node, and emits corresponding bytecode based
//  on that. If it finds complex data types like strings, tables, etc., it loads them into the
//  constant table and emits a `LOADK` instruction with the corresponding constant id.
//
// - SymExprNode compilation:
//  This node represents a "symbol" that is either a local, global, argument or UpValue.
//  It first checks the stack for the symbol, if found, emits a `STKGET` instruction with the
//  stack id of the symbol. After that, it checks for upvalues, if found emits `GETUPV`. Next,
//  it checks for arguments by traversing the parameters of the top function in
//  `unit_ctx::internal.variable_stack::function_stack` and looking for the symbol. If found, emits
//  `ARGGET`. Finally, looks for the variable in the global scope by querying
//  `unit_ctx::internal::globals` and if found emits GGET. If all of these queries fail, throws
//  a "Use of undeclared variable" compilation error.
//
// - UnaryExprNode compilation:
//
//
// - GroupExprNode compilation:
//  Compiles the inner expression into dst.
//
// - CallExprNode compilation:
//  This node represents a function call expression, which first loads the arguments onto the stack
//  (LIFO), loads the callee object, and calls it. And finally, emits a POP instruction to retrieve
//  the return value.
//
// - IndexExprNode compilation:
//  This node represents a member access, which could follow either of these patterns:
//    -> Direct table member access: table.index
//      This pattern compiles into a TBLGET instruction that uses the hashed version of the index.
//    -> Expressional table access: table[index]
//      This pattern first compiles the index expression, then casts it into a String, and finally
//      uses it as a table index.
//    -> Object member access: object::index
//      This pattern is by far the most complex pattern with multiple arbitriary checks in order to
//      ensure access validity, conventional integrity, and type safety. It first checks if the
//      index value is a symbol or not to make the guarantee that aggregate types can only have
//      symbols as keys. After that, it searches for the field name inside the aggregate type, if
//      not found, throws a "Aggregate object has no field named ..." compilation error. And the
//      final check, which checks for private member access patterns (object::_index) which by
//      convention tell the compiler that members prefixed with '_' are private members of the
//      object. Upon failure, throws a "Private member access into object..." warning.
//
//
// ==================================================================================================
namespace via {

using enum IOpCode;
using enum Value::Tag;
using namespace compiler_util;
using OpCodeId = std::underlying_type_t<IOpCode>;

void ExprNodeVisitor::visit(LitExprNode& lit_expr, operand_t dst) {
  if (int* integer_value = std::get_if<int>(&lit_expr.value)) {
    uint32_t final_value = *integer_value;
    auto operands = reinterpret_u32_as_2u16(final_value);
    bytecode_emit(ctx, LOADI, {dst, operands.high, operands.low});
  }
  else if (float* float_value = std::get_if<float>(&lit_expr.value)) {
    uint32_t final_value = std::bit_cast<uint32_t>(*float_value);
    auto operands = reinterpret_u32_as_2u16(final_value);
    bytecode_emit(ctx, LOADF, {dst, operands.high, operands.low});
  }
  else if (bool* bool_value = std::get_if<bool>(&lit_expr.value)) {
    bytecode_emit(ctx, *bool_value ? LOADBT : LOADBF, {dst});
  }
  else {
    Value constant = construct_constant(lit_expr);
    operand_t constant_id = push_constant(ctx, std::move(constant));
    bytecode_emit(ctx, LOADK, {dst, constant_id});
  }
}

void ExprNodeVisitor::visit(SymExprNode& sym_expr, operand_t dst) {
  Token& id = sym_expr.identifier;

  if (resolve_lvalue(ctx, &sym_expr, dst)) {
    // Error: "undeclared-id-use"
    auto message = std::format("Use of undeclared identifier '{}'", id.lexeme);
    compiler_error(ctx, id, message);
    compiler_output_end(ctx);
  }
}

void ExprNodeVisitor::visit(UnaryExprNode& unary_node, operand_t dst) {
  TypeNodeBase* type = resolve_type(ctx, &unary_node);
  resolve_rvalue(this, &unary_node, dst);

  if (unary_node.op.type == TokenType::OP_SUB) {
    if (is_arithmetic(type)) {
      bytecode_emit(ctx, NEG, {dst});
    }
    else {
      // Error: "ill-negation"
      auto message = std::format("Negating non-negatable type {}", type->to_output_string());
      compiler_error(ctx, unary_node.begin, unary_node.end, message);
      compiler_output_end(ctx);
    }
  }
  else if (unary_node.op.type == TokenType::OP_LEN) {
    if (is_derived_instance<TypeNodeBase, ArrayTypeNode>(type)) {
      register_t reg = ctx.reg_alloc.allocate_register();
      bytecode_emit(ctx, MOV, {reg, dst});
      bytecode_emit(ctx, LENARR, {dst, reg});
      return;
    }

    // Error: "ill-length-computation"
    auto message = std::format("Taking length of unbounded type {}", type->to_output_string());
    compiler_error(ctx, unary_node.begin, unary_node.end, message);
    compiler_output_end(ctx);
  }
  else if (unary_node.op.type == TokenType::OP_INC || unary_node.op.type == TokenType::OP_DEC) {
    IOpCode opcode = unary_node.op.type == TokenType::OP_INC ? INC : DEC;

    if (!is_arithmetic(type)) {
      // Error: "ill-step"
      compiler_error(ctx, unary_node.begin, unary_node.end, "Stepping non-arithmetic data type");
      compiler_output_end(ctx);
      return;
    }

    bytecode_emit(ctx, opcode, {dst});
  }
  else {
    // Error: "unknown-operator"
    auto message = std::format("Unknown unary operator '{}'", unary_node.op.lexeme);
    compiler_error(ctx, unary_node.op, message);
    compiler_output_end(ctx);
  }
}

void ExprNodeVisitor::visit(GroupExprNode& group_node, operand_t dst) {
  resolve_rvalue(this, group_node.expression, dst);
}

void ExprNodeVisitor::visit(CallExprNode& call_node, operand_t dst) {
  ExprNodeBase* callee = call_node.callee;
  TypeNodeBase* callee_type = resolve_type(ctx, callee);
  operand_t argc = call_node.arguments.size();
  operand_t callee_reg = alloc_register(ctx);

  CHECK_INFERED_TYPE(callee_type, callee);

  if (FunctionTypeNode* fn_ty = get_derived_instance<TypeNodeBase, FunctionTypeNode>(callee_type)) {
    size_t expected_argc = fn_ty->parameters.size();
    if (argc != static_cast<operand_t>(fn_ty->parameters.size())) {
      // Error: "function-call-argc-mismatch"
      auto message = std::format("Function type expects {} arguments, got {}", expected_argc, argc);
      compiler_error(ctx, call_node.begin, call_node.end, message);
      compiler_output_end(ctx);
    }
  }
  else {
    // Error: "ill-function-call"
    auto message =
      std::format("Value of type '{}' is not callable", callee_type->to_output_string());
    compiler_error(ctx, callee->begin, callee->end, message);
    compiler_output_end(ctx);
  }

  resolve_rvalue(this, callee, callee_reg);

  for (ExprNodeBase* argument : call_node.arguments) {
    operand_t arg_reg = alloc_register(ctx);

    if (LitExprNode* lit_expr = get_derived_instance<ExprNodeBase, LitExprNode>(argument)) {
      if (int* integer_value = std::get_if<int>(&lit_expr->value)) {
        uint32_t final_value = *integer_value;
        auto operands = reinterpret_u32_as_2u16(final_value);
        bytecode_emit(ctx, PUSHI, {operands.high, operands.low});
      }
      else if (float* float_value = std::get_if<float>(&lit_expr->value)) {
        uint32_t final_value = std::bit_cast<uint32_t>(*float_value);
        auto operands = reinterpret_u32_as_2u16(final_value);
        bytecode_emit(ctx, PUSHF, {operands.high, operands.low});
      }
      else if (bool* bool_value = std::get_if<bool>(&lit_expr->value)) {
        bytecode_emit(ctx, *bool_value ? PUSHBT : PUSHBF);
      }
      else {
        Value constant = construct_constant(*lit_expr);
        operand_t constant_id = push_constant(ctx, std::move(constant));
        bytecode_emit(ctx, PUSHK, {constant_id});
      }
    }
    else {
      resolve_rvalue(this, argument, arg_reg);
      bytecode_emit(ctx, PUSH, {arg_reg});
      free_register(ctx, arg_reg);
    }
  }

  bytecode_emit(ctx, CALL, {callee_reg, argc});
  bytecode_emit(ctx, RETGET, {dst});
  free_register(ctx, callee_reg);
}

void ExprNodeVisitor::visit(IndexExprNode& index_node, operand_t dst) {
  TypeNodeBase* object_type = index_node.object->infer_type(ctx.unit_ctx);
  TypeNodeBase* index_type = index_node.index->infer_type(ctx.unit_ctx);
  operand_t obj_reg = alloc_register(ctx);

  CHECK_INFERED_TYPE(object_type, index_node.object);
  CHECK_INFERED_TYPE(index_type, index_node.object);

  resolve_rvalue(this, index_node.object, obj_reg);

  if (is_derived_instance<TypeNodeBase, ArrayTypeNode>(object_type)) {
    if (PrimTypeNode* primitive = get_derived_instance<TypeNodeBase, PrimTypeNode>(index_type)) {
      if (primitive->type == Int) {
        register_t reg = alloc_register(ctx);
        resolve_rvalue(this, index_node.index, reg);
        bytecode_emit(ctx, GETARR, {dst, obj_reg, reg});
        free_register(ctx, reg);
        return;
      }
    }

    // Error: "ill-array-subscript"
    auto message = std::format("Subscripting array with type {}", index_type->to_output_string());
    compiler_error(ctx, index_node.index->begin, index_node.index->end, message);
    compiler_output_end(ctx);
  }
  else {
    // Error: "ill-subscript"
    auto message =
      std::format("lvalue of type {} is not subscriptable", object_type->to_output_string());
    compiler_error(ctx, index_node.object->begin, index_node.object->end, message);
    compiler_output_end(ctx);
  }
}

void ExprNodeVisitor::visit(BinExprNode& binary_node, operand_t dst) {
  using enum TokenType;

  static const std::unordered_map<TokenType, IOpCode> operator_map = {
    {OP_ADD, IOpCode::ADD},
    {OP_SUB, IOpCode::SUB},
    {OP_MUL, IOpCode::MUL},
    {OP_DIV, IOpCode::DIV},
    {OP_EXP, IOpCode::POW},
    {OP_MOD, IOpCode::MOD},
    {OP_EQ, IOpCode::EQ},
    {OP_NEQ, IOpCode::NEQ},
    {OP_LT, IOpCode::LT},
    {OP_GT, IOpCode::GT},
    {OP_LEQ, IOpCode::LTEQ},
    {OP_GEQ, IOpCode::GTEQ},
    {KW_AND, IOpCode::AND},
    {KW_OR, IOpCode::OR},
  };

  ExprNodeBase* lhs = binary_node.lhs_expression;
  ExprNodeBase* rhs = binary_node.rhs_expression;

  auto op_it = operator_map.find(binary_node.op.type);
  if (op_it == operator_map.end()) {
    // Error: "unknown-operator"
    auto message = std::format("Unknown binary operator '{}'", binary_node.op.lexeme);
    compiler_error(ctx, binary_node.op, message);
    compiler_output_end(ctx);
    return;
  }

  // Infer types
  TypeNodeBase* left_type = resolve_type(ctx, lhs);
  TypeNodeBase* right_type = resolve_type(ctx, rhs);

  CHECK_INFERED_TYPE(left_type, binary_node.lhs_expression);
  CHECK_INFERED_TYPE(right_type, binary_node.rhs_expression);

  if (!is_compatible(left_type, right_type)) {
    // Error: "bin-op-incompatible-types"
    auto message = std::format(
      "Binary operation on incompatible types '{}' (left) and '{}' (right)",
      left_type->to_output_string(),
      right_type->to_output_string()
    );
    compiler_error(ctx, binary_node.begin, binary_node.end, message);
    compiler_output_end(ctx);
    return;
  }

  const IOpCode base_opcode = op_it->second;
  const OpCodeId base_opcode_id = static_cast<OpCodeId>(base_opcode);
  OpCodeId opcode_id = base_opcode_id;

  bool is_left_constexpr = is_constant_expression(ctx.unit_ctx, lhs);
  bool is_right_constexpr = is_constant_expression(ctx.unit_ctx, rhs);

  // For Bool/relational operations, always handle as non-constant.
  bool is_bool_or_relational = base_opcode == AND || base_opcode == OR || base_opcode == LT
    || base_opcode == GT || base_opcode == LTEQ || base_opcode == GTEQ;

  if (is_left_constexpr && is_right_constexpr && !is_bool_or_relational) {
    // Constant folding is an O1 optimization.
    if (ctx.unit_ctx.optimization_level < 1) {
      goto non_constexpr;
    }

    LitExprNode folded_constant = fold_constant(ctx, &binary_node);
    resolve_rvalue(this, &folded_constant, dst);
  }
  else if (is_right_constexpr && !is_bool_or_relational) {
    if (ctx.unit_ctx.optimization_level < 1) {
      goto non_constexpr;
    }

    LitExprNode literal = fold_constant(ctx, rhs);

    // Special handling for DIV: check for division by zero.
    if (base_opcode == DIV) {
      if (int* int_val = std::get_if<int>(&literal.value)) {
        if VIA_LIKELY (*int_val != 0) {
          goto not_div_by_zero;
        }
      }
      else if (float* float_val = std::get_if<float>(&literal.value)) {
        if VIA_LIKELY (*float_val != 0.0f) {
          goto not_div_by_zero;
        }
      }

      { // Error: "explicit-division-by-zero"
        auto message = "Explicit division by zero";
        compiler_error(ctx, literal.value_token, message);
        compiler_output_end(ctx);
        return;
      }
    not_div_by_zero:
    }

    // Emit code for the constant-case.
    resolve_rvalue(this, lhs, dst);

    // Special handling for Bool operations.
    if (base_opcode == AND || base_opcode == OR) {
      bool is_rhs_falsy = false;
      if (bool* rhs_bool = std::get_if<bool>(&literal.value)) {
        is_rhs_falsy = !(*rhs_bool);
      }
      else if (std::get_if<std::monostate>(&literal.value)) {
        is_rhs_falsy = true;
      }

      if (base_opcode == AND && is_rhs_falsy) {
        bytecode_emit(ctx, LOADBF, {dst});
      }
      else if (base_opcode == OR && !is_rhs_falsy) {
        bytecode_emit(ctx, LOADBT, {dst});
      }
      return;
    }

    // Handle numeric constant: Int or float.
    if (int* int_value = std::get_if<int>(&literal.value)) {
      IOpCode opc = static_cast<IOpCode>(opcode_id + 1); // OPI for Int
      uint32_t final_value = static_cast<uint32_t>(*int_value);
      auto operands = reinterpret_u32_as_2u16(final_value);
      bytecode_emit(ctx, opc, {dst, operands.high, operands.low});
    }
    else if (float* float_value = std::get_if<float>(&literal.value)) {
      IOpCode opc = static_cast<IOpCode>(opcode_id + 2); // OPF for float
      uint32_t final_value = std::bit_cast<uint32_t>(*float_value);
      auto operands = reinterpret_u32_as_2u16(final_value);
      bytecode_emit(ctx, opc, {dst, operands.high, operands.low});
    }
  }
  else {
  non_constexpr:
    // Non-constant expression or Bool/relational operator.
    operand_t reg = alloc_register(ctx);

    // Evaluate expressions based on operator precedence.
    if (rhs->precedence() > lhs->precedence()) {
      resolve_rvalue(this, rhs, dst);
      resolve_rvalue(this, lhs, reg);
    }
    else {
      resolve_rvalue(this, lhs, reg);
      resolve_rvalue(this, rhs, dst);
    }

    if (is_bool_or_relational) {
      operand_t left_reg = alloc_register(ctx);
      bytecode_emit(ctx, MOV, {left_reg, dst});
      bytecode_emit(ctx, base_opcode, {dst, left_reg, reg});
      free_register(ctx, reg);
    }
    else {
      bytecode_emit(ctx, base_opcode, {dst, reg});
      free_register(ctx, reg);
    }
  }
}

void ExprNodeVisitor::visit(CastExprNode& type_cast, operand_t dst) {
  TypeNodeBase* left_type = type_cast.expression->infer_type(ctx.unit_ctx);

  CHECK_INFERED_TYPE(left_type, type_cast.expression);

  if (!is_castable(left_type, type_cast.type)) {
    // Error: "ill-explicit-cast"
    auto message = std::format(
      "Expression of type {} is not castable into type {}",
      left_type->to_output_string(),
      type_cast.type->to_output_string()
    );
    compiler_error(ctx, type_cast.expression->begin, type_cast.expression->end, message);
    compiler_output_end(ctx);
  }

  operand_t temp = alloc_register(ctx);
  resolve_rvalue(this, type_cast.expression, temp);

  if (PrimTypeNode* primitive = get_derived_instance<TypeNodeBase, PrimTypeNode>(type_cast.type)) {
    if (primitive->type == Int) {
      bytecode_emit(ctx, ICAST, {dst, temp});
    }
    else if (primitive->type == Float) {
      bytecode_emit(ctx, FCAST, {dst, temp});
    }
    else if (primitive->type == String) {
      bytecode_emit(ctx, STRCAST, {dst, temp});
    }
    else if (primitive->type == Bool) {
      bytecode_emit(ctx, BCAST, {dst, temp});
    }
  }

  free_register(ctx, temp);
}

void ExprNodeVisitor::visit(StepExprNode& step_expr, operand_t dst) {
  auto opcode = step_expr.is_increment ? INC : DEC;
  operand_t temp = alloc_register(ctx);
  resolve_lvalue(ctx, step_expr.target, dst);
  bytecode_emit(ctx, MOV, {temp, dst});
  bytecode_emit(ctx, opcode, {temp});
  bind_lvalue(ctx, step_expr.target, temp);
  free_register(ctx, temp);
}

void ExprNodeVisitor::visit(ArrayExprNode& array_expr, operand_t dst) {
  bytecode_emit(ctx, LOADARR, {dst});

  register_t key_reg = alloc_register(ctx);
  register_t val_reg = alloc_register(ctx);

  for (u32 i = 0; ExprNodeBase * expr : array_expr.values) {
    u16_result result = reinterpret_u32_as_2u16(i++);
    resolve_rvalue(this, expr, val_reg);
    bytecode_emit(ctx, LOADI, {key_reg, result.high, result.low});
    bytecode_emit(ctx, SETARR, {val_reg, dst, key_reg});
  }

  free_register(ctx, val_reg);
  free_register(ctx, key_reg);
}

} // namespace via
