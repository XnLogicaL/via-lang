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
    EQUALS,
    DB_EQUALS,
    PLUS,
    MINUS,
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
    ASTERISK,
    F_SLASH,
    EXCLAMATION,
    DOUBLE_QUOTE,
};

std::string TokenType_to_string(TokenType enum_token)
{
    switch (enum_token)
    {
    case KEYWORD:       return "KEYWORD";
    case IDENTIFIER:    return "IDENTIFIER";
    case TYPE:          return "TYPE";
    case INT_LIT:       return "INT_LIT";
    case FLOAT_LIT:     return "FLOAT_LIT";
    case STRING_LIT:    return "STRING_LIT";
    case BOOL_ALPHA:    return "BOOL_ALPHA";
    case PLUS:          return "PLUS";
    case MINUS:         return "MINUS";
    case START:         return "START";
    case END:           return "END";
    case ERROR:         return "ERROR";
    case EQUALS:        return "EQUALS";
    case DB_EQUALS:     return "DOUBLE_EQUALS";
    case L_PAR:         return "L_PAR";
    case R_PAR:         return "R_PAR";
    case L_CR_BRACKET:  return "L_CR_BRACKET";
    case R_CR_BRACKET:  return "R_CR_BRACKET";
    case L_SQ_BRACKET:  return "L_SQ_BRACKET";
    case R_SQ_BRACKET:  return "R_SQ_BRACKET";
    case COMMA:         return "COMMA";
    case SEMICOLON:     return "SEMICOLON";
    case COLON:         return "COLON";
    case ASTERISK:      return "ASTERISK";
    case F_SLASH:       return "F_SLASH";
    case EXCLAMATION:   return "EXCLAMATION";
    case DOUBLE_QUOTE:  return "DOUBLE_QUOTE";
    default:
        return "UNKNOWN(" + std::to_string(enum_token) + ")";
    }
}