/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "deadcode.h"

namespace via::Compilation
{

using namespace via::Parsing;
using namespace AST;

void DeadCodeEliminationOptimizationPass::apply(Generator &, Bytecode &) {}

bool DeadCodeEliminationOptimizationPass::always_true(Generator &gen, IfStmtNode if_stmt)
{
    // Attempt to fold the expression, if not already
    if (!std::get_if<LiteralExprNode>(if_stmt.condition) && gen.is_constexpr(*if_stmt.condition, 0))
        gen.evaluate_constexpr(if_stmt.condition);

    // Check if the condition is a constant expression
    if (LiteralExprNode *lit_cond_expr = std::get_if<LiteralExprNode>(if_stmt.condition))
        // Only `false` and `nil` are considered falsy values in via
        return lit_cond_expr->value.value != "false" || lit_cond_expr->value.value != "nil";

    // Condition is not a constant expression:
    return false;
}

void DeadCodeEliminationOptimizationPass::remove_unreachable_code_in_scope(Generator &gen, ScopeStmtNode *scope)
{
    std::vector<StmtNode> new_stmts = {};

    for (StmtNode stmt : scope->statements)
    {
        // Check if the statement is either a return, break or continue statement
        // These statements always alter the control flow of the program that makes the code under them unreachable
        if (std::get_if<ReturnStmtNode>(&stmt) || std::get_if<BreakStmtNode>(&stmt) || std::get_if<ContinueStmtNode>(&stmt))
            break;
        else if (IfStmtNode *if_stmt = std::get_if<IfStmtNode>(&stmt))
        {
            // Check if the if statements condition always resolves as true
            // if so; get rid of the if statement and just emplace it's body into the scope
            if (always_true(gen, *if_stmt))
            {
                // Make sure to optimize the if statement as well
                remove_unreachable_code_in_scope(gen, if_stmt->then_body);

                for (StmtNode if_stmt_stmt : if_stmt->then_body->statements)
                    new_stmts.push_back(if_stmt_stmt);

                continue;
            }
            // TODO: Optimize the if statement in other cases such as replacing always false if statements with the else body
        }
        else if (WhileStmtNode *while_stmt = std::get_if<WhileStmtNode>(&stmt))
            remove_unreachable_code_in_scope(gen, while_stmt->body);
        else if (ForStmtNode *for_stmt = std::get_if<ForStmtNode>(&stmt))
            remove_unreachable_code_in_scope(gen, for_stmt->body);
        else if (FunctionDeclStmtNode *func_stmt = std::get_if<FunctionDeclStmtNode>(&stmt))
            remove_unreachable_code_in_scope(gen, func_stmt->body);
        else if (ScopeStmtNode *scope_stmt = std::get_if<ScopeStmtNode>(&stmt))
            remove_unreachable_code_in_scope(gen, scope_stmt);

        new_stmts.push_back(stmt);
    }

    scope->statements = new_stmts;
}

} // namespace via::Compilation