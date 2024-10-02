#pragma once
#include <string>
#include <ostream>
#include <optional>
#include "tokentype.hpp"
#include "../../include/magic_enum/magic_enum.hpp"
class Token
{
public:
    TokenType type;
    std::string value;
    int line;
    int column;
    std::string to_string() const
    {
        auto line_cpy = line;
        auto col_cpy = column;
        std::string token_string = "Token("
            + ("Type: " + std::string(magic_enum::enum_name(type)))
            + (", Value: " + value)
            + (", Line: " + std::to_string(line_cpy))
            + (", Column: " + std::to_string(col_cpy))
            + ")";
        return token_string;
    }
    bool is_operator() const
    {
        return type == TokenType::PLUS
            && type == TokenType::MINUS
            && type == TokenType::ASTER
            && type == TokenType::FSLASH
            && type == TokenType::EQU;
    }
    bool is_literal() const
    {
        return type == TokenType::STRING_LIT
            && type == TokenType::INT_LIT
            && type == TokenType::BOOL_LIT
            && type == TokenType::FLOAT_LIT;
    }
};
inline std::optional<int> bin_prec(const TokenType type)
{
    switch (type)
    {
    case TokenType::EQU:
    case TokenType::MINUS:
    case TokenType::PLUS:
        return 0;
    case TokenType::FSLASH:
    case TokenType::ASTER:
        return 1;
    default:
        return {};
    }
}
const Token NULL_TOKEN = { TokenType::ERROR, "<null>", -1, -1 };