/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "gen.h"
#include "instruction.h"
#include <cmath>

#ifndef VIA_CONSTEXPR_MAX_DEPTH
#    define VIA_CONSTEXPR_MAX_DEPTH 5
#endif

namespace via::Compilation
{

using namespace via::Parsing;
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

// Generates bytecode and returns it
std::unique_ptr<Bytecode> Generator::generate()
{
    for (StmtNode stmt : bytecode->ast->statements)
        generate_statement(stmt);

    // Transfer ownership of bytecode object
    return std::move(bytecode);
}

// Returns the successor of iota everytime it's called
size_t Generator::iota()
{
    return ++__iota__;
}

bool Generator::is_constexpr(ExprNode node, int current_depth)
{
    if (current_depth > VIA_CONSTEXPR_MAX_DEPTH)
        // Stack search-depth limit exceeded
        return false;

    if (std::get_if<LiteralExprNode>(&node))
        // Literal is constant
        return true;
    else if (VarExprNode *var_expr = std::get_if<VarExprNode>(&node))
    {
        // Check if the constant has been declared in the stack
        std::optional<ExprNode> const_var = stack.get_constant(var_expr->ident.value);
        if (const_var.has_value())
            // Variable is constant, check further
            return is_constexpr(const_var.value(), current_depth + 1);

        // Variable is not constant
        return false;
    }
    else if (BinaryExprNode *bin_expr = std::get_if<BinaryExprNode>(&node))
    {
        // Check both lhs and rhs of the binary expression
        bool lhs_constexpr = is_constexpr(*bin_expr->lhs, current_depth + 1);
        bool rhs_constexpr = is_constexpr(*bin_expr->rhs, current_depth + 1);
        return lhs_constexpr && rhs_constexpr;
    }
    else if (UnaryExprNode *un_expr = std::get_if<UnaryExprNode>(&node))
        return is_constexpr(*un_expr->expr, current_depth + 1);

    // Non-constant expression type
    // CallExpr, IndexExpr, etc.
    return false;
}

void Generator::evaluate_constexpr(ExprNode *expr)
{
    // Check if the expression is already literal,
    // which means it already has been evaluated
    if (std::get_if<LiteralExprNode>(expr))
        return;

    if (UnaryExprNode *un_expr = std::get_if<UnaryExprNode>(expr))
    {
        ExprNode *inner_expr = un_expr->expr;
        evaluate_constexpr(inner_expr);
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
        *expr = *alloc.emplace<ExprNode>(*inner_literal);
    }
    else if (BinaryExprNode *bin_expr = std::get_if<BinaryExprNode>(expr))
    {
        ExprNode *lhs_expr = bin_expr->lhs;
        ExprNode *rhs_expr = bin_expr->rhs;
        evaluate_constexpr(lhs_expr);
        evaluate_constexpr(rhs_expr);
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
            LiteralExprNode *result_lit_expr = alloc.emplace<LiteralExprNode>();
            result_lit_expr->value = {
                .type = TokenType::LIT_FLOAT,
                .value = std::to_string(result),
                .line = lhs_lit_expr->value.line,
                .offset = lhs_lit_expr->value.offset,
                .has_thrown_error = false,
            };

            // Replace the old expression node with the new evaluated literal expression
            *expr = *alloc.emplace<ExprNode>(*result_lit_expr);
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
            LiteralExprNode *result_lit_expr = alloc.emplace<LiteralExprNode>();
            result_lit_expr->value = {
                .type = TokenType::LIT_BOOL,
                .value = std::string(result ? "true" : "false"),
                .line = lhs_lit_expr->value.line,
                .offset = lhs_lit_expr->value.offset,
                .has_thrown_error = false,
            };

            // Replace the old expression node with the new evaluated literal expression
            *expr = *alloc.emplace<ExprNode>(*result_lit_expr);
        }
        else if (BinaryExprNode *bin_expr = std::get_if<BinaryExprNode>(expr))
        {
            ExprNode *lhs_expr = bin_expr->lhs;
            ExprNode *rhs_expr = bin_expr->rhs;
            evaluate_constexpr(lhs_expr);
            evaluate_constexpr(rhs_expr);

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
                LiteralExprNode *result_lit_expr = alloc.emplace<LiteralExprNode>();
                result_lit_expr->value = {
                    .type = TokenType::LIT_BOOL,
                    .value = std::string(result ? "true" : "false"),
                    .line = lhs_lit_expr->value.line,
                    .offset = lhs_lit_expr->value.offset,
                    .has_thrown_error = false,
                };

                // Replace the old expression node with the new evaluated literal expression
                *expr = *alloc.emplace<ExprNode>(*result_lit_expr);
            }
        }
    }
    else if (VarExprNode *var_expr = std::get_if<VarExprNode>(expr))
    {
        // Search the stack for the requested variable
        std::optional<ExprNode> next_expr = stack.get_constant(var_expr->ident.value);
        // If not found, we can't evaluate the expression
        if (!next_expr.has_value())
            return;

        // Evaluate the expression
        evaluate_constexpr(&next_expr.value());
        // Replace the old expression node with the found stack node
        *expr = next_expr.value();
    }
}

// Pushes a bytecode instruction
void Generator::push_instruction(OpCode op, std::vector<Operand> operands)
{
    Instruction instruction = viaC_newinstruction(op, operands);

    if (initialize_with_chunk)
    {
        initialize_with_chunk = false;
        instruction.chunk = current_chunk;
    }

    bytecode->add_instruction(instruction);
}

} // namespace via::Compilation