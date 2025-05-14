// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "visitor.h"
#include "bytecode-builder.h"
#include <tarray.h>
#include <tdict.h>
#include <api-impl.h>
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
// corresponding Opcode(s), and then determining the operands via the built-in node parameters.
//
// - NodeLitExpr compilation:
//  This node only emits `LOAD` opcodes, and is considered a constant expression.
//  It first checks for primitive data types within the node, and emits corresponding bytecode based
//  on that. If it finds complex data types like strings, tables, etc., it loads them into the
//  constant table and emits a `LOADK` instruction with the corresponding constant id.
//
// - NodeSymExpr compilation:
//  This node represents a "symbol" that is either a local, global, argument or UpValue.
//  It first checks the stack for the symbol, if found, emits a `STKGET` instruction with the
//  stack id of the symbol. After that, it checks for upvalues, if found emits `GETUPV`. Next,
//  it checks for arguments by traversing the parameters of the top function in
//  `lctx::variable_stack::function_stack` and looking for the symbol. If found, emits
//  `ARGGET`. Finally, looks for the variable in the global scope by querying
//  `lctx:::globals` and if found emits GGET. If all of these queries fail, throws
//  a "Use of undeclared variable" compilation error.
//
// - NodeUnExpr compilation:
//
//
// - NodeGroupExpr compilation:
//  Compiles the inner expression into dst.
//
// - NodeCallExpr compilation:
//  This node represents a function call expression, which first loads the arguments onto the stack
//  (LIFO), loads the callee object, and calls it. And finally, emits a POP instruction to retrieve
//  the return value.
//
// - NodeIndexExpr compilation:
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

using enum Opcode;
using enum Value::Tag;
using enum TokenType;
using OpCodeId = std::underlying_type_t<Opcode>;

void ExprNodeVisitor::visit(AstNode*, NodeLitExpr& lit_expr, operand_t dst) {
  if (lit_expr.kind == Int) {
    auto operands = ubit_u32to2u16(lit_expr.u.i);
    sema::bytecode_emit(ctx, LOADI, {dst, operands.high, operands.low});
  }
  else if (lit_expr.kind == Float) {
    uint32_t final_value = std::bit_cast<uint32_t>(lit_expr.u.f);
    auto operands = ubit_u32to2u16(final_value);
    sema::bytecode_emit(ctx, LOADF, {dst, operands.high, operands.low});
  }
  else if (lit_expr.kind == Bool) {
    sema::bytecode_emit(ctx, lit_expr.u.b ? LOADBT : LOADBF, {dst});
  }
  else {
    Value kval = sema::construct_constant(lit_expr);
    operand_t kid = sema::push_constant(ctx, std::move(kval));
    sema::bytecode_emit(ctx, LOADK, {dst, kid});
  }
}

void ExprNodeVisitor::visit(AstNode* node, NodeSymExpr& sym_expr, operand_t dst) {
  if (sema::resolve_lvalue(ctx, node, dst)) {
    // Error: "undeclared-id-use"
    auto message = std::format("Use of undeclared identifier '{}'", sym_expr.symbol);
    sema::error(ctx, node->loc, message);
    sema::flush(ctx);
  }
}

void ExprNodeVisitor::visit(AstNode* node, NodeUnExpr& unary_node, operand_t dst) {
  AstNode* type = sema::resolve_type(ctx, unary_node.expr);
  sema::resolve_rvalue(this, unary_node.expr, dst);

  if (unary_node.op == OP_SUB) {
    if (sema::is_arithmetic(type))
      sema::bytecode_emit(ctx, NEG, {dst});
    else {
      // Error: "ill-negation"
      auto message = std::format("Negating non-negatable type {}", sema::to_string(type));
      sema::error(ctx, node->loc, message);
      sema::flush(ctx);
    }
  }
  else if (unary_node.op == TokenType::OP_LEN) {
    if (type->kind == AstKind::TYPE_Arr) {
      register_t reg = sema::alloc_register(ctx);
      sema::bytecode_emit(ctx, MOV, {reg, dst});
      sema::bytecode_emit(ctx, LENARR, {dst, reg});
      sema::free_register(ctx, reg);
      return;
    }

    // Error: "ill-length-computation"
    auto message = std::format("Taking length of unbounded type {}", sema::to_string(type));
    sema::error(ctx, node->loc, message);
    sema::flush(ctx);
  }
  else if (unary_node.op == TokenType::OP_INC || unary_node.op == TokenType::OP_DEC) {
    Opcode opcode = unary_node.op == TokenType::OP_INC ? INC : DEC;

    if (!sema::is_arithmetic(type)) {
      // Error: "ill-step"
      sema::error(ctx, node->loc, "Stepping non-arithmetic data type");
      sema::flush(ctx);
      return;
    }

    sema::bytecode_emit(ctx, opcode, {dst});
  }
}

void ExprNodeVisitor::visit(AstNode*, NodeGroupExpr& group_node, operand_t dst) {
  sema::resolve_rvalue(this, group_node.expr, dst);
}

void ExprNodeVisitor::visit(AstNode* node, NodeCallExpr& call_node, operand_t dst) {
  AstNode* type = sema::resolve_type(ctx, call_node.callee);
  operand_t reg = sema::alloc_register(ctx);

  if (type->kind == AstKind::TYPE_Fun) {
    size_t expc = type->u.t_fun.paramc;
    if (call_node.argc != expc) {
      // Error: "function-call-argc-mismatch"
      auto message =
        std::format("Function type expects {} arguments, got {}", expc, call_node.argc);
      sema::error(ctx, node->loc, message);
      sema::flush(ctx);
    }
  }
  else {
    // Error: "ill-function-call"
    auto message = std::format("Value of type '{}' is not callable", sema::to_string(type));
    sema::error(ctx, node->loc, message);
    sema::flush(ctx);
  }

  sema::resolve_rvalue(this, call_node.callee, reg);

  ctx.args = sema::alloc_register(ctx);
  for (size_t i = 0; i < call_node.argc; i++) {
    AstNode* arg = call_node.args[i];
    sema::resolve_rvalue(this, arg, ctx.args + i);
  }

  sema::bytecode_emit(ctx, CALL, {reg, ctx.args, dst});
  sema::free_register(ctx, reg);
  sema::free_register(ctx, ctx.args);
}

void ExprNodeVisitor::visit(AstNode* node, NodeIndexExpr& index_node, operand_t dst) {
  AstNode* obj_type = sema::resolve_type(ctx, index_node.obj);
  AstNode* idx_type = sema::resolve_type(ctx, index_node.idx);
  operand_t obj_reg = sema::alloc_register(ctx);

  sema::resolve_rvalue(this, index_node.obj, obj_reg);

  if (obj_type->kind == AstKind::TYPE_Arr) {
    if (idx_type->kind == AstKind::TYPE_Prim) {
      if (idx_type->u.t_prim.type == Int) {
        register_t reg = sema::alloc_register(ctx);
        sema::resolve_rvalue(this, index_node.idx, reg);
        sema::bytecode_emit(ctx, GETARR, {dst, obj_reg, reg});
        sema::free_register(ctx, reg);
        sema::free_register(ctx, obj_reg);
        return;
      }
    }

    // Error: "ill-array-subscript"
    auto message = std::format("Subscripting array with type {}", sema::to_string(idx_type));
    sema::error(ctx, node->loc, message);
    sema::flush(ctx);
  }
  else {
    // Error: "ill-subscript"
    auto message = std::format("lvalue of type {} is not subscriptable", sema::to_string(obj_type));
    sema::error(ctx, node->loc, message);
    sema::flush(ctx);
  }
}

// TODO: Fix bug where constants are folded even when optimization level is O0
void ExprNodeVisitor::visit(AstNode* node, NodeBinExpr& bin_node, operand_t dst) {
  using enum TokenType;

  static const std::unordered_map<TokenType, Opcode> operator_map = {
    {OP_ADD, Opcode::ADD},
    {OP_SUB, Opcode::SUB},
    {OP_MUL, Opcode::MUL},
    {OP_DIV, Opcode::DIV},
    {OP_EXP, Opcode::POW},
    {OP_MOD, Opcode::MOD},
    {OP_EQ, Opcode::EQ},
    {OP_NEQ, Opcode::NEQ},
    {OP_LT, Opcode::LT},
    {OP_GT, Opcode::GT},
    {OP_LEQ, Opcode::LTEQ},
    {OP_GEQ, Opcode::GTEQ},
    {KW_AND, Opcode::AND},
    {KW_OR, Opcode::OR},
  };

  auto op_it = operator_map.find(bin_node.op);

  // Infer types
  AstNode* lhst = sema::resolve_type(ctx, bin_node.lhs);
  AstNode* rhst = sema::resolve_type(ctx, bin_node.rhs);

  if (!sema::is_compatible(lhst, rhst)) {
    // Error: "bin-op-incompatible-types"
    auto message = std::format(
      "Binary operation on incompatible types '{}' (left) and '{}' (right)",
      sema::to_string(lhst),
      sema::to_string(rhst)
    );
    sema::error(ctx, node->loc, message);
    sema::flush(ctx);
    return;
  }

  const Opcode base_opcode = op_it->second;
  const OpCodeId base_opcode_id = static_cast<OpCodeId>(base_opcode);
  OpCodeId opcode_id = base_opcode_id;

  bool is_left_constexpr = sema::is_constexpr(ctx.lctx, bin_node.lhs);
  bool is_right_constexpr = sema::is_constexpr(ctx.lctx, bin_node.rhs);

  // For Bool/relational operations, always handle as non-constant.
  bool is_bool_or_relational = base_opcode == AND || base_opcode == OR || base_opcode == LT
    || base_opcode == GT || base_opcode == LTEQ || base_opcode == GTEQ;

  if (is_left_constexpr && is_right_constexpr && !is_bool_or_relational) {
    // Constant folding is an O1 optimization.
    if (ctx.lctx.optimization_level < 1) {
      goto non_constexpr;
    }

    NodeLitExpr kfolded = sema::fold_constant(ctx, node);
    AstNode knode;
    knode.loc = node->loc;
    knode.kind = AstKind::EXPR_Lit;
    knode.u = {.e_lit = kfolded};

    sema::resolve_rvalue(this, &knode, dst);
  }
  else if (is_right_constexpr && !is_bool_or_relational) {
    if (ctx.lctx.optimization_level < 1) {
      goto non_constexpr;
    }

    NodeLitExpr lit = sema::fold_constant(ctx, bin_node.rhs);

    // Special handling for DIV: check for division by zero.
    if (base_opcode == DIV) {
      if (lit.kind == Int) {
        if VIA_LIKELY (lit.u.i != 0) {
          goto not_div_by_zero;
        }
      }
      else if (lit.kind == Float) {
        if VIA_LIKELY (lit.u.f != 0.0f) {
          goto not_div_by_zero;
        }
      }

      { // Error: "explicit-division-by-zero"
        auto message = "Explicit division by zero";
        sema::error(ctx, bin_node.rhs->loc, message);
        sema::flush(ctx);
        return;
      }
    not_div_by_zero:
    }

    sema::resolve_rvalue(this, bin_node.lhs, dst);

    // Special handling for Bool operations.
    if (base_opcode == AND || base_opcode == OR) {
      bool is_rhs_falsy = false;

      if (lit.kind == Bool)
        is_rhs_falsy = !lit.u.b;
      else if (lit.kind == Nil) // 'nil' is always falsy
        is_rhs_falsy = true;

      if (base_opcode == AND && is_rhs_falsy)
        sema::bytecode_emit(ctx, LOADBF, {dst});
      else if (base_opcode == OR && !is_rhs_falsy)
        sema::bytecode_emit(ctx, LOADBT, {dst});

      return;
    }

    // Handle numeric constant: Int or float.
    if (lit.kind == Int) {
      Opcode opc = static_cast<Opcode>(opcode_id + 1); // OPI for Int
      uint32_t final_value = static_cast<uint32_t>(lit.u.i);
      auto operands = ubit_u32to2u16(final_value);
      sema::bytecode_emit(ctx, opc, {dst, operands.high, operands.low});
    }
    else if (lit.kind == Float) {
      Opcode opc = static_cast<Opcode>(opcode_id + 2); // OPF for float
      uint32_t final_value = std::bit_cast<uint32_t>(lit.u.f);
      auto operands = ubit_u32to2u16(final_value);
      sema::bytecode_emit(ctx, opc, {dst, operands.high, operands.low});
    }
  }
  else {
  non_constexpr:
    // Non-constant expression or Bool/relational operator.
    operand_t reg = sema::alloc_register(ctx);

    // Grouping expressions have the highest precedence
    if (bin_node.rhs->kind == AstKind::EXPR_Group) {
      sema::resolve_rvalue(this, bin_node.rhs, dst);
      sema::resolve_rvalue(this, bin_node.lhs, reg);
    }
    else {
      sema::resolve_rvalue(this, bin_node.lhs, reg);
      sema::resolve_rvalue(this, bin_node.rhs, dst);
    }

    if (is_bool_or_relational) {
      operand_t right = sema::alloc_register(ctx);
      sema::bytecode_emit(ctx, MOV, {right, dst});
      sema::bytecode_emit(ctx, base_opcode, {dst, reg, right});
      sema::free_register(ctx, reg);
    }
    else {
      sema::bytecode_emit(ctx, base_opcode, {dst, reg});
      sema::free_register(ctx, reg);
    }
  }
}

void ExprNodeVisitor::visit(AstNode* node, NodeCastExpr& type_cast, operand_t dst) {
  AstNode* tleft = sema::resolve_type(ctx, type_cast.expr);

  if (!sema::is_castable(tleft, type_cast.ty)) {
    // Error: "ill-explicit-cast"
    auto message = std::format(
      "Expression of type {} is not castable into type {}",
      sema::to_string(tleft),
      sema::to_string(type_cast.ty)
    );
    sema::error(ctx, node->loc, message);
    sema::flush(ctx);
  }

  operand_t temp = sema::alloc_register(ctx);
  sema::resolve_rvalue(this, type_cast.expr, temp);

  if (type_cast.ty->kind == AstKind::TYPE_Prim) {
    NodePrimType* prim = &type_cast.ty->u.t_prim;
    if (prim->type == Int)
      sema::bytecode_emit(ctx, ICAST, {dst, temp});
    else if (prim->type == Float)
      sema::bytecode_emit(ctx, FCAST, {dst, temp});
    else if (prim->type == String)
      sema::bytecode_emit(ctx, STRCAST, {dst, temp});
    else if (prim->type == Bool)
      sema::bytecode_emit(ctx, BCAST, {dst, temp});
  }

  sema::free_register(ctx, temp);
}

void ExprNodeVisitor::visit(AstNode*, NodeStepExpr& step_expr, operand_t dst) {
  auto opcode = step_expr.op == OP_INC ? INC : DEC;
  operand_t temp = sema::alloc_register(ctx);
  sema::resolve_lvalue(ctx, step_expr.expr, dst);
  sema::bytecode_emit(ctx, MOV, {temp, dst});
  sema::bytecode_emit(ctx, opcode, {temp});
  sema::bind_lvalue(ctx, step_expr.expr, temp);
  sema::free_register(ctx, temp);
}

void ExprNodeVisitor::visit(AstNode* node, NodeArrExpr& array_expr, operand_t dst) {
  if (array_expr.valc == 0) {
    sema::bytecode_emit(ctx, LOADARR, {dst});
    return;
  }

  if (sema::is_constexpr(ctx.lctx, node)) {
    struct Array* arr = new struct Array();
    struct Value val = Value(arr);

    for (size_t i = 0; i < array_expr.valc; i++) {
      AstNode* kexpr = array_expr.vals[i];
      NodeLitExpr literal = sema::fold_constant(ctx, kexpr);
      Value kval = sema::construct_constant(literal);
      impl::__array_set(arr, i, std::move(kval));
    }

    operand_t kid = sema::push_constant(ctx, std::move(val));
    sema::bytecode_emit(ctx, LOADK, {dst, kid});
  }
  else {
    sema::error(ctx, "TODO: IMPLEMENT NON-CONSTANT ARRAY CONSTRUCTION");
    sema::flush(ctx);
  }
}

void ExprNodeVisitor::visit(AstNode* node, NodeIntrExpr& intrinsic_expr, operand_t dst) {
  bool is_print = !std::strcmp(intrinsic_expr.id, "print");
  bool is_error = !std::strcmp(intrinsic_expr.id, "error");
  if (is_print || is_error) {
    if (intrinsic_expr.exprc == 0) {
      auto message = std::format("Intrinsic '{}' expects 1 argument(s), got 0", intrinsic_expr.id);
      sema::error(ctx, node->loc, message);
      sema::flush(ctx);
      return;
    }

    NodeLitExpr lit;
    lit.kind = String;
    lit.u.s = (char*)(is_print ? "__print" : "__error");

    Value kval = sema::construct_constant(lit);
    operand_t kid = sema::push_constant(ctx, std::move(kval));
    operand_t fn_reg = sema::alloc_register(ctx);
    operand_t arg_reg = sema::alloc_register(ctx);

    sema::bytecode_emit(ctx, LOADK, {fn_reg, kid});
    sema::bytecode_emit(ctx, GETGLOBAL, {fn_reg, fn_reg});
    sema::resolve_rvalue(this, intrinsic_expr.exprs[0], arg_reg);
    sema::bytecode_emit(ctx, CALL, {fn_reg, arg_reg, fn_reg});
    sema::free_register(ctx, fn_reg);
    sema::free_register(ctx, arg_reg);
    return;
  }

  if (!std::strcmp(intrinsic_expr.id, "nameof")) {
    if (intrinsic_expr.exprc == 0) {
      sema::error(ctx, node->loc, "Expected 1 argument for intrinsic 'nameof'");
      sema::flush(ctx);
      return;
    }

    AstNode* target = intrinsic_expr.exprs[0];
    if (target->kind == AstKind::EXPR_Sym) {
      NodeSymExpr sym = target->u.e_sym;
      NodeLitExpr lit;
      lit.kind = String;
      lit.u.s = sym.symbol;

      Value kval = sema::construct_constant(lit);
      operand_t kid = sema::push_constant(ctx, std::move(kval));

      auto comment = std::format("nameof({})", sym.symbol);
      sema::bytecode_emit(ctx, LOADK, {dst, kid}, comment);
    }
    else {
      sema::error(ctx, target->loc, "Expected lvalue expression for 'nameof'");
      sema::flush(ctx);
    }
  }
  else if (!std::strcmp(intrinsic_expr.id, "type")) {
    if (intrinsic_expr.exprc == 0) {
      sema::error(ctx, node->loc, "Expected 1 argument for intrinsic 'type'");
      sema::flush(ctx);
      return;
    }

    AstNode* expr = intrinsic_expr.exprs[0];
    AstNode* tinf = sema::resolve_type(ctx, expr);
    NodeLitExpr lit;

    if (tinf->kind == AstKind::TYPE_Prim) {
      std::string type_name = std::string(magic_enum::enum_name(tinf->u.t_prim.type));
      std::transform(type_name.begin(), type_name.end(), type_name.begin(), [](unsigned char chr) {
        return std::tolower(chr);
      });

      lit.kind = String;
      lit.u.s = sema::alloc_string(ctx.lctx.stralloc, type_name);
    }
    else if (tinf->kind == AstKind::TYPE_Fun) {
      lit.kind = String;
      lit.u.s = "function";
    }
    else {
      sema::error(ctx, "TODO: Implement rest of intrinsic: type()");
      sema::flush(ctx);
    }

    Value kval = sema::construct_constant(lit);
    operand_t kid = sema::push_constant(ctx, std::move(kval));

    sema::bytecode_emit(ctx, LOADK, {dst, kid}, std::format("typeof({})", sema::to_string(expr)));
  }
  else if (!std::strcmp(intrinsic_expr.id, "deep_eq")) {
    if (intrinsic_expr.exprc < 2) {
      sema::error(ctx, node->loc, "Expected 2 arguments for intrinsic 'deep_eq'");
      sema::flush(ctx);
      return;
    }

    AstNode *left = intrinsic_expr.exprs[0], *right = intrinsic_expr.exprs[1];
    register_t lreg = sema::alloc_register(ctx), rreg = sema::alloc_register(ctx);

    left->accept(*this, lreg);
    right->accept(*this, rreg);

    sema::bytecode_emit(ctx, DEQ, {dst, lreg, rreg}, "deep_eq(...)");
    sema::free_register(ctx, lreg);
    sema::free_register(ctx, rreg);
  }
  else if (!std::strcmp(intrinsic_expr.id, "try")) {
    if (intrinsic_expr.exprc < 1) {
      sema::error(ctx, node->loc, "Expected 1 argument for intrinsic 'try'");
      sema::flush(ctx);
      return;
    }

    AstNode* expr = intrinsic_expr.exprs[0];
    if (expr->kind == AstKind::EXPR_Call) {
      call_expr->accept(*this, dst);

      // Modify CALL instruction to PCALL
      Instruction& call_insn = ctx.lctx.bytecode.back();
      call_insn.op = PCALL;
    }
    else {
      sema::error(ctx, expr->loc, "Intrinsic 'try' expects function call");
      sema::flush(ctx);
    }
  }
}

} // namespace via
