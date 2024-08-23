// implement these data structures:

// - scopes (eg. { code... }, needs to be parsed)
// - expressions (eg. 5 + 7)
// - identifiers (eg. local x = 0)

#pragma once

#include <string>
#include <vector>
#include <optional>

#include "lexer.hpp"
#include "variable.hpp"
#include "scope.hpp"
#include "expression.hpp"

class Parser
{
public:

    int i = 0;

    Token token;
    std::vector<Token> tokens;

    Parser(std::vector<Token> toks)
    {
        tokens = toks;
    }

    void consume(int amount = 1)
    {
        i += amount;
    }
    
    Token peek(std::optional<int> ahead = 1)
    {
        if (ahead.has_value())
            return tokens[i + ahead.value()];
        else
            return tokens[i + 1];
    }

    std::vector<Variable> get_arguments()
    {
        std::vector<Variable> nullscope = {};

        if (tokens[i + 1].type != TokenType::L_PAR)
        {
            Console::error("parser::get_arguments failed to collect arguments:\n  arguments expected");
            return nullscope;
        }

        std::vector<Variable> arguments;
        int argc = 0;
        bool expecting_arg = true;
        int j = i + 2;

        while (j < tokens.size())
        {
            Token current_token = tokens[j];

            if (current_token.type == TokenType::R_PAR) break;
            if (current_token.type == TokenType::IDENTIFIER)
            {
                if (!expecting_arg)
                {
                    Console::error("parser::get_arguments failed to collect arguments:\n  unexpected identifier");
                    return nullscope;
                }

                if (tokens[j + 1].type == TokenType::COLON)
                {
                    if (tokens[j + 2].type == TokenType::TYPE)
                    {
                        Type vType = tokens[j + 2].value;
                        arguments.push_back(
                            Variable(false, true, vType, NULL, current_token.value)
                        );
                        j += 2;
                    }
                    else
                    {
                        Console::error("failed to parse argument:\n  malformed type declaration");
                        return nullscope;
                    }
                }
                else
                {
                    Console::error("parser::get_arguments failed to collect arguments:\n  colon expected after identifier");
                    return nullscope;
                }

                expecting_arg = false; // After a type declaration, expect either a comma or a closing parenthesis
            }
            else if (current_token.type == TokenType::COMMA)
            {
                if (expecting_arg)
                {
                    Console::error("parser::get_arguments failed to collect arguments:\n  unexpected comma");
                    return nullscope;
                }
                expecting_arg = true; // After a comma, expect a new argument
            }
            else
            {
                Console::error("parser::get_arguments failed to collect arguments:\n  unexpected token: " + token_to_string(current_token.type));
                return nullscope;
            }

            j++;
        }

        if (tokens[j].type != TokenType::R_PAR)
        {
            Console::error("parser::get_arguments failed to collect arguments:\n  closing parenthesis expected");
            return nullscope;
        }

        return arguments;
    }

    int evaluate(std::vector<Token> closure)
    {
        int result;

        Token left = closure.at(1);
        Token right = closure.at(closure.size());

        std::cout << "left: " << token_to_string(left.type) << std::endl;
        std::cout << "right: " << token_to_string(right.type) << std::endl;
    }

    void parse()
    {
        Token token = tokens[i]; // Initialize the token

        for (; i < tokens.size();)
        {
            token = tokens[i];

            switch (token.type)
            {
            case TokenType::KEYWORD:
                if (token.value == "local" || token.value == "global")
                {
                    bool is_var_declr =
                        (peek().type == TokenType::IDENTIFIER) &&
                        (peek(2).type == TokenType::EQUALS) &&
                        is_literal(peek(3).type);

                    bool is_func_declr =
                        (peek().value == "function") &&
                        (peek(2).type == TokenType::IDENTIFIER);

                    if (is_var_declr)
                    {
                        consume(3);
                        std::cout << "var declared" << std::endl;
                    }
                    else if (is_func_declr)
                    {
                        consume(2);
                        std::cout << "func declared" << std::endl;
                    }
                }
                break;

                // Handle other cases as needed
            case TokenType::INT_LIT:
                std::vector<Token> closure;

                for (int j = i; j < tokens.size();)
                {

                }
            }

            consume();
        }

    }
};