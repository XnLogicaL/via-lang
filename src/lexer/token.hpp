#pragma once

#include <string>
#include <ostream>
#include <optional>

#include "tokentype.hpp"

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
            + ("Type: " + TokenType_to_string(type))
            + (", Value: " + value)
            + (", Line: " + std::to_string(line_cpy))
            + (", Column: " + std::to_string(col_cpy))
            + ")";

        return token_string;
    }
};

inline std::optional<int> bin_prec(const TokenType type)
{
    switch (type)
    {
    case TokenType::DB_EQUALS:
    case TokenType::MINUS:
    case TokenType::PLUS:
        return 0;
    case TokenType::F_SLASH:
    case TokenType::ASTERISK:
        return 1;
    default:
        return {};
    }
}

const Token NULL_TOKEN = { TokenType::ERROR, "<null>", -1, -1 };