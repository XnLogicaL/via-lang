#include "common.h"
#include "syntax.h"

#include "../util/highlighter.hpp"

using namespace via::Tokenization;
using namespace SyntaxAnalysis;

Token SyntaxAnalyzer::peek(size_t ahead)
{
    if (pos + ahead >= container.tokens.size())
    {
        return Token(TokenType::EOF_);  // Return a special token if out of bounds
    }

    return container.tokens.at(pos + ahead);
}

Token SyntaxAnalyzer::consume(size_t ahead)
{
    pos += ahead;

    if (pos >= container.tokens.size())
    {
        return Token(TokenType::EOF_);  // Return a special token if out of bounds
    }

    return container.tokens.at(pos);
}

bool SyntaxAnalyzer::is_valid_expression()
{
    // Start parsing a term (this could be a literal, identifier, or grouped expression)
    if (!is_valid_term())
    {
        return false;
    }

    // Check for operators to handle binary expressions
    while (peek().is_operator())
    {
        consume();  // Consume the operator

        if (!is_valid_term())  // Parse the right-hand side of the expression
        {
            SourceLineHighlighter::token_error(
                container,
                pos,
                "Invalid right-hand side of binary expression",
                SourceLineHighlighter::Severity::ERROR
            );

            failed = true;

            return false;
        }
    }

    return true;
}

bool SyntaxAnalyzer::is_valid_term()
{
    Token next = peek();

    if (next.is_literal() || next.type == TokenType::IDENTIFIER)
    {
        consume();
        return true;
    }
    else if (next.type == TokenType::PAREN_OPEN)
    {
        consume();  // Consume '('
        if (!is_valid_expression())  // Recursively parse the expression inside parentheses
        {
            SourceLineHighlighter::token_error(
                container,
                pos,
                "Invalid expression inside parentheses",
                SourceLineHighlighter::Severity::ERROR
            );
            failed = true;
            return false;
        }
        if (peek().type == TokenType::PAREN_CLOSE)
        {
            consume();  // Consume ')'
            return true;
        }
        else
        {
            SourceLineHighlighter::token_error(
                container,
                pos,
                "Expected closing parenthesis",
                SourceLineHighlighter::Severity::ERROR
            );
            failed = true;
            return false;
        }
    }
    return false;  // If none of the conditions matched, return false
}

void SyntaxAnalyzer::expect(std::vector<TokenType> expected_types)
{
    size_t off = 0;
    for (const auto& tp : expected_types)
    {
        if (peek(off).type == tp)
        {
            off++;
        }
        else
        {
            SourceLineHighlighter::token_error(
                container,
                pos + off,
                "Unexpected token type; Expected: " + std::string(magic_enum::enum_name(tp)),
                SourceLineHighlighter::Severity::ERROR
            );
            failed = true;
            break;
        }
    }
}

void SyntaxAnalyzer::check_invalid_token()
{
    auto current = peek();

    if (current.type == TokenType::UNKNOWN)
    {
        SourceLineHighlighter::token_error(
            container,
            pos,
            "Invalid token '" + current.value + "'",
            SourceLineHighlighter::Severity::ERROR
        );
        failed = true;
    }
}

void SyntaxAnalyzer::check_fun_call()
{
    // Assuming current token is IDENTIFIER and the next is PAREN_OPEN
    consume();  // Consume IDENTIFIER
    consume();  // Consume PAREN_OPEN

    while (peek().type != TokenType::PAREN_CLOSE && !failed)
    {
        check_argument();
        if (peek().type == TokenType::COMMA)
        {
            consume();  // Consume COMMA and check next argument
        }
        else
        {
            break;
        }
    }

    if (peek().type != TokenType::PAREN_CLOSE)
    {
        SourceLineHighlighter::token_error(
            container,
            pos,
            "Expected closing parenthesis for function call",
            SourceLineHighlighter::Severity::ERROR
        );
        failed = true;
    }
    else
    {
        consume();  // Consume PAREN_CLOSE
    }
}

void SyntaxAnalyzer::check_argument()
{
    Token next = peek();

    if (next.type == TokenType::IDENTIFIER || next.is_literal())
    {
        consume();
    }
    else
    {
        // Attempt to parse a full expression as the argument
        if (!is_valid_expression())
        {
            SourceLineHighlighter::token_error(
                container,
                pos,
                "Invalid argument in function call; expected identifier, literal, or expression",
                SourceLineHighlighter::Severity::ERROR
            );

            failed = true;
        }
    }
}

void SyntaxAnalyzer::check_ident_token()
{
    auto current = peek();

    if (current.type == TokenType::IDENTIFIER)
    {
        if (peek(1).type == TokenType::PAREN_OPEN)
        {
            check_fun_call();
        }
        else if (peek(1).type == TokenType::DOT)
        {
            consume();  // Consume IDENTIFIER
            consume();  // Consume DOT

            if (peek().type == TokenType::IDENTIFIER)
            {
                consume();  // Consume identifier after DOT
            }
            else
            {
                SourceLineHighlighter::token_error(
                    container,
                    pos,
                    "Expected identifier after dot for indexing",
                    SourceLineHighlighter::Severity::ERROR
                );

                failed = true;
            }
        }
        else
        {
            SourceLineHighlighter::token_error(
                container,
                pos,
                std::format("Incomplete statement '{}' (IDENTIFIER), expected function call, index or assignment", current.value),
                SourceLineHighlighter::Severity::ERROR
            );

            failed = true;
        }
    }
}

void SyntaxAnalyzer::check_spec_char()
{
    auto current = peek();

    switch (current.type)
    {
    case TokenType::PAREN_OPEN:
    case TokenType::BRACE_OPEN:
        // Handle scope openings and grouped statements
        break;
    case TokenType::PAREN_CLOSE:
    case TokenType::BRACE_CLOSE:
    case TokenType::BRACKET_OPEN:
    case TokenType::BRACKET_CLOSE:
    case TokenType::AMPERSAND:
    case TokenType::AT:
    case TokenType::BACKTICK:
    case TokenType::COLON:
    case TokenType::COMMA:
    case TokenType::DOLLAR:
    case TokenType::DOT:
    case TokenType::DOUBLE_QUOTE:
    case TokenType::PIPE:
    case TokenType::SEMICOLON:
    case TokenType::TILDE:
    case TokenType::OP_ADD:
    case TokenType::OP_DEC:
    case TokenType::OP_DIV:
    case TokenType::OP_EQ:
    case TokenType::OP_EXP:
    case TokenType::OP_GEQ:
    case TokenType::OP_GT:
    case TokenType::OP_INC:
    case TokenType::OP_LEQ:
    case TokenType::OP_LT:
    case TokenType::OP_MOD:
    case TokenType::OP_MUL:
    case TokenType::OP_NEQ:
    case TokenType::OP_SUB:
        // These tokens should not be encountered on non-illformed programs
        SourceLineHighlighter::token_error(
            container,
            pos,
            std::format("Unexpected token '{}' expected statement or term, found special character", current.value),
            SourceLineHighlighter::Severity::ERROR
        );

        failed = true;
        break;
    }
}

void SyntaxAnalyzer::check_literal()
{
    auto current = peek();

    if (current.is_literal())
    {
        SourceLineHighlighter::token_error(
            container,
            pos,
            std::format("Unexpected token '{}' expected statement, found literal expression", current.value),
            SourceLineHighlighter::Severity::ERROR
        );

        failed = true;
    }
}

void SyntaxAnalyzer::match()
{
    check_invalid_token();
    check_spec_char();
    check_literal();
    check_ident_token();
}

bool SyntaxAnalyzer::analyze()
{
    while (pos < container.tokens.size())
    {
        size_t prev_pos = pos;  // Track position to detect non-consuming `match` calls

        match();

        // Ensure progress is made
        if (pos == prev_pos)
        {
            consume();  // Skip unprocessed tokens to prevent infinite loop
        }
    }

    return failed;
}

bool SyntaxAnalysis::analyze(viaSourceContainer& container)
{
    auto analyzer = new SyntaxAnalyzer(container);
    auto fail = analyzer->analyze();

    delete analyzer;

    return fail;
}
