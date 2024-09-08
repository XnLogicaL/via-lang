#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <sstream> // Removed <format> since we're using ostringstream
#include <map>

const char* Keywords[] = {
    "function",
    "local",
    "global",
    "return",
};

enum TokenType
{
    IDENTIFIER,
    TYPE,
    INT_LIT,
    FLOAT_LIT,
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
    EXCLAMATION
};

std::string token_to_string(TokenType enum_token)
{
    switch (enum_token)
    {
    case KEYWORD:       return "KEYWORD";
    case IDENTIFIER:    return "IDENTIFIER";
    case TYPE:          return "TYPE";
    case INT_LIT:       return "INT_LIT";
    case FLOAT_LIT:     return "FLOAT_LIT";
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
    case COLON:         return "COLON";
    case ASTERISK:      return "ASTERISK";
    case F_SLASH:       return "F_SLASH";
    case EXCLAMATION:   return "EXCLAMATION";
    default:            return "UNKNOWN";
    }
}

bool is_literal(TokenType type)
{
    return type == INT_LIT || type == FLOAT_LIT;
}

class Token
{
public:
    TokenType type;
    std::string value;
    int line;
    int column;

    friend std::ostream &operator<<(std::ostream &os, const Token &tok)
    {
        os << "Token("
           << "Type: " << token_to_string(tok.type)
           << ", Value: " << tok.value
           << ", Line: " << tok.line
           << ", Column: " << tok.column
           << ")";

        return os;
    }
};

class Lexer
{
public:
    Lexer(const std::string &src) : src(src), pos(-2), line(1), column(1) {}

    Token get_next_token(Token last_tok)
    {
        while (pos < src.length())
        {
            char current = src.at(pos);

            if (isspace(current))
            {
                consume_white_space();
                continue;
            }

            if (isalpha(current)) return read_identifier(last_tok);
            if (isdigit(current)) return read_number(last_tok);

            switch (current)
            {
            case '+':   return create_token(TokenType::PLUS, "+");
            case '-':   return create_token(TokenType::MINUS, "-");
            case '=':   return create_token(TokenType::EQUALS, "=");
            case '==':  return create_token(TokenType::DB_EQUALS, "==");
            case '{':   return create_token(TokenType::L_CR_BRACKET, "{");
            case '}':   return create_token(TokenType::R_CR_BRACKET, "}");
            case '[':   return create_token(TokenType::L_CR_BRACKET, "[");
            case ']':   return create_token(TokenType::L_CR_BRACKET, "]");
            case '(':   return create_token(TokenType::L_PAR, "(");
            case ')':   return create_token(TokenType::R_PAR, ")");
            case ',':   return create_token(TokenType::COMMA, ",");
            case ':':   return create_token(TokenType::COLON, ":");
            case '*':   return create_token(TokenType::ASTERISK, "*");
            case '/':   return create_token(TokenType::F_SLASH, "/");
            case '!':   return create_token(TokenType::EXCLAMATION, "!");
            default:    return create_token(TokenType::ERROR, std::string(1, current));
            }
        }

        return create_token(TokenType::ERROR, "<error-token>");
    }

    std::vector<Token> tokenize()
    {
        Token tok;
        std::vector<Token> tokens;
        Token last_token = create_token(TokenType::START, "");
        tokens.push_back(create_token(TokenType::L_CR_BRACKET, "{"));

        while (tok.type != TokenType::END)
        {
            tok = get_next_token(last_token);

            if (tok.type == TokenType::ERROR)
                break;

            tokens.push_back(tok);
            last_token = tok;
        }

        tokens.push_back(create_token(TokenType::R_CR_BRACKET, "}"));
        tokens.push_back(create_token(TokenType::END, "<eof>"));

        return tokens;
    }

private:
    std::string src;
    size_t pos;
    int line;
    int column;

    void consume_white_space()
    {
        while (pos < src.length() && isspace(src.at(pos)))
        {
            if (src.at(pos) == '\n')
            {
                line++;
                column = 1;
            }
            else
                column++;
            pos++;
        }
    }

    Token read_identifier(Token last_tok)
    {
        std::string tok_value;
        TokenType tok_type = TokenType::IDENTIFIER;
        int start_column = column;

        while (pos < src.length() && isalnum(src.at(pos)))
        {
            tok_value += src.at(pos);
            pos++;
            column++;
        }

        for (const char* keyword : Keywords)
        {
            if (tok_value == keyword)
                tok_type = TokenType::KEYWORD;
        }

        if (last_tok.type == TokenType::COLON)
            tok_type = TokenType::TYPE;

        return {tok_type, tok_value, line, start_column};
    }

    Token read_number(Token last_tok)
    {
        std::string tok_value;
        TokenType tok_type = TokenType::INT_LIT;
        int start_column = column;

        while (pos < src.length() && (isdigit(src.at(pos)) || src.at(pos) == '.'))
        {
            if (src.at(pos) == '.')
            {
                if (tok_type == TokenType::INT_LIT) 
                    tok_type = TokenType::FLOAT_LIT;
                else 
                    tok_type = TokenType::ERROR;
            }

            tok_value += src.at(pos);
            pos++;
            column++;
        }

        return {tok_type, tok_value, line, start_column};
    }

    Token create_token(TokenType type, const std::string &value)
    {
        Token token = {type, value, line, column};
        pos++;
        column++;
        return token;
    }
};