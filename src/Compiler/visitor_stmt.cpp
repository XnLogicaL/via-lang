// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "visitor.h"
#include "stack.h"

#define VISITOR_INDEX program->bytecode->get().size()
#define VISITOR_DEF_LABEL(lbl) VIA_OPERAND lbl = VISITOR_INDEX;
#define VISITOR_GET_LABEL(lbl) static_cast<I32>(VISITOR_INDEX - lbl)

namespace via {

using enum OpCode;
using enum OutputSeverity;

void StmtVisitor::visit(DeclarationNode &declaration_node)
{
    bool is_global = declaration_node.is_global;
    bool is_const = declaration_node.modifiers.is_const;

    ExprNode &val = *std::move(declaration_node.value_expression);
    Token ident = declaration_node.identifier;
    std::string symbol = ident.lexeme;

    if (is_global) {
        std::string comment = std::format("global {}", symbol);
        std::optional<Global> previously_declared = program->globals->get_global(symbol);

        if (previously_declared.has_value()) {
            std::string previously_declared_error =
                std::format("Attempt to re-declare global '{}'", symbol);

            visitor_failed = true;
            emitter.out(ident.position, previously_declared_error, Error);
            emitter.out(previously_declared->token.position, "Previously declared here", Info);
        }
        else {
            VIA_OPERAND value_reg = allocator.allocate_register();
            VIA_OPERAND symbol_hash = hash_string(symbol.c_str());

            declaration_node.value_expression->accept(expression_visitor, value_reg);

            Global global{.token = ident, .symbol = symbol};
            program->globals->declare_global(global);
            program->bytecode->emit(SETGLOBAL, {value_reg, symbol_hash}, comment);

            allocator.free_register(value_reg);
        }
    }
    else {
        std::string comment = std::format("local {}", symbol);

        if IS_CONSTEXPR (val) {
            const TValue &constant = construct_constant(dynamic_cast<LiteralNode &>(val));
            const VIA_OPERAND const_id = PUSH_K(constant);

            program->bytecode->emit(PUSHK, {const_id}, comment);
            program->test_stack->push({
                .symbol = symbol,
                .is_const = is_const,
                .is_constexpr = true,
                .primitive_type = constant.type,
            });
        }
        else {
            VIA_OPERAND dst = allocator.allocate_register();

            declaration_node.value_expression->accept(expression_visitor, dst);

            program->bytecode->emit(PUSH, {dst}, comment);
            program->test_stack->push({
                .symbol = symbol,
                .is_const = is_const,
                .is_constexpr = false,
                .primitive_type = ValueType::nil,
            });

            allocator.free_register(dst);
        }
    }
}

void StmtVisitor::visit(ScopeNode &scope_node)
{
    VIA_OPERAND stack_pointer = program->test_stack->sp;

    for (const pStmtNode &pstmt : scope_node.statements) {
        pstmt->accept(*this);
    }

    VIA_OPERAND stack_allocations = program->test_stack->sp - stack_pointer;
    for (; stack_allocations > 0; stack_allocations--) {
        program->bytecode->emit(POP);
    }
}

void StmtVisitor::visit(FunctionNode &function_node)
{
    VIA_OPERAND function_reg = allocator.allocate_register();

    // Push function to the function stack
    program->test_stack->function_stack.push({
        .is_global = function_node.is_global,
        .modifiers = function_node.modifiers,
        .identifier = function_node.identifier,
        .parameters = function_node.parameters,
    });

    program->bytecode->emit(
        LOADFUNCTION,
        {function_reg},
        std::format(
            "{} func {}",
            function_node.is_global ? "global" : "local",
            function_node.identifier.lexeme
        )
    );

    ScopeNode &scope = dynamic_cast<ScopeNode &>(*function_node.body);
    for (const pStmtNode &pstmt : scope.statements) {
        const StmtNode &stmt = *pstmt;
        if (IS_INHERITOR(stmt, DeclarationNode) || IS_INHERITOR(stmt, FunctionNode)) {
            bool is_global;
            U32 identifier_position;

            if IS_INHERITOR (stmt, DeclarationNode) {
                const DeclarationNode &node = dynamic_cast<const DeclarationNode &>(stmt);
                is_global = node.is_global;
                identifier_position = node.identifier.position;
            }
            else {
                const FunctionNode &node = dynamic_cast<const FunctionNode &>(stmt);
                is_global = node.is_global;
                identifier_position = node.identifier.position;
            }

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

    Token symbol_token = function_node.identifier;
    std::string symbol = symbol_token.lexeme;
    U32 symbol_hash = hash_string(symbol.c_str());

    if (function_node.is_global) {
        if (program->globals->was_declared(symbol)) {
            visitor_failed = true;
            emitter.out(
                symbol_token.position,
                std::format("Attempt to re-declare global '{}'", symbol),
                Error
            );
            return;
        }

        program->bytecode->emit(
            SETGLOBAL,
            {
                function_reg,
                static_cast<VIA_OPERAND>(symbol_hash & 0xFFFF),
                static_cast<VIA_OPERAND>(symbol_hash >> 16),
            }
        );
    }
    else {
        program->bytecode->emit(PUSH, {function_reg});
    }

    allocator.free_register(function_reg);

    program->test_stack->function_stack.pop();
    program->test_stack->push({
        .symbol = symbol,
        .is_const = function_node.modifiers.is_const,
        .is_constexpr = false,
        .primitive_type = ValueType::function,
    });
}

void StmtVisitor::visit(AssignNode &assign_node)
{
    Token symbol_token = assign_node.identifier;
    std::string symbol = symbol_token.lexeme;
    std::optional<VIA_OPERAND> stk_id = program->test_stack->find_symbol({
        .symbol = symbol,
    });

    if (stk_id.has_value()) {
        const auto &test_stack_member = program->test_stack->at(stk_id.value());
        if (test_stack_member.has_value() && test_stack_member->is_const) {
            visitor_failed = true;
            emitter.out(
                symbol_token.position,
                std::format("Attempt to modify constant variable '{}'", symbol),
                Error
            );

            return;
        }

        VIA_OPERAND value_reg = allocator.allocate_register();
        assign_node.value->accept(expression_visitor, value_reg);
        program->bytecode->emit(SETSTACK, {value_reg, stk_id.value()});
    }
    else {
        visitor_failed = true;
        emitter.out(symbol_token.position, "Attempt to assign to undeclared symbol", Error);
    }
}

void StmtVisitor::visit(IfNode &) {}

void StmtVisitor::visit(WhileNode &while_node)
{
    ScopeNode &body = dynamic_cast<ScopeNode &>(*while_node.body);
    VIA_OPERAND cond_reg = allocator.allocate_register();

    VISITOR_DEF_LABEL(cond_label);

    while_node.condition->accept(expression_visitor, cond_reg);

    VISITOR_DEF_LABEL(body_label);

    body.accept(*this);

    // Account for conditional jump and jump-back instructions (+2)
    I32 body_delta = VISITOR_GET_LABEL(body_label) + 2;
    I32 cond_delta = VISITOR_GET_LABEL(cond_label) + 2;

    program->bytecode->emit(JUMP, {static_cast<VIA_OPERAND>(-cond_delta)});
    program->bytecode->insert(
        body_label,
        JUMPIFNOT,
        {cond_reg, static_cast<VIA_OPERAND>(body_delta)},
        std::format("while R{}", cond_reg)
    );

    allocator.free_register(cond_reg);
}

void StmtVisitor::visit(ExprStmtNode &expr_stmt)
{
    VIA_OPERAND trash_register = allocator.allocate_register();
    expr_stmt.expression->accept(expression_visitor, trash_register);
    allocator.free_register(trash_register);
}

} // namespace via
