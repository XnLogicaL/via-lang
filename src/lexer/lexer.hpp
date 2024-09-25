#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <map>
#include <optional>

#include "keywords.hpp"
#include "token.hpp"
#include "tokentype.hpp"

class Lexer
{
public:
    Lexer(const std::string& src) : src(src), pos(-1), line(1), column(1) {}

    Token get_next_token(Token last_tok)
    {
        if (pos >= src.length())
            return create_token(TokenType::END, "<eof>");

        char current = src.at(pos);

        while (isspace(current))
        {
            consume_white_space();

            if (pos >= src.length())
                return create_token(TokenType::END, "<eof>");

            current = src.at(pos);
        }

        if (isalpha(current)) return read_identifier(last_tok);
        if (isdigit(current)) return read_number(last_tok);

        switch (current)
        {
        case '+': return create_token(TokenType::ADD, "+");
        case '-': return create_token(TokenType::SUB, "-");
        case '=':
            if (peek() == '=')
            {
                pos++;
                return create_token(TokenType::EQU, "==");
            }
            return create_token(TokenType::ASSIGN, "=");
        case '{': return create_token(TokenType::L_CR_BRACKET, "{");
        case '}': return create_token(TokenType::R_CR_BRACKET, "}");
        case '[': return create_token(TokenType::L_SQ_BRACKET, "[");
        case ']': return create_token(TokenType::R_SQ_BRACKET, "]");
        case '(': return create_token(TokenType::L_PAR, "(");
        case ')': return create_token(TokenType::R_PAR, ")");
        case ',': return create_token(TokenType::COMMA, ",");
        case ':': return create_token(TokenType::COLON, ":");
        case '*': return create_token(TokenType::MUL, "*");
        case '/': return create_token(TokenType::DIV, "/");
        case '!': return create_token(TokenType::EXCLAMATION, "!");
        case ';': return create_token(TokenType::SEMICOLON, ";");
        case '"': return read_string();
        default:
            return create_token(TokenType::ERROR, std::string(1, current));
        }
    }

    std::vector<Token> tokenize()
    {
        Token tok;
        std::vector<Token> tokens;
        Token last_token = create_token(TokenType::START, "<sof>");

        while (tok.type != TokenType::END)
        {
            tok = get_next_token(last_token);

            if (tok.type == TokenType::ERROR)
                break;

            tokens.push_back(tok);
            last_token = tok;
        }

        return tokens;
    }

    void print_tokens(const std::vector<Token> toks)
    {
        std::cout << std::endl;
        std::cout << "Token count: " << toks.size() << std::endl;

        for (const auto& tok : toks)
            std::cout << tok.to_string() << std::endl;

        std::cout << std::endl;
    }

    Token create_token(TokenType type, const std::string& value)
    {
        Token token = { type, value, line, column };
        pos++;
        column++;
        return token;
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

        while (pos < src.length() && (isalnum(src.at(pos)) || src.at(pos) == '_'))
        {
            tok_value += src.at(pos);
            pos++;
            column++;
        }

        for (const char* keyword : Keywords)
        {
            if (tok_value == keyword)
            {
                tok_type = TokenType::KEYWORD;
                break;
            }
        }

        if (last_tok.type == TokenType::COLON)
            tok_type = TokenType::TYPE;

        if (tok_value == "false" || tok_value == "true")
            tok_type = TokenType::BOOL_ALPHA;

        return { tok_type, tok_value, line, start_column };
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
            consume();
            column++;
        }

        return { tok_type, tok_value, line, start_column };
    }

    Token read_string()
    {
        consume();

        std::string str;

        while (true)
        {
            if (src.at(pos) == '"')
            {
                consume();
                break;
            }

            str += src.at(pos);
            consume();
        }

        return { TokenType::STRING_LIT, str, line, column };
    }

    char peek()
    {
        if (pos + 1 >= src.length())
            return '\0';
        return src.at(pos + 1);
    }

    char consume()
    {
        return src.at(pos++);
    }
};