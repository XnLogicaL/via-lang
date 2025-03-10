// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "bitutils.h"
#include "strutils.h"
#include "visitor.h"
#include "stack.h"

VIA_NAMESPACE_BEGIN

using enum OpCode;
using enum OutputSeverity;

void StmtVisitor::visit(DeclarationNode& declaration_node) {
    bool is_global = declaration_node.is_global;
    bool is_const  = declaration_node.modifiers.is_const;

    ExprNode&   val    = *std::move(declaration_node.value_expression);
    Token       ident  = declaration_node.identifier;
    std::string symbol = ident.lexeme;

    if (is_global) {
        std::string           comment             = symbol;
        std::optional<Global> previously_declared = program.globals->get_global(symbol);

        if (previously_declared.has_value()) {
            std::string previously_declared_error =
                std::format("Attempt to re-declare global '{}'", symbol);

            visitor_failed = true;
            emitter.out(ident.position, previously_declared_error, Error);
            emitter.out(previously_declared->token.position, "Previously declared here", Info);
        }
        else {
            Operand value_reg   = allocator.allocate_register();
            Operand symbol_hash = hash_string_custom(symbol.c_str());

            declaration_node.value_expression->accept(expression_visitor, value_reg);

            Global global{.token = ident, .symbol = symbol};
            program.globals->declare_global(global);
            program.bytecode->emit(SETGLOBAL, {value_reg, symbol_hash}, comment);

            allocator.free_register(value_reg);
        }
    }
    else {
        std::string comment = std::format("{}", symbol);

        if (is_constant_expression(val)) {
            LiteralNode& literal = dynamic_cast<LiteralNode&>(val);

            if (std::get_if<std::monostate>(&literal.value)) {
                program.bytecode->emit(PUSHNIL, {}, comment);
                program.test_stack->push({
                    .symbol         = symbol,
                    .is_const       = is_const,
                    .is_constexpr   = true,
                    .primitive_type = ValueType::nil,
                });
            }
            else if (int* int_value = std::get_if<int>(&literal.value)) {
                U32  final_value = *int_value;
                auto operands    = reinterpret_u32_as_2u16(final_value);

                program.bytecode->emit(PUSHINT, {operands.l, operands.r}, comment);
                program.test_stack->push({
                    .symbol         = symbol,
                    .is_const       = is_const,
                    .is_constexpr   = true,
                    .primitive_type = ValueType::integer,
                });
            }
            else if (float* float_value = std::get_if<float>(&literal.value)) {
                U32  final_value = std::bit_cast<U32>(*float_value);
                auto operands    = reinterpret_u32_as_2u16(final_value);

                program.bytecode->emit(PUSHFLOAT, {operands.l, operands.r}, comment);
                program.test_stack->push({
                    .symbol         = symbol,
                    .is_const       = is_const,
                    .is_constexpr   = true,
                    .primitive_type = ValueType::floating_point,
                });
            }
            else if (bool* bool_value = std::get_if<bool>(&literal.value)) {
                program.bytecode->emit(*bool_value ? PUSHTRUE : PUSHFALSE, {}, comment);
                program.test_stack->push({
                    .symbol         = symbol,
                    .is_const       = is_const,
                    .is_constexpr   = true,
                    .primitive_type = ValueType::boolean,
                });
            }
            else {
                const TValue& constant = construct_constant(dynamic_cast<LiteralNode&>(val));
                const Operand const_id = program.constants->push_constant(constant);

                program.bytecode->emit(PUSHK, {const_id}, comment);
                program.test_stack->push({
                    .symbol         = symbol,
                    .is_const       = is_const,
                    .is_constexpr   = true,
                    .primitive_type = constant.type,
                });
            }
        }
        else {
            Operand dst = allocator.allocate_register();

            declaration_node.value_expression->accept(expression_visitor, dst);

            program.bytecode->emit(PUSH, {dst}, comment);
            program.test_stack->push({
                .symbol         = symbol,
                .is_const       = is_const,
                .is_constexpr   = false,
                .primitive_type = ValueType::nil,
            });

            allocator.free_register(dst);
        }
    }
}

void StmtVisitor::visit(ScopeNode& scope_node) {
    Operand stack_pointer = program.test_stack->sp;

    for (const pStmtNode& pstmt : scope_node.statements) {
        pstmt->accept(*this);
    }

    Operand stack_allocations = program.test_stack->sp - stack_pointer;
    for (; stack_allocations > 0; stack_allocations--) {
        program.bytecode->emit(DROP);
    }
}

void StmtVisitor::visit(FunctionNode& function_node) {
    Operand function_reg = allocator.allocate_register();

    // Push function to the function stack
    program.test_stack->function_stack.push(FunctionNode::StackNode(
        function_node.is_global,
        function_node.modifiers,
        function_node.identifier,
        function_node.parameters
    ));

    program.bytecode->emit(LOADFUNCTION, {function_reg}, function_node.identifier.lexeme);

    ScopeNode& scope = dynamic_cast<ScopeNode&>(*function_node.body);
    for (const pStmtNode& pstmt : scope.statements) {
        const StmtNode& stmt = *pstmt;

        const DeclarationNode* declaration_node = dynamic_cast<const DeclarationNode*>(&stmt);
        const FunctionNode*    function_node    = dynamic_cast<const FunctionNode*>(&stmt);

        if (declaration_node || function_node) {
            bool is_global =
                declaration_node ? declaration_node->is_global : function_node->is_global;
            SIZE identifier_position = declaration_node ? declaration_node->identifier.position
                                                        : function_node->identifier.position;

            if (is_global) {
                visitor_failed = true;
                emitter.out(identifier_position, "Function scopes cannot declare globals", Error);
                emitter.out_flat(
                    "Function scopes containing global declarations may cause previously declared "
                    "globals to be re-declared, therefore are not allowed.",
                    Info
                );
                break;
            }
        }

        pstmt->accept(*this);
    }

    Token       symbol_token = function_node.identifier;
    std::string symbol       = symbol_token.lexeme;
    U32         symbol_hash  = hash_string_custom(symbol.c_str());

    if (function_node.is_global) {
        if (program.globals->was_declared(symbol)) {
            visitor_failed = true;
            emitter.out(
                symbol_token.position,
                std::format("Attempt to re-declare global '{}'", symbol),
                Error
            );
            return;
        }

        auto operands = reinterpret_u32_as_2u16(symbol_hash);
        program.bytecode->emit(SETGLOBAL, {function_reg, operands.l, operands.r});
    }
    else {
        program.bytecode->emit(PUSH, {function_reg});
    }

    allocator.free_register(function_reg);
    program.test_stack->function_stack.pop();
    program.test_stack->push({
        .is_const       = function_node.modifiers.is_const,
        .is_constexpr   = false,
        .symbol         = symbol,
        .primitive_type = ValueType::function,
    });
}

void StmtVisitor::visit(AssignNode& assign_node) {
    Token symbol_token = assign_node.identifier;

    std::string            symbol = symbol_token.lexeme;
    std::optional<Operand> stk_id = program.test_stack->find_symbol({.symbol = symbol});

    if (stk_id.has_value()) {
        const auto& test_stack_member = program.test_stack->at(stk_id.value());
        if (test_stack_member.has_value() && test_stack_member->is_const) {
            visitor_failed = true;
            emitter.out(
                symbol_token.position,
                std::format("Attempt to modify constant variable '{}'", symbol),
                Error
            );

            return;
        }

        Operand value_reg = allocator.allocate_register();
        assign_node.value->accept(expression_visitor, value_reg);
        program.bytecode->emit(SETSTACK, {value_reg, stk_id.value()});
    }
    else {
        visitor_failed = true;
        emitter.out(symbol_token.position, "Attempt to assign to undeclared symbol", Error);
    }
}

void StmtVisitor::visit(IfNode&) {}
void StmtVisitor::visit(WhileNode&) {}

void StmtVisitor::visit(ExprStmtNode& expr_stmt) {
    Operand trash_register = allocator.allocate_register();
    expr_stmt.expression->accept(expression_visitor, trash_register);
    allocator.free_register(trash_register);
}

VIA_NAMESPACE_END
