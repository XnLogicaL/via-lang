/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "constfold.h"
#include "Parser/ast.h"
#include "arena.hpp"
#include "token.h"

#include <cmath>

using namespace via::Parsing;
using namespace via::Tokenization;

namespace via::Compilation
{

// Returns if the given expression is a constant expression that can be evaluated by the compiler
bool _is_constexpr(const Parsing::AST::ExprNode &expr)
{
    if (auto lit = std::get_if<AST::LiteralExprNode>(&expr))
        // Check literal type to make sure that it's a number
        return lit->value.type == TokenType::LIT_INT || lit->value.type == TokenType::LIT_FLOAT;
    else if (auto un_expr = std::get_if<AST::UnaryExprNode>(&expr))
        return _is_constexpr(*un_expr->expr);
    else if (auto group_expr = std::get_if<AST::GroupExprNode>(&expr))
        return _is_constexpr(*group_expr->expr);
    else if (auto bin_expr = std::get_if<AST::BinaryExprNode>(&expr))
        return _is_constexpr(*bin_expr->lhs) && _is_constexpr(*bin_expr->rhs);

    return false;
}

// Evaluates a literal expression
// Can only be a number literal
// ! Does not perform sanity checks
double _eval_litexpr(const Parsing::AST::LiteralExprNode &lit_expr)
{
    return std::stod(lit_expr.value.value);
}

// Evaluates a binary expression using `_eval_litexpr` on lvalue and rvalue
double _eval_binexpr(const Parsing::AST::BinaryExprNode &bin_expr)
{
    double lhs = _eval_litexpr(std::get<AST::LiteralExprNode>(*bin_expr.lhs));
    double rhs = _eval_litexpr(std::get<AST::LiteralExprNode>(*bin_expr.rhs));

    switch (bin_expr.op.type)
    {
    case TokenType::OP_ADD:
        return lhs + rhs;
    case TokenType::OP_SUB:
        return lhs - rhs;
    case TokenType::OP_MUL:
        return lhs * rhs;
    case TokenType::OP_DIV:
        if (rhs == 0.0)
            throw std::runtime_error("Division by zero in constant folding");
        return lhs / rhs;
    case TokenType::OP_MOD:
        return std::fmod(lhs, rhs);
    case TokenType::OP_EXP:
        return std::pow(lhs, rhs);
    default:
        throw std::runtime_error("Unsupported binary operator in constant folding");
    }
}

double _eval_expr(const Parsing::AST::ExprNode &expr)
{
    if (auto lit = std::get_if<AST::LiteralExprNode>(&expr))
        return _eval_litexpr(*lit);
    else if (auto un = std::get_if<AST::UnaryExprNode>(&expr))
        return _eval_expr(*un->expr);
    else if (auto grp = std::get_if<AST::GroupExprNode>(&expr))
        return _eval_expr(*grp->expr);
    else if (auto bin = std::get_if<AST::BinaryExprNode>(&expr))
        return _eval_binexpr(*bin);

    return 0.0f;
}

void optimize_constfold(Parsing::AST::ExprNode &expr)
{
    if (!_is_constexpr(expr))
        return;

    double expr_result = _eval_expr(expr);
    Token expr_token{TokenType::LIT_FLOAT, std::to_string(expr_result), 0, 0};
    AST::LiteralExprNode new_expr(expr_token);
    expr = new_expr;
}

} // namespace via::Compilation
