/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "constfold.h"

namespace via::Compilation
{

using namespace via::Parsing;
using namespace via::Tokenization;
using namespace AST;

template<typename T, typename K>
using BinaryEvaluator = std::function<K(T, T)>;

// Utility function for checking if an expression holds a number literal
bool is_number_literal(LiteralExprNode lit_expr)
{
    return lit_expr.value.type == TokenType::LIT_INT || lit_expr.value.type == TokenType::LIT_FLOAT;
}

// Utility function for checking if an operator type is an arithmetic operator
bool is_arithmetic_operator(TokenType op)
{
    return op == TokenType::OP_ADD || op == TokenType::OP_SUB || op == TokenType::OP_MUL || op == TokenType::OP_MOD || op == TokenType::OP_EXP;
}

// Utility function for checking if an operator type is a comparison operator
bool is_comparison_operator(TokenType op)
{
    return op == TokenType::OP_EQ || op == TokenType::OP_NEQ;
}

// Utility function for checking if an operator type is a numeric comparsion operator
bool is_numeric_comparison_operator(TokenType op)
{
    return op == TokenType::OP_LEQ || op == TokenType::OP_GEQ || op == TokenType::OP_LT || op == TokenType::OP_GT;
}

// Utility function to check if two token types are compatible for comparison
bool are_compatible_types(TokenType lhs_type, TokenType rhs_type)
{
    return (lhs_type == TokenType::LIT_INT && rhs_type == TokenType::LIT_FLOAT) ||
           (lhs_type == TokenType::LIT_FLOAT && rhs_type == TokenType::LIT_INT) || lhs_type == rhs_type; // Same types are always compatible
}

// Evaluates a constant expression, helper method for fold_constexpr
void ConstFoldOptimizationPass::evaluate_constexpr(Generator &gen, ExprNode *expr)
{
    // Check if the expression is already literal,
    // which means it already has been evaluated
    if (std::get_if<LiteralExprNode>(expr))
        return;

    if (UnaryExprNode *un_expr = std::get_if<UnaryExprNode>(expr))
    {
        ExprNode *inner_expr = un_expr->expr;
        evaluate_constexpr(gen, inner_expr);
        // This is declared after the evaluate_constexpr call because the inner expression might not be a literal initially
        LiteralExprNode *inner_literal = std::get_if<LiteralExprNode>(inner_expr);
        // Check if the inner expression has collapsed
        if (!inner_literal)
            // We can't negate a non-constant expression
            return;

        // Negation is only supported on numbers (when the expression is constant)
        if (is_number_literal(*inner_literal))
            // Add an unmary operator to the inner expressions value
            inner_literal->value.value = std::string("-") + inner_literal->value.value;

        // Overwrite the old expression with the new modified & evaluated literal
        *expr = *gen.alloc.emplace<ExprNode>(*inner_literal);
    }
    else if (GroupExprNode *grp_expr = std::get_if<GroupExprNode>(expr))
    {
        ExprNode *inner_expr = grp_expr->expr;
        evaluate_constexpr(gen, inner_expr);
        // This is declared after the evaluate_constexpr call because the inner expression might not be a literal initially
        LiteralExprNode *inner_literal = std::get_if<LiteralExprNode>(inner_expr);
        // Check if the inner expression has collapsed
        if (!inner_literal)
            // We can't negate a non-constant expression
            return;

        // Negation is only supported on numbers (when the expression is constant)
        if (is_number_literal(*inner_literal))
            inner_literal->value.value = std::string("-") + inner_literal->value.value;

        // Overwrite the old expression with the new modified & evaluated literal
        *expr = *gen.alloc.emplace<ExprNode>(*inner_literal);
    }
    else if (BinaryExprNode *bin_expr = std::get_if<BinaryExprNode>(expr))
    {
        ExprNode *lhs_expr = bin_expr->lhs;
        ExprNode *rhs_expr = bin_expr->rhs;
        evaluate_constexpr(gen, lhs_expr);
        evaluate_constexpr(gen, rhs_expr);
        // Save both literals for later use
        LiteralExprNode *lhs_lit_expr = std::get_if<LiteralExprNode>(lhs_expr);
        LiteralExprNode *rhs_lit_expr = std::get_if<LiteralExprNode>(rhs_expr);
        // Check if boths sides of the binary expression have collapsed
        if (!lhs_lit_expr || !rhs_lit_expr)
            // We can't evaluate the binary expression if both sides aren't literal at this point
            return;

        // Check if the binary operator is an arithmetic operator
        if (is_arithmetic_operator(bin_expr->op.type))
        {
            // Check if both sides are number literals
            if (!is_number_literal(*lhs_lit_expr) || !is_number_literal(*rhs_lit_expr))
                return;

            // Convert literals into numbers
            double lhs_number = std::stod(lhs_lit_expr->value.value);
            double rhs_number = std::stod(rhs_lit_expr->value.value);
            double result;

            // Map of lambda functions for quickly evaluating lhs and rhs without switch statements
            static std::unordered_map<TokenType, BinaryEvaluator<double, double>> evaluator_map = {
                // clang-format off
                {TokenType::OP_ADD, [](double x, double y) -> double { return x + y; }},
                {TokenType::OP_SUB, [](double x, double y) -> double { return x - y; }},
                {TokenType::OP_MUL, [](double x, double y) -> double { return x * y; }},
                {TokenType::OP_DIV, [](double x, double y) -> double { return x / y; }},
                {TokenType::OP_EXP, [](double x, double y) -> double { return std::pow(x, y); }},
                {TokenType::OP_MOD, [](double x, double y) -> double { return std::fmod(x, y); }},
            }; // clang-format on

            // Find the appropriate evaluator by indexing with our TokenType
            BinaryEvaluator<double, double> evaluator = evaluator_map[bin_expr->op.type];
            // Call the evaluator and save it as result
            result = evaluator(lhs_number, rhs_number);

            // Create a new literal expression node containing the result
            LiteralExprNode *result_lit_expr = gen.alloc.emplace<LiteralExprNode>();
            result_lit_expr->value = {
                .type = TokenType::LIT_FLOAT,
                .value = std::to_string(result),
                .line = lhs_lit_expr->value.line,
                .offset = lhs_lit_expr->value.offset,
                .has_thrown_error = false,
            };

            // Replace the old expression node with the new evaluated literal expression
            *expr = *gen.alloc.emplace<ExprNode>(*result_lit_expr);
        }
        else if (is_numeric_comparison_operator(bin_expr->op.type))
        {
            // Check if both sides are number literals
            if (!is_number_literal(*lhs_lit_expr) || !is_number_literal(*rhs_lit_expr))
                return;

            // Convert literals into numbers
            double lhs_number = std::stod(lhs_lit_expr->value.value);
            double rhs_number = std::stod(rhs_lit_expr->value.value);
            bool result;

            // Map of lambda functions for quickly evaluating lhs and rhs without switch statements
            static std::unordered_map<TokenType, BinaryEvaluator<bool, double>> evaluator_map = {
                // clang-format off
                {TokenType::OP_LT, [](double x, double y) -> bool { return x < y; }},
                {TokenType::OP_GT, [](double x, double y) -> bool { return x > y; }},
                {TokenType::OP_LEQ, [](double x, double y) -> bool { return x <= y; }},
                {TokenType::OP_GEQ, [](double x, double y) -> bool { return x >= y; }},
            }; // clang-format on

            // Find evaluator
            BinaryEvaluator<bool, double> evaluator = evaluator_map[bin_expr->op.type];
            // Call evaluator and save as result
            result = evaluator(lhs_number, rhs_number);

            // Create a new literal expression node containing the result
            LiteralExprNode *result_lit_expr = gen.alloc.emplace<LiteralExprNode>();
            result_lit_expr->value = {
                .type = TokenType::LIT_BOOL,
                .value = std::string(result ? "true" : "false"),
                .line = lhs_lit_expr->value.line,
                .offset = lhs_lit_expr->value.offset,
                .has_thrown_error = false,
            };

            // Replace the old expression node with the new evaluated literal expression
            *expr = *gen.alloc.emplace<ExprNode>(*result_lit_expr);
        }
        else if (BinaryExprNode *bin_expr = std::get_if<BinaryExprNode>(expr))
        {
            ExprNode *lhs_expr = bin_expr->lhs;
            ExprNode *rhs_expr = bin_expr->rhs;
            evaluate_constexpr(gen, lhs_expr);
            evaluate_constexpr(gen, rhs_expr);

            LiteralExprNode *lhs_lit_expr = std::get_if<LiteralExprNode>(lhs_expr);
            LiteralExprNode *rhs_lit_expr = std::get_if<LiteralExprNode>(rhs_expr);

            // Check if both sides of the binary expression are literals
            if (!lhs_lit_expr || !rhs_lit_expr)
                return;

            // Handle comparison operators
            if (is_comparison_operator(bin_expr->op.type))
            {
                bool result = false;

                // Quick return for incompatible types
                if (!are_compatible_types(lhs_lit_expr->value.type, rhs_lit_expr->value.type))
                    // Skip directly to the replacement block with false
                    goto eval;

                // Evaluate the comparison
                switch (bin_expr->op.type)
                {
                case TokenType::OP_EQ:
                    result = lhs_lit_expr->value.value == rhs_lit_expr->value.value;
                    break;
                case TokenType::OP_NEQ:
                    result = lhs_lit_expr->value.value != rhs_lit_expr->value.value;
                    break;
                default:
                    UNREACHABLE();
                    break;
                }

            eval:
                // Create a new literal expression node containing the result
                LiteralExprNode *result_lit_expr = gen.alloc.emplace<LiteralExprNode>();
                result_lit_expr->value = {
                    .type = TokenType::LIT_BOOL,
                    .value = std::string(result ? "true" : "false"),
                    .line = lhs_lit_expr->value.line,
                    .offset = lhs_lit_expr->value.offset,
                    .has_thrown_error = false,
                };

                // Replace the old expression node with the new evaluated literal expression
                *expr = *gen.alloc.emplace<ExprNode>(*result_lit_expr);
            }
        }
    }
    else if (VarExprNode *var_expr = std::get_if<VarExprNode>(expr))
    {
        // Search the stack for the requested variable
        std::optional<ExprNode> next_expr = gen.stack.get_constant(var_expr->ident.value);
        // If not found, we can't evaluate the expression
        if (!next_expr.has_value())
            return;

        // Evaluate the expression
        evaluate_constexpr(gen, &next_expr.value());
        // Replace the old expression node with the found stack node
        *expr = next_expr.value();
    }
}

// Folds a constant expression into a constant value
// eg. `1 + 2 * 4` into `9`
void ConstFoldOptimizationPass::fold_constexpr(Generator &gen, ExprNode *expr)
{
    if (!gen.is_constexpr(*expr, 0))
        return;

    evaluate_constexpr(gen, expr);
}

// Folds all possible constants in the AST
void ConstFoldOptimizationPass::apply(Generator &gen, Bytecode &bytecode)
{
#define CHECK(ident, type) type *ident = std::get_if<type>(&stmt)
    for (StmtNode stmt : bytecode.ast->statements)
    {
        if (CHECK(decl_stmt, LocalDeclStmtNode))
            fold_constexpr(gen, &decl_stmt->value.value());
        else if (CHECK(decl_stmt, GlobalDeclStmtNode))
            fold_constexpr(gen, &decl_stmt->value.value());
        else if (CHECK(call_stmt, CallStmtNode))
        {
            fold_constexpr(gen, call_stmt->callee);
            for (ExprNode arg : call_stmt->args)
                fold_constexpr(gen, &arg);
        }
    }
#undef CHECK
}

bool ConstFoldOptimizationPass::is_applicable(const Bytecode &bytecode) const
{
    // Check if the AST has already been optimized
    return bytecode.ast != nullptr;
}

} // namespace via::Compilation
