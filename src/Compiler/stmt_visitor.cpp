// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "visitor.h"
#include "stack.h"

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
                std::format("Attempt to reassign global '{}'", symbol);

            visitor_failed = true;

            emitter.out(ident.position, previously_declared_error, Error);
            emitter.out(previously_declared->declared_at, "Previously declared here", Info);
        }
        else {
            U32 value_reg = allocator.allocate_register();
            U32 symbol_hash = hash_string(symbol.c_str());

            declaration_node.value_expression->accept(expression_visitor, value_reg);

            Global global(symbol, ident.position);
            program->globals->declare_global(global);
            program->bytecode->emit(SETGLOBAL, {value_reg, symbol_hash}, comment);

            allocator.free_register(value_reg);
        }
    }
    else {
        std::string comment = std::format("local {}", symbol);

        if IS_CONSTEXPR (val) {
            const TValue &constant = construct_constant(dynamic_cast<LiteralNode &>(val));
            const U32 const_id = PUSH_K(constant);

            program->bytecode->emit(PUSHK, {const_id}, comment);
            program->test_stack->push({
                .symbol = symbol,
                .is_const = is_const,
                .is_constexpr = true,
            });
        }
        else {
            U32 dst = allocator.allocate_register();

            declaration_node.value_expression->accept(expression_visitor, dst);

            program->bytecode->emit(PUSH, {dst}, comment);
            program->test_stack->push({
                .symbol = symbol,
                .is_const = is_const,
            });

            allocator.free_register(dst);
        }
    }
}

void StmtVisitor::visit(ScopeNode &scope_node)
{
    U32 stack_pointer = program->test_stack->sp;

    for (const pStmtNode &pstmt : scope_node.statements) {
        pstmt->accept(*this);
    }

    U32 stack_allocations = program->test_stack->sp - stack_pointer;
    for (; stack_allocations > 0; stack_allocations--) {
        program->bytecode->emit(POP);
    }
}

void StmtVisitor::visit(FunctionNode &) {}
void StmtVisitor::visit(AssignNode &) {}
void StmtVisitor::visit(IfNode &) {}
void StmtVisitor::visit(WhileNode &) {}
void StmtVisitor::visit(ExprStmtNode &) {}

} // namespace via
