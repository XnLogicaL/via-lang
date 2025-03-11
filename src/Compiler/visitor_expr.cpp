// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "api.h"
#include "bitutils.h"
#include "stack.h"
#include "types.h"
#include "visitor.h"

// ==========================================================================================
// visitor_expr.cpp
//
// This file is a part of the first compiler stage (0), and is used to compile expressions.
// It defines the `ExpressionVisitor::visit` function overloads.
//
// All visitor functions have common parameters: (NodeType& node, Operand dst).
// - NodeType& node: AST node object.
// - Operand dst: The destination register in which the expression lives until externally free'd.
//  - Note: This also implies that `dst` is not owned.
//
// Visitor functions compile each type of expression node by first converting it into
// corresponding opcode(s), and then determining the operands via the built-in node parameters.
//
// - LiteralNode compilation:
//  This node only emits `LOAD` opcodes, and is considered a constant expression.
//  It first checks for primitive data types within the node, and emits corresponding bytecode based
//  on that. If it finds complex data types like strings, tables, etc., it loads them into the
//  constant table and emits a `LOADK` instruction with the corresponding constant id.
//
// - SymbolNode compilation:
//  This node represents a "symbol" that is either a local, global, argument or upvalue.
//  It first checks the stack for the symbol, if found, emits a `GETSTACK` instruction with the
//  stack id of the symbol. After that, it checks for upvalues, if found emits `GETUPVALUE`. Next,
//  it checks for arguments by traversing the parameters of the top function in
//  `program::test_stack::function_stack` and looking for the symbol and if found, emits
//  `GETARGUMENT`. Finally, looks for the variable in the global scope by querying
//  `program::globals` and if found emits GETGLOBAL. If all of these queries fail, throws an
//  "Use of undeclared variable" compilation error.
//
// ==================================================================================================
VIA_NAMESPACE_BEGIN

using enum OpCode;
using enum OutputSeverity;

void ExprVisitor::visit(LiteralNode& literal_node, Operand dst) {
    using enum ValueType;

    if (int* integer_value = std::get_if<int>(&literal_node.value)) {
        u32  final_value = *integer_value;
        auto operands    = reinterpret_u32_as_2u16(final_value);

        program.bytecode->emit(LOADINT, {dst, operands.l, operands.r});
    }
    else if (float* float_value = std::get_if<float>(&literal_node.value)) {
        u32  final_value = std::bit_cast<u32>(*float_value);
        auto operands    = reinterpret_u32_as_2u16(final_value);

        program.bytecode->emit(LOADFLOAT, {dst, operands.l, operands.r});
    }
    else if (bool* bool_value = std::get_if<bool>(&literal_node.value)) {
        program.bytecode->emit(*bool_value ? LOADTRUE : LOADFALSE, {dst});
    }
    else {
        const TValue& constant    = construct_constant(literal_node);
        const Operand constant_id = program.constants->push_constant(constant);

        program.bytecode->emit(LOADK, {dst, constant_id});
    }
}

void ExprVisitor::visit(SymbolNode& variable_node, Operand dst) {
    Token var_id = variable_node.identifier;

    std::string            symbol = var_id.lexeme;
    std::optional<Operand> stk_id = program.test_stack->find_symbol(var_id.lexeme);

    if (stk_id.has_value()) {
        auto& current_closure = program.test_stack->function_stack.top();
        if (current_closure.upvalues < stk_id.value()) {
            program.bytecode->emit(GETSTACK, {dst, stk_id.value()}, symbol);
        }
        else {
            program.bytecode->emit(GETUPVALUE, {dst, stk_id.value()}, symbol);
        }
        return;
    }
    else if (program.globals->was_declared(symbol)) {
        u32  symbol_hash = hash_string_custom(symbol.c_str());
        auto operands    = reinterpret_u32_as_2u16(symbol_hash);

        program.bytecode->emit(GETGLOBAL, {dst, operands.l, operands.r}, symbol);
        return;
    }
    else if (!program.test_stack->function_stack.empty()) {
        u16   index = 0;
        auto& top   = program.test_stack->function_stack.top();

        for (const auto& parameter : top.parameters) {
            if (parameter.identifier.lexeme == symbol) {
                program.bytecode->emit(GETARGUMENT, {dst, index});
                return;
            }

            ++index;
        }
    }

    visitor_failed = true;
    emitter.out(var_id, std::format("Use of undeclared variable '{}'", var_id.lexeme), Error);
}

void ExprVisitor::visit(UnaryNode& unary_node, Operand dst) {
    unary_node.accept(*this, dst);
    program.bytecode->emit(NEG, {dst});
}

void ExprVisitor::visit(GroupNode& group_node, Operand dst) {
    group_node.accept(*this, dst);
}

void ExprVisitor::visit(CallNode& call_node, Operand dst) {
    Operand argc       = call_node.arguments.size();
    Operand callee_reg = allocator.allocate_register();

    call_node.callee->accept(*this, callee_reg);

    for (const pExprNode& argument : call_node.arguments) {
        Operand argument_reg = allocator.allocate_register();

        argument->accept(*this, argument_reg);
        program.bytecode->emit(PUSH, {argument_reg});
        allocator.free_register(argument_reg);
    }

    program.bytecode->emit(CALL, {callee_reg, argc});
    program.bytecode->emit(POP, {dst});
    allocator.free_register(callee_reg);
}

void ExprVisitor::visit(IndexNode& index_node, Operand dst) {
    Operand obj_reg = allocator.allocate_register();
    index_node.object->accept(*this, obj_reg);

    Operand index_reg = allocator.allocate_register();
    index_node.index->accept(*this, index_reg);

    program.bytecode->emit(GET, {dst, obj_reg, index_reg});
    allocator.free_register(obj_reg);
    allocator.free_register(index_reg);
}

void ExprVisitor::visit(BinaryNode& binary_node, Operand dst) {
    using enum TokenType;
    using OpCodeId = std::underlying_type_t<OpCode>;

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
        {TokenType::KW_OR, OpCode::OR},
    };

    pExprNode& p_lhs = binary_node.lhs_expression;
    pExprNode& p_rhs = binary_node.rhs_expression;

    ExprNode& lhs = *p_lhs;
    ExprNode& rhs = *p_rhs;

    auto it = operator_map.find(binary_node.op.type);
    if (it == operator_map.end()) {
        visitor_failed = true;
        emitter.out(
            binary_node.op,
            std::format("Unknown binary operator '{}'", binary_node.op.lexeme),
            Error
        );
        return;
    }

    const OpCode   base_opcode    = it->second;
    const OpCodeId base_opcode_id = static_cast<OpCodeId>(base_opcode);
    OpCodeId       opcode_id      = base_opcode_id;

    if (is_constant_expression(rhs)) {
        LiteralNode& literal = dynamic_cast<LiteralNode&>(rhs);

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
            visitor_failed = true;
            emitter.out(literal.value_token, "Explicit division by zero", Error);
            return;

        good_division:
        }

        lhs.accept(*this, dst);

        if (base_opcode == AND || base_opcode == OR) {
            bool is_rhs_falsy = ({
                bool* rhs_bool = std::get_if<bool>(&literal.value);
                auto* rhs_nil  = std::get_if<std::monostate>(&literal.value);
                rhs_bool != nullptr ? !(*rhs_bool) : rhs_nil != nullptr;
            });

            if (base_opcode == AND && is_rhs_falsy) {
                program.bytecode->emit(LOADFALSE, {dst});
            }

            if (base_opcode == OR && !is_rhs_falsy) {
                program.bytecode->emit(LOADTRUE, {dst});
            }

            return;
        }

        if (int* int_value = std::get_if<int>(&literal.value)) {
            OpCode opcode      = static_cast<OpCode>(opcode_id + 2); // OPINT
            u32    final_value = *int_value;
            auto   operands    = reinterpret_u32_as_2u16(final_value);

            program.bytecode->emit(opcode, {dst, operands.l, operands.r});
        }
        else if (float* float_value = std::get_if<float>(&literal.value)) {
            OpCode opcode      = static_cast<OpCode>(opcode_id + 3); // OPFLOAT
            u32    final_value = std::bit_cast<u32>(*float_value);
            auto   operands    = reinterpret_u32_as_2u16(final_value);

            program.bytecode->emit(opcode, {dst, operands.l, operands.r});
        }
        else {
            OpCode  opcode          = static_cast<OpCode>(opcode_id + 1); // OPK
            TValue  right_const_val = construct_constant(dynamic_cast<LiteralNode&>(rhs));
            Operand right_const_id  = program.constants->push_constant(right_const_val);

            program.bytecode->emit(opcode, {dst, right_const_id});
        }
    }
    else {
        Operand reg = allocator.allocate_register();

        if (rhs.precedence() > lhs.precedence()) {
            rhs.accept(*this, dst);
            lhs.accept(*this, reg);
        }
        else {
            lhs.accept(*this, dst);
            rhs.accept(*this, reg);
        }

        program.bytecode->emit(base_opcode, {dst, reg});
        allocator.free_register(reg);
    }
}

VIA_NAMESPACE_END
