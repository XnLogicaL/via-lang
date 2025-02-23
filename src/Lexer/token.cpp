// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GPL v3.           |
// =========================================================================================== |

#include "token.h"
#include "common.h"

namespace via {

using enum TokenType;

std::string Token::to_string() const noexcept
{
    std::basic_string_view<char> name = magic_enum::enum_name(type);
    std::string fmt = std::format(
        "Token(type: {}, value: '{}', line: {}, offset: {})", name, lexeme, line, offset
    );

    return fmt;
}

bool Token::is_literal() const noexcept
{
    return type == LIT_BOOL || type == LIT_FLOAT || type == LIT_INT || type == LIT_STRING;
}

bool Token::is_operator() const noexcept
{
    return type == OP_ADD || type == OP_DECREMENT || type == OP_DIV || type == OP_EQ ||
           type == OP_EXP || type == OP_GEQ || type == OP_GT || type == OP_INCREMENT ||
           type == OP_LEQ || type == OP_LT || type == OP_MOD || type == OP_MUL || type == OP_NEQ ||
           type == OP_SUB;
}

bool Token::is_modifier() const noexcept
{
    return type == KW_CONST;
}

int Token::bin_prec() const noexcept
{
    switch (type) {
    case OP_EXP:
        return 4;
    case OP_MUL:
    case OP_DIV:
    case OP_MOD:
        return 3;
    case OP_ADD:
    case OP_SUB:
        return 2;
    case OP_EQ:
    case OP_NEQ:
    case OP_LT:
    case OP_GT:
    case OP_LEQ:
    case OP_GEQ:
        return 1;
    default:
        return -1;
    }
}

} // namespace via
