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
    Lexer(const std::string& src)
        : src(src)
        , pos(-1)
        , line(1)
        , column(1) {}

    std::vector<Token> tokenize()
    {
        std::vector<Token> tokens;
        Token last_token = create_token(TokenType::START, "<sof>");
        Token tok = last_token;

        while (tok.type != TokenType::END)
        {
            tok = get_next_token(last_token);
            tokens.push_back(tok);
            last_token = tok;
        }

        return tokens;
    }

    void print_tokens(const std::vector<Token>& toks) const
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
            {
                column++;
            }
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
            consume();
        }

        if (is_keyword(tok_value))
        {
            tok_type = TokenType::KEYWORD;
        }

        if (last_tok.type == TokenType::COLON)
            tok_type = TokenType::TYPE;

        if (tok_value == "false" || tok_value == "true")
            tok_type = TokenType::BOOL_LIT;

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
                    tok_type = TokenType::ERROR; // Multiple dots detected
            }

            tok_value += src.at(pos);
            consume();
        }

        return { tok_type, tok_value, line, start_column };
    }

    Token read_string()
    {
        consume();  // Skip opening "

        std::string str;
        int start_column = column;

        while (pos < src.length() && src.at(pos) != '"')
        {
            str += consume();
        }

        if (pos >= src.length() || src.at(pos) != '"')
        {
            return { TokenType::ERROR, str, line, start_column };
        }

        consume();  // Skip closing "
        return { TokenType::STRING_LIT, str, line, start_column };
    }

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
        case '+': return create_token(TokenType::PLUS, "+");
        case '-': return create_token(TokenType::MINUS, "-");
        case '=':
            if (peek() == '=')
            {
                consume();  // Move past second '='
                return create_token(TokenType::EQU, "==");
            }
            return create_token(TokenType::ASSIGN, "=");
        case '{': return create_token(TokenType::LCURLY, "{");
        case '}': return create_token(TokenType::RCURLY, "}");
        case '[': return create_token(TokenType::LSQU, "[");
        case ']': return create_token(TokenType::RSQU, "]");
        case '(': return create_token(TokenType::LPAR, "(");
        case ')': return create_token(TokenType::RPAR, ")");
        case ',': return create_token(TokenType::COMMA, ",");
        case ':': return create_token(TokenType::COLON, ":");
        case '*': return create_token(TokenType::ASTER, "*");
        case '/': return create_token(TokenType::FSLASH, "/");
        case '!': return create_token(TokenType::EXCLAM, "!");
        case ';': return create_token(TokenType::SEMICOL, ";");
        case '"': return read_string();
        default:
            return create_token(TokenType::ERROR, std::string(1, current));
        }
    }

    char peek() const
    {
        if (pos + 1 >= src.length())
            return '\0';
        return src.at(pos + 1);
    }

    char consume()
    {
        char current = src.at(pos);
        pos++;
        column++;
        return current;
    }

    bool is_keyword(const std::string& word) const
    {
        for (const auto& keyword : Keywords)
        {
            if (word == keyword)
                return true;
        }
        return false;
    }
};
