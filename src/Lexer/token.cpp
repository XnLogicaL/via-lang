#include "common.h"
#include "token.h"

#include "magic_enum/magic_enum.hpp"

using namespace via::Tokenization;

std::string Token::to_string() const noexcept
{
    auto fmt = std::format(
        "Token(type: {}, value: {}, line: {}, offset: {})",
        magic_enum::enum_name(type),
        value,
        std::to_string(line),
        std::to_string(offset)
    );

    return std::string(fmt).c_str();
}

bool Token::is_literal() const noexcept
{
    return type == TokenType::LIT_BOOL ||
        type == TokenType::LIT_CHAR ||
        type == TokenType::LIT_FLOAT ||
        type == TokenType::LIT_INT ||
        type == TokenType::LIT_STRING;
}

bool Token::is_operator() const noexcept
{
    return type == TokenType::OP_ADD ||
        type == TokenType::OP_DEC ||
        type == TokenType::OP_DIV ||
        type == TokenType::OP_EQ ||
        type == TokenType::OP_EXP ||
        type == TokenType::OP_GEQ ||
        type == TokenType::OP_GT ||
        type == TokenType::OP_INC ||
        type == TokenType::OP_LEQ ||
        type == TokenType::OP_LT ||
        type == TokenType::OP_MOD ||
        type == TokenType::OP_MUL ||
        type == TokenType::OP_NEQ ||
        type == TokenType::OP_SUB ||
        type == TokenType::OP_ASGN;
}

int Token::bin_prec() const noexcept
{
    switch (type)
    {
    case TokenType::OP_EXP:
        return 4;
    case TokenType::OP_MUL:
    case TokenType::OP_DIV:
    case TokenType::OP_MOD:
        return 3;
    case TokenType::OP_ADD:
    case TokenType::OP_SUB:
        return 2;
    case TokenType::OP_EQ:
    case TokenType::OP_NEQ:
    case TokenType::OP_LT:
    case TokenType::OP_GT:
    case TokenType::OP_LEQ:
    case TokenType::OP_GEQ:
        return 1;
    case TokenType::OP_ASGN:
        return 0;
    default:
        return -1;
    }
}