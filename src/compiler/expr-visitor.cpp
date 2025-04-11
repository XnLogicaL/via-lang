//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================
#include "state.h"
#include "bit-utility.h"
#include "stack.h"
#include "string-utility.h"
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
//  This node represents a "symbol" that is either a local, global, argument or IUpValue.
//  It first checks the stack for the symbol, if found, emits a `STKGET` instruction with the
//  stack id of the symbol. After that, it checks for upvalues, if found emits `UPVGET`. Next,
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
//      This pattern first compiles the index expression, then casts it into a string, and finally
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

void ExprNodeVisitor::visit(LitExprNode& literal_node, operand_t dst) {
  using enum IValueType;

  if (int* integer_value = std::get_if<int>(&literal_node.value)) {
    uint32_t final_value = *integer_value;
    auto operands = reinterpret_u32_as_2u16(final_value);

    unit_ctx.bytecode->emit(LOADI, {dst, operands.high, operands.low});
  }
  else if (float* float_value = std::get_if<float>(&literal_node.value)) {
    uint32_t final_value = std::bit_cast<uint32_t>(*float_value);
    auto operands = reinterpret_u32_as_2u16(final_value);

    unit_ctx.bytecode->emit(LOADF, {dst, operands.high, operands.low});
  }
  else if (bool* bool_value = std::get_if<bool>(&literal_node.value)) {
    unit_ctx.bytecode->emit(*bool_value ? LOADBT : LOADBF, {dst});
  }
  else {
    const IValue& constant = construct_constant(literal_node);
    const operand_t constant_id = unit_ctx.constants->push_constant(constant);

    unit_ctx.bytecode->emit(LOADK, {dst, constant_id});
  }
}

void ExprNodeVisitor::visit(SymExprNode& variable_node, operand_t dst) {
  Token var_id = variable_node.identifier;

  std::string symbol = var_id.lexeme;
  std::optional<operand_t> stk_id = unit_ctx.internal.variable_stack->find_symbol(var_id.lexeme);

  if (stk_id.has_value()) {
    if (unit_ctx.internal.function_stack->size() > 0) {
      auto& current_closure = unit_ctx.internal.function_stack->top();
      if (current_closure.stack_pointer > stk_id.value()) {
        unit_ctx.bytecode->emit(UPVGET, {dst, stk_id.value()}, symbol);
        return;
      }
    }

    unit_ctx.bytecode->emit(STKGET, {dst, stk_id.value()}, symbol);
    return;
  }
  else if (unit_ctx.internal.globals->was_declared(symbol)) {
    LitExprNode lit_translation = LitExprNode(variable_node.identifier, symbol);
    IValue const_val = construct_constant(lit_translation);
    operand_t const_id = unit_ctx.constants->push_constant(const_val);
    operand_t tmp_reg = allocator.allocate_temp();

    unit_ctx.bytecode->emit(LOADK, {tmp_reg, const_id});
    unit_ctx.bytecode->emit(GGET, {dst, tmp_reg}, symbol);
    return;
  }
  else if (unit_ctx.internal.function_stack->size() > 0) {
    uint16_t index = 0;
    auto& top = unit_ctx.internal.function_stack->top();

    for (const auto& parameter : top.decl->parameters) {
      if (parameter.identifier.lexeme == symbol) {
        unit_ctx.bytecode->emit(ARGGET, {dst, index});
        return;
      }

      ++index;
    }
  }

  compiler_error(var_id, std::format("Use of undeclared identifier '{}'", var_id.lexeme));
  compiler_output_end();
}

void ExprNodeVisitor::visit(UnaryExprNode& unary_node, operand_t dst) {
  TypeNodeBase* type = unary_node.expression->infer_type(unit_ctx);
  unary_node.expression->accept(*this, dst);

  if (unary_node.op.type == TokenType::OP_SUB) {
    if (is_arithmetic(type)) {
      unit_ctx.bytecode->emit(NEG, {dst});
    }
    else {
      compiler_error(
        unary_node.begin,
        unary_node.end,
        std::format("Negating non-negatable type {}", type->to_output_string())
      );
      compiler_output_end();
    }
  }
  else if (unary_node.op.type == TokenType::OP_LEN) {
    if (is_derived_instance<TypeNodeBase, ArrayTypeNode>(type)) {
      register_t reg = allocator.allocate_register();
      unit_ctx.bytecode->emit(MOV, {reg, dst});
      unit_ctx.bytecode->emit(ARRLEN, {dst, reg});
      return;
    }

    compiler_error(
      unary_node.begin,
      unary_node.end,
      std::format("Taking length of unbounded type {}", type->to_output_string())
    );
    compiler_output_end();
  }
  else if (unary_node.op.type == TokenType::OP_INC || unary_node.op.type == TokenType::OP_DEC) {
    IOpCode opcode = unary_node.op.type == TokenType::OP_INC ? INC : DEC;

    if (!is_arithmetic(type)) {
      compiler_error(unary_node.begin, unary_node.end, "Stepping non-arithmetic data type");
      compiler_output_end();
      return;
    }

    unit_ctx.bytecode->emit(opcode, {dst});
  }
}

void ExprNodeVisitor::visit(GroupExprNode& group_node, operand_t dst) {
  group_node.expression->accept(*this, dst);
}

void ExprNodeVisitor::visit(CallExprNode& call_node, operand_t dst) {
  operand_t argc = call_node.arguments.size();
  operand_t callee_reg = allocator.allocate_register();

  ExprNodeBase*& callee = call_node.callee;
  TypeNodeBase* callee_type = call_node.callee->infer_type(unit_ctx);

  VIA_TINFERENCE_FAILURE(callee_type, callee);

  if (FunctionTypeNode* fn_ty = get_derived_instance<TypeNodeBase, FunctionTypeNode>(callee_type)) {
    size_t expected_argc = fn_ty->parameters.size();
    if (argc != static_cast<operand_t>(expected_argc)) {
      compiler_error(
        call_node.begin,
        call_node.end,
        std::format("Function type expects {} arguments, got {}", expected_argc, argc)
      );
      compiler_output_end();
    }
  }
  else {
    compiler_error(
      callee->begin,
      callee->end,
      std::format("Value of type '{}' is not callable", callee_type->to_output_string())
    );
    compiler_output_end();
  }

  call_node.callee->accept(*this, callee_reg);

  for (ExprNodeBase*& argument : call_node.arguments) {
    operand_t argument_reg = allocator.allocate_register();

    if (LitExprNode* literal_node = get_derived_instance<ExprNodeBase, LitExprNode>(argument)) {
      if (int* integer_value = std::get_if<int>(&literal_node->value)) {
        uint32_t final_value = *integer_value;
        auto operands = reinterpret_u32_as_2u16(final_value);

        unit_ctx.bytecode->emit(PUSHI, {operands.high, operands.low});
      }
      else if (float* float_value = std::get_if<float>(&literal_node->value)) {
        uint32_t final_value = std::bit_cast<uint32_t>(*float_value);
        auto operands = reinterpret_u32_as_2u16(final_value);

        unit_ctx.bytecode->emit(PUSHF, {operands.high, operands.low});
      }
      else if (bool* bool_value = std::get_if<bool>(&literal_node->value)) {
        unit_ctx.bytecode->emit(*bool_value ? PUSHBT : PUSHBF);
      }
      else {
        const IValue& constant = construct_constant(*literal_node);
        const operand_t constant_id = unit_ctx.constants->push_constant(constant);

        unit_ctx.bytecode->emit(PUSHK, {constant_id});
      }
    }
    else {
      argument->accept(*this, argument_reg);
      unit_ctx.bytecode->emit(PUSH, {argument_reg});
      allocator.free_register(argument_reg);
    }
  }

  unit_ctx.bytecode->emit(CALL, {callee_reg, argc});
  unit_ctx.bytecode->emit(POP, {dst});
  allocator.free_register(callee_reg);
}

void ExprNodeVisitor::visit(IndexExprNode& index_node, operand_t dst) {
  size_t begin = 0, end = 0;
  operand_t obj_reg = allocator.allocate_register();

  index_node.object->accept(*this, obj_reg);

  if (SymExprNode* sym_expr = get_derived_instance<ExprNodeBase, SymExprNode>(index_node.object)) {
    auto id = unit_ctx.internal.variable_stack->find_symbol(sym_expr->identifier.lexeme);
    if (id.has_value()) {
      auto obj = unit_ctx.internal.variable_stack->at(*id);
      begin = obj->decl->begin;
      end = obj->decl->end;
    }
  }

  TypeNodeBase* object_type = index_node.object->infer_type(unit_ctx);
  TypeNodeBase* index_type = index_node.index->infer_type(unit_ctx);

  if (is_derived_instance<TypeNodeBase, ArrayTypeNode>(object_type)) {
    if (PrimTypeNode* primitive = get_derived_instance<TypeNodeBase, PrimTypeNode>(index_type)) {
      if (primitive->type == IValueType::integer) {
        register_t reg = allocator.allocate_register();
        index_node.index->accept(*this, reg);
        unit_ctx.bytecode->emit(ARRGET, {dst, obj_reg, reg});
        allocator.free_register(reg);
        return;
      }
    }

    compiler_error(
      index_node.index->begin,
      index_node.index->end,
      std::format("Subscripting array with type {}", index_type->to_output_string())
    );
    compiler_output_end();
  }
  else {
    compiler_error(
      index_node.object->begin,
      index_node.object->end,
      std::format("lvalue of type {} is not subscriptable", object_type->to_output_string())
    );

    if (begin != 0 && end != 0) {
      compiler_info(
        begin, end, std::format("Declared as {} here", object_type->to_output_string())
      );
    }

    compiler_output_end();
  }
}

void ExprNodeVisitor::visit(BinExprNode& binary_node, operand_t dst) {
  using enum TokenType;
  using OpCodeId = std::underlying_type_t<IOpCode>;

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

  ExprNodeBase* p_lhs = binary_node.lhs_expression;
  ExprNodeBase* p_rhs = binary_node.rhs_expression;

  auto op_it = operator_map.find(binary_node.op.type);
  if (op_it == operator_map.end()) {
    compiler_error(
      binary_node.op, std::format("Unknown binary operator '{}'", binary_node.op.lexeme)
    );
    compiler_output_end();
    return;
  }

  // Infer types
  TypeNodeBase* left_type = p_lhs->infer_type(unit_ctx);
  TypeNodeBase* right_type = p_rhs->infer_type(unit_ctx);
  VIA_TINFERENCE_FAILURE(left_type, binary_node.lhs_expression);
  VIA_TINFERENCE_FAILURE(right_type, binary_node.rhs_expression);

  if (!is_compatible(left_type, right_type)) {
    compiler_error(
      binary_node.begin,
      binary_node.end,
      std::format(
        "Binary operation on incompatible types '{}' (left) and '{}' (right)",
        left_type->to_output_string(),
        right_type->to_output_string()
      )
    );
    compiler_output_end();
    return;
  }

  const IOpCode base_opcode = op_it->second;
  const OpCodeId base_opcode_id = static_cast<OpCodeId>(base_opcode);
  OpCodeId opcode_id = base_opcode_id;

  // For boolean/relational operations, always handle as non-constant.
  bool is_bool_or_relational =
    (base_opcode == AND || base_opcode == OR || base_opcode == LT || base_opcode == GT
     || base_opcode == LTEQ || base_opcode == GTEQ);

  if (is_constant_expression(unit_ctx, p_rhs) && is_constant_expression(unit_ctx, p_lhs)
      && !is_bool_or_relational) {
    // Constant folding is an O1 optimization.
    if (unit_ctx.optimization_level < 1) {
      goto non_constexpr;
    }

    LitExprNode folded_constant = fold_constant(binary_node);
    folded_constant.accept(*this, dst);
  }
  else if (is_constant_expression(unit_ctx, p_rhs) && !is_bool_or_relational) {
    if (unit_ctx.optimization_level < 1) {
      goto non_constexpr;
    }

    LitExprNode literal = fold_constant(*p_rhs);

    // Special handling for DIV: check for division by zero.
    if (base_opcode == DIV) {
      if (int* int_val = std::get_if<int>(&literal.value)) {
        if (*int_val == 0) {
          compiler_error(literal.value_token, "Explicit division by zero");
          compiler_output_end();
          return;
        }
      }
      if (float* float_val = std::get_if<float>(&literal.value)) {
        if (*float_val == 0.0f) {
          compiler_error(literal.value_token, "Explicit division by zero");
          compiler_output_end();
          return;
        }
      }
    }

    // Emit code for the constant-case.
    p_lhs->accept(*this, dst);

    // Special handling for boolean operations.
    if (base_opcode == AND || base_opcode == OR) {
      bool is_rhs_falsy = false;
      if (bool* rhs_bool = std::get_if<bool>(&literal.value)) {
        is_rhs_falsy = !(*rhs_bool);
      }
      else if (std::get_if<std::monostate>(&literal.value)) {
        is_rhs_falsy = true;
      }

      if (base_opcode == AND && is_rhs_falsy) {
        unit_ctx.bytecode->emit(LOADBF, {dst});
      }
      else if (base_opcode == OR && !is_rhs_falsy) {
        unit_ctx.bytecode->emit(LOADBT, {dst});
      }
      return;
    }

    // Handle numeric constant: integer or float.
    if (int* int_value = std::get_if<int>(&literal.value)) {
      IOpCode opc = static_cast<IOpCode>(opcode_id + 1); // OPI for integer
      uint32_t final_value = static_cast<uint32_t>(*int_value);
      auto operands = reinterpret_u32_as_2u16(final_value);
      unit_ctx.bytecode->emit(opc, {dst, operands.high, operands.low});
    }
    else if (float* float_value = std::get_if<float>(&literal.value)) {
      IOpCode opc = static_cast<IOpCode>(opcode_id + 2); // OPF for float
      uint32_t final_value = std::bit_cast<uint32_t>(*float_value);
      auto operands = reinterpret_u32_as_2u16(final_value);
      unit_ctx.bytecode->emit(opc, {dst, operands.high, operands.low});
    }
  }
  else {
  non_constexpr:
    // Non-constant expression or boolean/relational operator.
    operand_t reg = allocator.allocate_register();

    // Evaluate expressions based on operator precedence.
    if (p_rhs->precedence() > p_lhs->precedence()) {
      p_rhs->accept(*this, dst);
      p_lhs->accept(*this, reg);
    }
    else {
      p_lhs->accept(*this, dst);
      p_rhs->accept(*this, reg);
    }

    if (is_bool_or_relational) {
      operand_t left_reg = allocator.allocate_register();
      unit_ctx.bytecode->emit(MOV, {left_reg, dst});
      unit_ctx.bytecode->emit(base_opcode, {dst, left_reg, reg});
      allocator.free_register(reg);
      return;
    }
    else {
      unit_ctx.bytecode->emit(base_opcode, {dst, reg});
      allocator.free_register(reg);
    }
  }
}

void ExprNodeVisitor::visit(CastExprNode& type_cast, operand_t dst) {
  TypeNodeBase* left_type = type_cast.expression->infer_type(unit_ctx);

  VIA_TINFERENCE_FAILURE(left_type, type_cast.expression);

  if (!is_castable(left_type, type_cast.type)) {
    compiler_error(
      type_cast.expression->begin,
      type_cast.expression->end,
      std::format(
        "Expression of type {} is not castable into type {}",
        left_type->to_output_string(),
        type_cast.type->to_output_string()
      )
    );
    compiler_output_end();
  }

  operand_t temp = allocator.allocate_register();
  type_cast.expression->accept(*this, temp);

  if (PrimTypeNode* primitive = get_derived_instance<TypeNodeBase, PrimTypeNode>(type_cast.type)) {
    if (primitive->type == IValueType::integer) {
      unit_ctx.bytecode->emit(CASTI, {dst, temp});
    }
    else if (primitive->type == IValueType::floating_point) {
      unit_ctx.bytecode->emit(CASTF, {dst, temp});
    }
    else if (primitive->type == IValueType::string) {
      unit_ctx.bytecode->emit(CASTSTR, {dst, temp});
    }
    else if (primitive->type == IValueType::boolean) {
      unit_ctx.bytecode->emit(CASTB, {dst, temp});
    }
  }

  allocator.free_register(temp);
}

void ExprNodeVisitor::visit(StepExprNode& step_expr, operand_t dst) {
  if (SymExprNode* symbol_node =
        get_derived_instance<ExprNodeBase, SymExprNode>(step_expr.target)) {
    Token symbol_token = symbol_node->identifier;
    std::string symbol = symbol_token.lexeme;
    std::optional<operand_t> stk_id = unit_ctx.internal.variable_stack->find_symbol(symbol);

    if (stk_id.has_value()) {
      const auto& test_stack_member = unit_ctx.internal.variable_stack->at(stk_id.value());
      if (test_stack_member.has_value() && test_stack_member->is_const) {
        compiler_error(symbol_token, std::format("Assignment to constant variable '{}'", symbol));
        compiler_output_end();
        return;
      }

      if (!is_arithmetic(test_stack_member->type)) {
        compiler_error(step_expr.begin, step_expr.end, "Stepping non-arithmetic datatype");
        compiler_output_end();
        return;
      }

      IOpCode opc = step_expr.is_increment ? INC : DEC;
      operand_t value_reg = allocator.allocate_register();
      step_expr.target->accept(*this, value_reg);

      unit_ctx.bytecode->emit(MOV, {dst, value_reg});
      unit_ctx.bytecode->emit(opc, {value_reg});
      unit_ctx.bytecode->emit(STKSET, {value_reg, *stk_id});
    }
    else {
      compiler_error(symbol_token, "Stepping invalid lvalue");
      compiler_info(std::format("Symbol '{}' not found in scope", symbol_node->identifier.lexeme));
      compiler_output_end();
    }
  }
  else {
    compiler_error(step_expr.target->begin, step_expr.target->end, "Stepping invalid lvalue");
    compiler_output_end();
  }
}

void ExprNodeVisitor::visit(ArrayExprNode& array_expr, operand_t dst) {
  unit_ctx.bytecode->emit(NEWARR, {dst});

  uint32_t i = 0;
  register_t key_reg = allocator.allocate_register();
  register_t val_reg = allocator.allocate_register();

  for (ExprNodeBase* expr : array_expr.values) {
    u16_result result = reinterpret_u32_as_2u16(i++);
    expr->accept(*this, val_reg);
    unit_ctx.bytecode->emit(LOADI, {key_reg, result.high, result.low});
    unit_ctx.bytecode->emit(ARRSET, {val_reg, dst, key_reg});
  }

  allocator.free_register(val_reg);
  allocator.free_register(key_reg);
}

} // namespace via
