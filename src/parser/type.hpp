#pragma once

#include <string>
#include <variant>
#include <optional>
#include <iostream>

#include "ast/StmtNode.hpp"

enum IntermediateType
{
    NIL,
    INT,
    FLOAT,
    BOOL,
    STRING,

};

void error(std::string msg)
{
    std::cerr << msg << std::endl;
    exit(1);
}

namespace TypeChecker
{
    bool is_literal(ExprNode* expr)
    {
        return std::visit([&](const auto &_expr) {
            using T = std::decay_t<decltype(_expr)>;

            if constexpr (std::is_same_v<T, IntLitNode*>) return true;
            else if constexpr (std::is_same_v<T, StringLitNode*>) return true;
            else if constexpr (std::is_same_v<T, BoolLitNode*>) return true;
            else
                return false;
        }, expr->node);
    }

    template <typename _Ty>
    bool is_type(StmtNode* stmt)
    {
        return std::visit([&](const auto &_stmt) {
            using T = std::decay_t<decltype(_stmt)>;
            return std::is_same_v<T, _Ty>;
        }, stmt->stmt);
    }

    template <typename _Ty>
    bool is_type(ExprNode* expr)
    {
        return std::visit([&](const auto &_expr) {
            using T = std::decay_t<decltype(_expr)>;
            return std::is_same_v<T, _Ty>;
        }, expr->node);
    }

    IntermediateType as_itype(ExprNode* literal)
    {
        if (!is_literal(literal))
            error("cannot get intermediate type representation of a non-literal value");

        return std::visit([&](const auto &lit) {
            using T = std::decay_t<decltype(lit)>;

            if constexpr (std::is_same_v<T, IntLitNode*>)
                return IntermediateType::INT;
            else if constexpr (std::is_same_v<T, StringLitNode*>)
                return IntermediateType::STRING;
            else if constexpr (std::is_same_v<T, BoolLitNode*>)
                return IntermediateType::BOOL;
            else
                return IntermediateType::NIL;
        }, literal->node);
    }

    IntermediateType as_itype(Token literal)
    {
        switch (literal.type)
        {
        case TokenType::INT_LIT:
            return IntermediateType::INT;
        case TokenType::STRING_LIT:
            return IntermediateType::STRING;
        case TokenType::BOOL_ALPHA:
            return IntermediateType::BOOL;
        default:
            return IntermediateType::NIL;
        }
    }

} // namespace TypeChecker

namespace TypeConverter
{
    Token get_literal_value(ExprNode* lit)
    {
        if (!TypeChecker::is_literal(lit))
            error("TypeConverter: cannot get value of non-literal");

        std::visit([&](const auto& _literal) {
            return _literal->val;
        }, lit->node);
    }

    StringLitNode to_string(ExprNode* expr)
    {
        if (!TypeChecker::is_literal(expr))
            error("TypeConverter: cannot convert non-literal into string literal");

        StringLitNode converted_string;
        converted_string.val = get_literal_value(expr);

        return converted_string;
    }

    IntLitNode to_int(ExprNode* expr)
    {
        if (!TypeChecker::is_literal(expr))
            error("TypeConverter: cannot convert non-literal into int literal");

        IntLitNode converted_int;
        converted_int.val = get_literal_value(expr);

        return converted_int;
    }

} // namespace TypeConverter
