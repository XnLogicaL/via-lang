/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <cstddef>
#include <via/config.hpp>
#include "location.hpp"
#include "support/utility.hpp"

namespace via {

#define FOR_EACH_TOKEN_KIND(X)                                                           \
    X(EOF_)                                                                              \
    X(ILLEGAL)                                                                           \
    X(IDENTIFIER)                                                                        \
    X(LIT_NIL)                                                                           \
    X(LIT_INT)                                                                           \
    X(LIT_BINT)                                                                          \
    X(LIT_XINT)                                                                          \
    X(LIT_FLOAT)                                                                         \
    X(LIT_TRUE)                                                                          \
    X(LIT_FALSE)                                                                         \
    X(LIT_STRING)                                                                        \
    X(KW_VAR)                                                                            \
    X(KW_CONST)                                                                          \
    X(KW_FN)                                                                             \
    X(KW_TYPE)                                                                           \
    X(KW_WHILE)                                                                          \
    X(KW_FOR)                                                                            \
    X(KW_IF)                                                                             \
    X(KW_IN)                                                                             \
    X(KW_OF)                                                                             \
    X(KW_ELSE)                                                                           \
    X(KW_DO)                                                                             \
    X(KW_AND)                                                                            \
    X(KW_OR)                                                                             \
    X(KW_NOT)                                                                            \
    X(KW_RETURN)                                                                         \
    X(KW_AS)                                                                             \
    X(KW_IMPORT)                                                                         \
    X(KW_MODULE)                                                                         \
    X(KW_STRUCT)                                                                         \
    X(KW_ENUM)                                                                           \
    X(KW_USING)                                                                          \
    X(KW_BOOL)                                                                           \
    X(KW_INT)                                                                            \
    X(KW_FLOAT)                                                                          \
    X(KW_STRING)                                                                         \
    X(PERIOD)                                                                            \
    X(COMMA)                                                                             \
    X(SEMICOLON)                                                                         \
    X(COLON)                                                                             \
    X(COLON_COLON)                                                                       \
    X(ARROW)                                                                             \
    X(QUESTION)                                                                          \
    X(PAREN_OPEN)                                                                        \
    X(PAREN_CLOSE)                                                                       \
    X(BRACKET_OPEN)                                                                      \
    X(BRACKET_CLOSE)                                                                     \
    X(BRACE_OPEN)                                                                        \
    X(BRACE_CLOSE)                                                                       \
    X(OP_PLUS)                                                                           \
    X(OP_MINUS)                                                                          \
    X(OP_STAR)                                                                           \
    X(OP_SLASH)                                                                          \
    X(OP_STAR_STAR)                                                                      \
    X(OP_PERCENT)                                                                        \
    X(OP_AMP)                                                                            \
    X(OP_TILDE)                                                                          \
    X(OP_CARET)                                                                          \
    X(OP_PIPE)                                                                           \
    X(OP_SHL)                                                                            \
    X(OP_SHR)                                                                            \
    X(OP_BANG)                                                                           \
    X(OP_LT)                                                                             \
    X(OP_GT)                                                                             \
    X(OP_DOT_DOT)                                                                        \
    X(OP_PLUS_PLUS)                                                                      \
    X(OP_MINUS_MINUS)                                                                    \
    X(OP_EQ)                                                                             \
    X(OP_EQ_EQ)                                                                          \
    X(OP_PLUS_EQ)                                                                        \
    X(OP_MINUS_EQ)                                                                       \
    X(OP_STAR_EQ)                                                                        \
    X(OP_SLASH_EQ)                                                                       \
    X(OP_STAR_STAR_EQ)                                                                   \
    X(OP_PERCENT_EQ)                                                                     \
    X(OP_AMP_EQ)                                                                         \
    X(OP_CARET_EQ)                                                                       \
    X(OP_PIPE_EQ)                                                                        \
    X(OP_SHL_EQ)                                                                         \
    X(OP_SHR_EQ)                                                                         \
    X(OP_BANG_EQ)                                                                        \
    X(OP_LT_EQ)                                                                          \
    X(OP_GT_EQ)                                                                          \
    X(OP_DOT_DOT_EQ)

enum class TokenKind
{
    FOR_EACH_TOKEN_KIND(DEFINE_ENUM)
};

DEFINE_TO_STRING(TokenKind, FOR_EACH_TOKEN_KIND(DEFINE_CASE_TO_STRING))

struct Token
{
    TokenKind kind;
    const char* lexeme;
    size_t size;

    std::string get_dump() const;
    std::string to_string() const { return std::string(lexeme, size); }
    std::string_view to_sv() const { return std::string_view(lexeme, size); }
    SourceLoc location(const std::string& source) const;
};

} // namespace via
