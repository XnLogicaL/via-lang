// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "api.h"
#include "stack.h"
#include "visitor.h"

namespace via {

using enum OpCode;
using enum OutputSeverity;

void ExprVisitor::visit(LiteralNode &literal_node, VIA_OPERAND dst)
{
    using enum ValueType;

    TValue      constant     = construct_constant(literal_node);
    VIA_OPERAND constant_id  = PUSH_K(constant);
    std::string value_string = ""; //to_cxx_string(nullptr, constant);
    program.bytecode->emit(LOADK, {dst, constant_id}, value_string);
}

void ExprVisitor::visit(SymbolNode &variable_node, VIA_OPERAND dst)
{
    Token var_id = variable_node.identifier;

    std::string                symbol = var_id.lexeme;
    std::optional<VIA_OPERAND> stk_id = program.test_stack->find_symbol({
        var_id.lexeme,
    });

    if (stk_id.has_value()) {
        program.bytecode->emit(
            GETSTACK,
            {
                dst,
                stk_id.value(),
            },
            std::format("local {}", symbol)
        );

        return;
    }
    else if (program.globals->was_declared(symbol)) {
        U32 symbol_hash = hash_string(symbol.c_str());
        program.bytecode->emit(
            GETGLOBAL,
            {
                dst,
                static_cast<VIA_OPERAND>(symbol_hash & 0xFFFF),
                static_cast<VIA_OPERAND>(symbol_hash >> 16),
            },
            std::format("global {}", symbol)
        );

        return;
    }
    else if (!program.test_stack->function_stack.empty()) {
        U16         index = 0;
        const auto &top   = program.test_stack->function_stack.top();
        for (const auto &parameter : top.parameters) {
            if (parameter.identifier.lexeme == symbol) {
                program.bytecode->emit(GETARGUMENT, {dst, index});
                return;
            }

            ++index;
        }
    }

    visitor_failed = true;
    emitter.out(
        var_id.position, std::format("Use of undeclared variable '{}'", var_id.lexeme), Error
    );
}

void ExprVisitor::visit(UnaryNode &unary_node, VIA_OPERAND dst)
{
    unary_node.accept(*this, dst);
    program.bytecode->emit(NEG, {dst});
}

void ExprVisitor::visit(GroupNode &group_node, VIA_OPERAND dst)
{
    group_node.accept(*this, dst);
}

void ExprVisitor::visit(CallNode &call_node, VIA_OPERAND dst)
{
    VIA_OPERAND argc       = call_node.arguments.size();
    VIA_OPERAND callee_reg = allocator.allocate_register();
    call_node.callee->accept(*this, callee_reg);

    for (const pExprNode &argument : call_node.arguments) {
        VIA_OPERAND argument_reg = allocator.allocate_register();
        argument->accept(*this, argument_reg);

        program.bytecode->emit(PUSH, {argument_reg});
        allocator.free_register(argument_reg);
    }

    program.bytecode->emit(CALL, {callee_reg, argc});
    program.bytecode->emit(POP, {dst});
    allocator.free_register(callee_reg);
}

void ExprVisitor::visit(IndexNode &index_node, VIA_OPERAND dst)
{
    VIA_OPERAND obj_reg = allocator.allocate_register();
    index_node.object->accept(*this, obj_reg);

    VIA_OPERAND index_reg = allocator.allocate_register();
    index_node.index->accept(*this, index_reg);

    program.bytecode->emit(GET, {dst, obj_reg, index_reg});
    allocator.free_register(obj_reg);
    allocator.free_register(index_reg);
}

void ExprVisitor::visit(BinaryNode &binary_node, VIA_OPERAND dst)
{
    using enum TokenType;

    static const std::unordered_map<TokenType, OpCode> operator_map = {
        {TokenType::OP_ADD, OpCode::ADD},
        {TokenType::OP_SUB, OpCode::SUB},
        {TokenType::OP_MUL, OpCode::MUL},
        {TokenType::OP_DIV, OpCode::DIV},
        {TokenType::OP_EXP, OpCode::POW},
        {TokenType::OP_MOD, OpCode::MOD},
        {TokenType::OP_EQ, OpCode::EQUAL},
        {TokenType::OP_NEQ, OpCode::NOTEQUAL},
        {TokenType::OP_LT, OpCode::LESS},
        {TokenType::OP_GT, OpCode::GREATER},
        {TokenType::OP_LEQ, OpCode::LESSOREQUAL},
        {TokenType::OP_GEQ, OpCode::GREATEROREQUAL},
        {TokenType::KW_AND, OpCode::AND},
        {TokenType::KW_OR, OpCode::OR}
    };

    pExprNode &p_lhs = binary_node.lhs_expression;
    pExprNode &p_rhs = binary_node.rhs_expression;

    ExprNode &lhs = *p_lhs;
    ExprNode &rhs = *p_rhs;

    auto it = operator_map.find(binary_node.op.type);
    if (it == operator_map.end()) {
        visitor_failed = true;
        emitter.out(
            binary_node.op.position,
            std::format("Unknown binary operator '{}'", binary_node.op.lexeme),
            Error
        );

        return;
    }

    U8 opcode_id = static_cast<U8>(it->second);

    if IS_INHERITOR (rhs, LiteralNode) {
        OpCode opcode = static_cast<OpCode>(opcode_id + 1); // OPK

        lhs.accept(*this, dst);

        TValue      right_const_val = construct_constant(dynamic_cast<LiteralNode &>(rhs));
        VIA_OPERAND right_const_id  = PUSH_K(right_const_val);

        program.bytecode->emit(opcode, {dst, right_const_id});
    }
    else {
        OpCode      opcode = static_cast<OpCode>(opcode_id);
        VIA_OPERAND reg    = allocator.allocate_register();

        if (rhs.precedence() > lhs.precedence()) {
            rhs.accept(*this, dst);
            lhs.accept(*this, reg);
        }
        else {
            lhs.accept(*this, dst);
            rhs.accept(*this, reg);
        }

        program.bytecode->emit(opcode, {dst, reg});
        allocator.free_register(reg);
    }
}

} // namespace via
