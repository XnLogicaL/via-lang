// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "api.h"
#include "stack.h"
#include "visitor.h"

namespace via {

using enum OpCode;
using enum OutputSeverity;

void ExprVisitor::visit(LiteralNode &literal_node, U32 dst)
{
    using enum ValueType;

    TValue constant = construct_constant(literal_node);
    U32 constant_id = PUSH_K(constant);
    std::string value_string = to_cxx_string(nullptr, constant);
    program->bytecode->emit(LOADK, {dst, constant_id}, value_string);
}

void ExprVisitor::visit(VariableNode &variable_node, U32 dst)
{
    Token var_id = variable_node.identifier;

    std::string comment = std::format("local {}", var_id.lexeme);
    std::optional<U32> stk_id = program->test_stack->find_symbol({var_id.lexeme});

    if (stk_id.has_value()) {
        program->bytecode->emit(GETSTACK, {dst, stk_id.value()}, comment);
    }
    else {
        program->bytecode->emit(LOADNIL, {dst}, comment);
        visitor_failed = true;
        emitter.out(
            var_id.position, std::format("Use of undeclared variable '{}'", var_id.lexeme), Error
        );
    }
}

void ExprVisitor::visit(UnaryNode &unary_node, U32 dst)
{
    unary_node.accept(*this, dst);
    program->bytecode->emit(NEG, {dst});
}

void ExprVisitor::visit(GroupNode &group_node, U32 dst)
{
    group_node.accept(*this, dst);
}

void ExprVisitor::visit(CallNode &call_node, U32 dst)
{
    U32 argc = call_node.arguments.size();
    U32 callee_reg = allocator.allocate_register();
    call_node.callee->accept(*this, callee_reg);

    for (const pExprNode &argument : call_node.arguments) {
        U32 argument_reg = allocator.allocate_register();
        argument->accept(*this, argument_reg);

        program->bytecode->emit(PUSH, {argument_reg});
        allocator.free_register(argument_reg);
    }

    program->bytecode->emit(CALL, {callee_reg, argc});
    program->bytecode->emit(POP, {dst});
    allocator.free_register(callee_reg);
}

void ExprVisitor::visit(IndexNode &index_node, U32 dst)
{
    U32 obj_reg = allocator.allocate_register();
    index_node.object->accept(*this, obj_reg);

    U32 index_reg = allocator.allocate_register();
    index_node.index->accept(*this, index_reg);

    program->bytecode->emit(GET, {dst, obj_reg, index_reg});
    allocator.free_register(obj_reg);
    allocator.free_register(index_reg);
}

void ExprVisitor::visit(BinaryNode &binary_node, U32 dst)
{
    using enum TokenType;

    static const constexpr U8 k_operator_offset = static_cast<U8>(OP_ADD) - static_cast<U8>(ADD);

    std::unique_ptr<ExprNode> &p_lhs = binary_node.lhs_expression;
    std::unique_ptr<ExprNode> &p_rhs = binary_node.rhs_expression;

    ExprNode &lhs = *p_lhs;
    ExprNode &rhs = *p_rhs;

    U32 reg = allocator.allocate_register();
    U8 opcode_id = static_cast<U8>(binary_node.op.type) + k_operator_offset;

    if IS_INHERITOR (rhs, LiteralNode) {
        OpCode opcode = static_cast<OpCode>(opcode_id + 1); // OPK

        lhs.accept(*this, reg);

        TValue right_const_val = construct_constant(dynamic_cast<LiteralNode &>(rhs));
        U32 right_const_id = PUSH_K(right_const_val);

        program->bytecode->emit(opcode, {reg, right_const_id});
        allocator.free_register(reg);
    }
    else {
        OpCode opcode = static_cast<OpCode>(opcode_id);

        if (rhs.precedence() > lhs.precedence()) {
            rhs.accept(*this, reg);
            lhs.accept(*this, dst);
        }
        else {
            lhs.accept(*this, dst);
            rhs.accept(*this, reg);
        }

        program->bytecode->emit(opcode, {dst, reg});
        allocator.free_register(reg);
    }
}

} // namespace via
