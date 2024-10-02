#pragma once

#include <string>

enum TokenType
{
    IDENTIFIER,
    TYPE,
    INT_LIT,
    FLOAT_LIT,
    STRING_LIT,
    BOOL_LIT,
    ASSIGN,
    EQU,
    PLUS,
    MINUS,
    END,
    START,
    ERROR,
    LCURLY,
    RCURLY,
    LSQU,
    RSQU,
    LPAR,
    RPAR,
    COMMA,
    COLON,
    SEMICOL,
    KEYWORD,
    ASTER,
    FSLASH,
    EXCLAM,
    DOUBLE_QUOTE,
};