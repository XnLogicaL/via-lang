// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "state.h"
#include "bit-utility.h"
#include "stack.h"
#include "string-utility.h"
#include "compiler-types.h"
#include "visitor.h"

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
// corresponding opcode(s), and then determining the operands via the built-in node parameters.
//
// - lit_expr_node compilation:
//  This node only emits `LOAD` opcodes, and is considered a constant expression.
//  It first checks for primitive data types within the node, and emits corresponding bytecode based
//  on that. If it finds complex data types like strings, tables, etc., it loads them into the
//  constant table and emits a `LOADK` instruction with the corresponding constant id.
//
// - sym_expr_node compilation:
//  This node represents a "symbol" that is either a local, global, argument or upvalue.
//  It first checks the stack for the symbol, if found, emits a `GETSTACK` instruction with the
//  stack id of the symbol. After that, it checks for upvalues, if found emits `GETUPVALUE`. Next,
//  it checks for arguments by traversing the parameters of the top function in
//  `unit_ctx::internal.stack::function_stack` and looking for the symbol. If found, emits
//  `GETARGUMENT`. Finally, looks for the variable in the global scope by querying
//  `unit_ctx::internal::globals` and if found emits GETGLOBAL. If all of these queries fail, throws
//  a "Use of undeclared variable" compilation error.
//
// - unary_expr_node compilation:
//  This node emits a NEG instruction onto the inner expression.
//
// - grp_expr_node compilation:
//  Compiles the inner expression into dst.
//
// - call_expr_node compilation:
//  This node represents a function call expression, which first loads the arguments onto the stack
//  (LIFO), loads the callee object, and calls it. And finally, emits a POP instruction to retrieve
//  the return value.
//
// - index_expr_node compilation:
//  This node represents a member access, which could follow either of these patterns:
//    -> Direct table member access: table.index
//      This pattern compiles into a GETTABLE instruction that uses the hashed version of the index.
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

using enum opcode;

void expr_node_visitor::visit(lit_expr_node& literal_node, operand_t dst) {
  using enum value_type;

  if (int* integer_value = std::get_if<int>(&literal_node.value)) {
    uint32_t final_value = *integer_value;
    auto operands = reinterpret_u32_as_2u16(final_value);

    unit_ctx.bytecode->emit(LOADINT, {dst, operands.l, operands.r});
  }
  else if (float* float_value = std::get_if<float>(&literal_node.value)) {
    uint32_t final_value = std::bit_cast<uint32_t>(*float_value);
    auto operands = reinterpret_u32_as_2u16(final_value);

    unit_ctx.bytecode->emit(LOADFLOAT, {dst, operands.l, operands.r});
  }
  else if (bool* bool_value = std::get_if<bool>(&literal_node.value)) {
    unit_ctx.bytecode->emit(*bool_value ? LOADTRUE : LOADFALSE, {dst});
  }
  else {
    const value_obj& constant = construct_constant(literal_node);
    const operand_t constant_id = unit_ctx.constants->push_constant(constant);

    unit_ctx.bytecode->emit(LOADK, {dst, constant_id});
  }
}

void expr_node_visitor::visit(sym_expr_node& variable_node, operand_t dst) {
  token var_id = variable_node.identifier;

  std::string symbol = var_id.lexeme;
  std::optional<operand_t> stk_id = unit_ctx.internal.stack->find_symbol(var_id.lexeme);

  if (stk_id.has_value()) {
    if (unit_ctx.internal.stack->function_stack.size() > 0) {
      auto& current_closure = unit_ctx.internal.stack->function_stack.top();
      if (current_closure.upvalues > stk_id.value()) {
        unit_ctx.bytecode->emit(GETUPVALUE, {dst, stk_id.value()}, symbol);
      }
    }
    else {
      unit_ctx.bytecode->emit(GETSTACK, {dst, stk_id.value()}, symbol);
    }
  }
  else if (unit_ctx.internal.globals->was_declared(symbol)) {
    uint32_t symbol_hash = hash_string_custom(symbol.c_str());
    auto operands = reinterpret_u32_as_2u16(symbol_hash);

    unit_ctx.bytecode->emit(GETGLOBAL, {dst, operands.l, operands.r}, symbol);
  }
  else if (!unit_ctx.internal.stack->function_stack.empty()) {
    uint16_t index = 0;
    auto& top = unit_ctx.internal.stack->function_stack.top();

    for (const auto& parameter : top.parameters) {
      if (parameter.identifier.lexeme == symbol) {
        unit_ctx.bytecode->emit(GETARGUMENT, {dst, index});
        return;
      }

      ++index;
    }
  }
  else {
    compiler_error(var_id, std::format("Use of undeclared identifier '{}'", var_id.lexeme));
  }
}

void expr_node_visitor::visit(unary_expr_node& unary_node, operand_t dst) {
  unary_node.accept(*this, dst);
  unit_ctx.bytecode->emit(NEG, {dst});
}

void expr_node_visitor::visit(grp_expr_node& group_node, operand_t dst) {
  group_node.accept(*this, dst);
}

void expr_node_visitor::visit(call_expr_node& call_node, operand_t dst) {
  operand_t argc = call_node.arguments.size();
  operand_t callee_reg = allocator.allocate_register();

  p_expr_node_t& callee = call_node.callee;
  p_type_node_t callee_type = call_node.callee->infer_type(unit_ctx);

  vl_tinference_failure(callee_type, callee);

  if (FunctionTypeNode* fn_ty =
        get_derived_instance<type_node_base, FunctionTypeNode>(*callee_type)) {
    size_t expected_argc = fn_ty->parameters.size();
    if (argc != static_cast<operand_t>(expected_argc)) {
      compiler_error(
        call_node.begin,
        call_node.end,
        std::format("Function type expects {} arguments, got {}", expected_argc, argc)
      );
    }
  }
  else {
    compiler_error(
      callee->begin,
      callee->end,
      std::format("Value of type '{}' is not callable", callee_type->to_string_x())
    );
  }

  call_node.callee->accept(*this, callee_reg);

  for (const p_expr_node_t& argument : call_node.arguments) {
    operand_t argument_reg = allocator.allocate_register();

    if (lit_expr_node* literal_node =
          get_derived_instance<expr_node_base, lit_expr_node>(*argument)) {
      if (int* integer_value = std::get_if<int>(&literal_node->value)) {
        uint32_t final_value = *integer_value;
        auto operands = reinterpret_u32_as_2u16(final_value);

        unit_ctx.bytecode->emit(PUSHINT, {operands.l, operands.r});
      }
      else if (float* float_value = std::get_if<float>(&literal_node->value)) {
        uint32_t final_value = std::bit_cast<uint32_t>(*float_value);
        auto operands = reinterpret_u32_as_2u16(final_value);

        unit_ctx.bytecode->emit(PUSHFLOAT, {operands.l, operands.r});
      }
      else if (bool* bool_value = std::get_if<bool>(&literal_node->value)) {
        unit_ctx.bytecode->emit(*bool_value ? PUSHTRUE : PUSHFALSE);
      }
      else {
        const value_obj& constant = construct_constant(*literal_node);
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

void expr_node_visitor::visit(index_expr_node& index_node, operand_t dst) {
  operand_t obj_reg = allocator.allocate_register();
  operand_t index_reg = allocator.allocate_register();

  index_node.object->accept(*this, obj_reg);
  index_node.index->accept(*this, index_reg);

  p_type_node_t object_type = index_node.object->infer_type(unit_ctx);
  p_type_node_t index_type = index_node.index->infer_type(unit_ctx);

  // Validate index type
  if (auto* primitive = get_derived_instance<type_node_base, primitive_type_node>(*index_type)) {
    if (primitive->type != value_type::string && primitive->type != value_type::integer) {
      compiler_error(
        index_node.index->begin, index_node.index->end, "Index type must be a string or integer"
      );
      return;
    }
  }

  // Handle different object types
  if (auto* primitive = get_derived_instance<type_node_base, primitive_type_node>(*object_type)) {
    switch (primitive->type) {
    case value_type::string:
      unit_ctx.bytecode->emit(GETSTRING, {dst, obj_reg, index_reg});
      break;
    case value_type::table:
      unit_ctx.bytecode->emit(GETTABLE, {dst, obj_reg, index_reg});
      break;
    default:
      compiler_error(
        index_node.object->begin, index_node.object->end, "Expression type is not subscriptable"
      );
    }
  }
}

void expr_node_visitor::visit(bin_expr_node& binary_node, operand_t dst) {
  using enum token_type;
  using OpCodeId = std::underlying_type_t<opcode>;

  static const std::unordered_map<token_type, opcode> operator_map = {
    {token_type::OP_ADD, opcode::ADD},
    {token_type::OP_SUB, opcode::SUB},
    {token_type::OP_MUL, opcode::MUL},
    {token_type::OP_DIV, opcode::DIV},
    {token_type::OP_EXP, opcode::POW},
    {token_type::OP_MOD, opcode::MOD},
    {token_type::OP_EQ, opcode::EQUAL},
    {token_type::OP_NEQ, opcode::NOTEQUAL},
    {token_type::OP_LT, opcode::LESS},
    {token_type::OP_GT, opcode::GREATER},
    {token_type::OP_LEQ, opcode::LESSOREQUAL},
    {token_type::OP_GEQ, opcode::GREATEROREQUAL},
    {token_type::KW_AND, opcode::AND},
    {token_type::KW_OR, opcode::OR},
  };

  p_expr_node_t& p_lhs = binary_node.lhs_expression;
  p_expr_node_t& p_rhs = binary_node.rhs_expression;

  expr_node_base& lhs = *p_lhs;
  expr_node_base& rhs = *p_rhs;

  auto it = operator_map.find(binary_node.op.type);
  if (it == operator_map.end()) {
    compiler_error(
      binary_node.op, std::format("Unknown binary operator '{}'", binary_node.op.lexeme)
    );
    return;
  }

  p_type_node_t left_type = p_lhs->infer_type(unit_ctx);
  p_type_node_t right_type = p_rhs->infer_type(unit_ctx);

  vl_tinference_failure(left_type, binary_node.lhs_expression);
  vl_tinference_failure(right_type, binary_node.rhs_expression);

  if (!is_compatible(left_type, right_type)) {
    compiler_error(
      binary_node.begin,
      binary_node.end,
      std::format(
        "Binary operation on incompatible types '{}' (left) and '{}' (right)",
        left_type->to_string_x(),
        right_type->to_string_x()
      )
    );
    return;
  }

  const opcode base_opcode = it->second;
  const OpCodeId base_opcode_id = static_cast<OpCodeId>(base_opcode);
  OpCodeId opcode_id = base_opcode_id;

  if (is_constant_expression(rhs)) {
    lit_expr_node& literal = dynamic_cast<lit_expr_node&>(rhs);

    if (base_opcode == DIV) {
      if (TInteger* int_val = std::get_if<TInteger>(&literal.value)) {
        if (*int_val == 0) {
          goto division_by_zero;
        }
      }

      if (TFloat* float_val = std::get_if<TFloat>(&literal.value)) {
        if (*float_val == 0.0f) {
          goto division_by_zero;
        }
      }

      goto good_division;

    division_by_zero:
      compiler_error(literal.value_token, "Explicit division by zero");
      return;

    good_division:
    }

    lhs.accept(*this, dst);

    if (base_opcode == AND || base_opcode == OR) {
      bool is_rhs_falsy = ({
        bool* rhs_bool = std::get_if<bool>(&literal.value);
        auto* rhs_nil = std::get_if<std::monostate>(&literal.value);
        rhs_bool != nullptr ? !(*rhs_bool) : rhs_nil != nullptr;
      });

      if (base_opcode == AND && is_rhs_falsy) {
        unit_ctx.bytecode->emit(LOADFALSE, {dst});
      }

      if (base_opcode == OR && !is_rhs_falsy) {
        unit_ctx.bytecode->emit(LOADTRUE, {dst});
      }

      return;
    }

    if (int* int_value = std::get_if<int>(&literal.value)) {
      opcode opc = static_cast<opcode>(opcode_id + 2); // OPINT
      uint32_t final_value = *int_value;
      auto operands = reinterpret_u32_as_2u16(final_value);

      unit_ctx.bytecode->emit(opc, {dst, operands.l, operands.r});
    }
    else if (float* float_value = std::get_if<float>(&literal.value)) {
      opcode opc = static_cast<opcode>(opcode_id + 3); // OPFLOAT
      uint32_t final_value = std::bit_cast<uint32_t>(*float_value);
      auto operands = reinterpret_u32_as_2u16(final_value);

      unit_ctx.bytecode->emit(opc, {dst, operands.l, operands.r});
    }
    else {
      opcode opc = static_cast<opcode>(opcode_id + 1); // OPK
      value_obj right_const_val = construct_constant(dynamic_cast<lit_expr_node&>(rhs));
      operand_t right_const_id = unit_ctx.constants->push_constant(right_const_val);

      unit_ctx.bytecode->emit(opc, {dst, right_const_id});
    }
  }
  else {
    operand_t reg = allocator.allocate_register();

    if (rhs.precedence() > lhs.precedence()) {
      rhs.accept(*this, dst);
      lhs.accept(*this, reg);
    }
    else {
      lhs.accept(*this, dst);
      rhs.accept(*this, reg);
    }

    unit_ctx.bytecode->emit(base_opcode, {dst, reg});
    allocator.free_register(reg);
  }
}

void expr_node_visitor::visit(cast_expr_node& type_cast, operand_t dst) {
  p_type_node_t left_type = type_cast.expression->infer_type(unit_ctx);

  vl_tinference_failure(left_type, type_cast.expression);

  if (!is_castable(left_type, type_cast.type)) {
    compiler_error(
      type_cast.expression->begin,
      type_cast.expression->end,
      std::format(
        "Expression of type '{}' can not be casted into type '{}'",
        left_type->to_string_x(),
        type_cast.type->to_string_x()
      )
    );
  }

  operand_t temp = allocator.allocate_register();
  type_cast.expression->accept(*this, temp);

  if (primitive_type_node* primitive =
        get_derived_instance<type_node_base, primitive_type_node>(*type_cast.type)) {
    if (primitive->type == value_type::integer) {
      unit_ctx.bytecode->emit(INTCAST, {dst, temp});
    }
    else if (primitive->type == value_type::floating_point) {
      unit_ctx.bytecode->emit(FLOATCAST, {dst, temp});
    }
    else if (primitive->type == value_type::string) {
      unit_ctx.bytecode->emit(STRINGCAST, {dst, temp});
    }
    else if (primitive->type == value_type::boolean) {
      unit_ctx.bytecode->emit();
    }
  }

  allocator.free_register(temp);
}

} // namespace via
