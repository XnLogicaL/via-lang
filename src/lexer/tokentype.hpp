#pragma once

#include <string>

enum TokenType
{
    IDENTIFIER,
    TYPE,
    INT_LIT,
    FLOAT_LIT,
    STRING_LIT,
    BOOL_ALPHA,
    ASSIGN,
    EQU,
    ADD,
    SUB,
    END,
    START,
    ERROR,
    L_CR_BRACKET,
    R_CR_BRACKET,
    L_SQ_BRACKET,
    R_SQ_BRACKET,
    L_PAR,
    R_PAR,
    COMMA,
    COLON,
    SEMICOLON,
    KEYWORD,
    MUL,
    DIV,
    EXCLAMATION,
    DOUBLE_QUOTE,
};