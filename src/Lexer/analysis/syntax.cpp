#include "syntax.h"
#include "highlighter.h" // Have to do this because ld is fucking retarded

#define REPORT_ERROR(message) \
    { \
        SourceLineHighlighter::token_error( \
            container, pos, message, \
            SourceLineHighlighter::Severity::ERROR); \
        failed = true; \
    } while (0)

#define EXPECT_TOKEN(expected_type) \
    do { \
        if (peek().type == expected_type) { \
            consume(); \
        } else { \
            REPORT_ERROR(std::format( \
                "Unexpected token '{}', Expected type {}", \
                peek().value, magic_enum::enum_name(expected_type))); \
            return; \
        } \
    } while (0)

using namespace via::Tokenization;
using namespace SyntaxAnalysis;

inline std::string get_value_type(Token tok)
{
    switch (tok.type)
    {
    case TokenType::LIT_INT:
    case TokenType::LIT_FLOAT:
        return "Number";
    case TokenType::LIT_BOOL:
        return "Bool";
    case TokenType::LIT_CHAR:
    case TokenType::LIT_STRING:
        return "String";
    default:
        return "any";
    }
}

Token SyntaxAnalyzer::peek(size_t ahead)
{
    if (pos + ahead >= container.tokens.size())
    {
        return Token(TokenType::EOF_, "", 0, 0);  // Return a special token if out of bounds
    }
    return container.tokens.at(pos + ahead);
}

Token SyntaxAnalyzer::consume(size_t ahead)
{
    pos += ahead;
    if (pos >= container.tokens.size())
    {
        return Token(TokenType::EOF_, "", 0, 0);  // Return a special token if out of bounds
    }
    return container.tokens.at(pos);
}

bool SyntaxAnalyzer::is_valid_expression()
{
    if (!is_valid_term())
    {
        return false;
    }

    while (peek().is_operator())
    {
        consume();  // Consume the operator
        if (!is_valid_term())
        {
            REPORT_ERROR("Invalid right-hand side of binary expression");
            return false;
        }
    }
    return true;
}

bool SyntaxAnalyzer::is_valid_type()
{
    Token next = peek();

    if (next.type != TokenType::IDENTIFIER)
    {
        return false;
    }

    consume();

    if (peek().type == TokenType::OP_LT)
    {
        consume();

        bool expecting_type = true;
        bool expecting_comma = false;

        while (peek().type != TokenType::OP_GT)
        {
            if (peek().type == TokenType::EOF_)
            {
                return false;
            }

            if (expecting_type)
            {
                if (!is_valid_type())
                {
                    return false;
                }

                expecting_type = false;
                expecting_comma = true;

                continue;
            }

            if (expecting_comma)
            {
                if (peek().type != TokenType::COMMA)
                {
                    return false;
                }

                expecting_type = true;
                expecting_comma = false;

                consume();
            }
        }
        
        if (peek().type != TokenType::OP_GT)
        {
            return false;
        }

        consume();
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
            REPORT_ERROR("Invalid expression inside parentheses");
            return false;
        }
        if (peek().type == TokenType::PAREN_CLOSE)
        {
            consume();  // Consume ')'
            return true;
        }
        REPORT_ERROR("Expected closing parenthesis");
    }
    return false;
}

void SyntaxAnalyzer::check_fun_call()
{
    consume();  // Consume IDENTIFIER
    consume();  // Consume PAREN_OPEN

    bool expecting_arg = true;
    bool expecting_comma = false;

    while (peek().type != TokenType::PAREN_CLOSE)
    {
        if (expecting_arg)
        {
            check_argument();

            expecting_arg = false;
            expecting_comma = true;

            continue;
        }

        if (expecting_comma)
        {
            EXPECT_TOKEN(TokenType::COMMA);

            expecting_arg = true;
            expecting_comma = false;
        }
    }

    if (peek().type == TokenType::COMMA)
    {
        REPORT_ERROR("Function call arguments closed with ','");
    }

    EXPECT_TOKEN(TokenType::PAREN_CLOSE);  // Ensure we have a closing parenthesis
}

void SyntaxAnalyzer::check_argument()
{
    if (peek().is_literal() || peek().type == TokenType::IDENTIFIER)
    {
        consume();
    }
    else if (!is_valid_expression())
    {
        REPORT_ERROR("Invalid argument in function call; expected identifier, literal, or expression");
    }
}

void SyntaxAnalyzer::check_ident_token()
{
    if (peek().type == TokenType::IDENTIFIER)
    {
        if (peek(1).type == TokenType::PAREN_OPEN)
        {
            check_fun_call();
        }
        else if (peek(1).type == TokenType::DOT)
        {
            consume();  // Consume IDENTIFIER
            consume();  // Consume DOT
            EXPECT_TOKEN(TokenType::IDENTIFIER);  // Expect another IDENTIFIER after DOT
        }
        else
        {
            REPORT_ERROR(std::format("Incomplete statement '{}', expected function call, index or assignment", peek().value));
        }
    }
}

void SyntaxAnalyzer::check_spec_char()
{
    Token current = peek();
    switch (current.type)
    {
    case TokenType::PAREN_OPEN:
    case TokenType::BRACE_OPEN:
        // Handle scope openings and grouped statements
        break;
    default:
        if (is_special_character(current.type))
        {
            REPORT_ERROR(std::format("Unexpected token '{}' expected statement or term", current.value));
        }
        break;
    }
}

void SyntaxAnalyzer::check_invalid_token()
{
    if (peek().type == TokenType::UNKNOWN)
    {
        REPORT_ERROR(std::format("Invalid token '{}'", peek().value));
        failed = true;
        return;
    }
}

bool SyntaxAnalyzer::is_special_character(TokenType type)
{
    return type == TokenType::PAREN_CLOSE || type == TokenType::BRACE_CLOSE || type == TokenType::BRACKET_OPEN ||
        type == TokenType::BRACKET_CLOSE || type == TokenType::AMPERSAND || type == TokenType::AT ||
        type == TokenType::BACKTICK || type == TokenType::COLON || type == TokenType::COMMA ||
        type == TokenType::DOLLAR || type == TokenType::DOT || type == TokenType::DOUBLE_QUOTE ||
        type == TokenType::PIPE || type == TokenType::SEMICOLON || type == TokenType::TILDE ||
        type == TokenType::OP_ADD || type == TokenType::OP_DEC || type == TokenType::OP_DIV ||
        type == TokenType::OP_EQ || type == TokenType::OP_EXP || type == TokenType::OP_GEQ ||
        type == TokenType::OP_GT || type == TokenType::OP_INC || type == TokenType::OP_LEQ ||
        type == TokenType::OP_LT || type == TokenType::OP_MOD || type == TokenType::OP_MUL ||
        type == TokenType::OP_NEQ || type == TokenType::OP_SUB;
}

void SyntaxAnalyzer::check_decl()
{
    if (peek().type == TokenType::KW_LOCAL || peek().type == TokenType::KW_GLOBAL || peek().type == TokenType::KW_PROPERTY)
    {
        bool is_global = peek().type == TokenType::KW_GLOBAL;
        bool is_prop = peek().type == TokenType::KW_PROPERTY;

        consume();

        if (peek().type == TokenType::KW_CONST)
        {
            if (is_global)
            {
                SourceLineHighlighter::token_error(
                    container,
                    pos,
                    "Redundant usage of 'const'; global declarations are implicitly constant",
                    SourceLineHighlighter::Severity::WARNING
                );
            }
            consume();
        }

        EXPECT_TOKEN(TokenType::IDENTIFIER);  // Expect an identifier

        if (peek().type == TokenType::COLON)
        {
            consume();

            if (!is_valid_type())
            {
                REPORT_ERROR("Expected valid type for declaration");
            }
        }
        else
        {
            if (is_prop)
            {
                REPORT_ERROR("Property declarations require explicit type declaration");
                return;
            }

            SourceLineHighlighter::token_error(
                container,
                pos - 1,
                std::format("Type not explicitly specified for variable '{}'; automatically deduced type '{}'",
                    container.tokens.at(pos - 1).value, get_value_type(peek(1))),
                SourceLineHighlighter::Severity::INFO
            );
        }

        EXPECT_TOKEN(TokenType::OP_ASGN);     // Expect '='

        if (!is_valid_expression())
        {
            REPORT_ERROR("Expected valid expression (rvalue) for declaration");
            return;
        }
    }
}

void SyntaxAnalyzer::check_ret()
{
    if (peek().type == TokenType::KW_RETURN)
    {
        consume();
        if (peek().type == TokenType::BRACE_CLOSE)
        {
            return;
        }
        if (!is_valid_expression())
        {
            REPORT_ERROR("Expected valid expression for return statement");
        }
    }
}

void SyntaxAnalyzer::check_scope()
{
    EXPECT_TOKEN(TokenType::BRACE_OPEN);

    while (peek().type != TokenType::BRACE_CLOSE)
    {
        auto prev_pos = pos;

        if (peek().type == TokenType::EOF_)
        {
            break;
        }

        match();

        if (prev_pos == pos)
        {
            consume(); // Ensure progress
        }
    }
    
    EXPECT_TOKEN(TokenType::BRACE_CLOSE);
}

void SyntaxAnalyzer::check_func()
{
    if (peek().type == TokenType::KW_FUNC)
    {
        consume();

        if (peek().type == TokenType::KW_CONST)
        {
            consume();
        }

        EXPECT_TOKEN(TokenType::IDENTIFIER);
        EXPECT_TOKEN(TokenType::PAREN_OPEN);

        bool expecting_arg = true;
        bool expecting_comma = false;

        if (peek().type != TokenType::PAREN_CLOSE)
        {
            while (peek().type != TokenType::PAREN_CLOSE)
            {
                // TODO: add EOF check here

                if (expecting_arg)
                {
                    EXPECT_TOKEN(TokenType::IDENTIFIER);

                    if (peek().type == TokenType::COLON)
                    {
                        consume();

                        if (!is_valid_type())
                        {
                            REPORT_ERROR("Expected valid type for explicit type declaration for function parameter");
                            break;
                        }
                    }

                    expecting_arg = false;
                    expecting_comma = true;

                    continue;
                }

                if (expecting_comma)
                {
                    EXPECT_TOKEN(TokenType::COMMA);

                    expecting_arg = true;
                    expecting_comma = false;
                }
            }

            if (container.tokens.at(pos - 1).type == TokenType::COMMA)
            {
                REPORT_ERROR("Function arguments closed with ','");
                return;
            }
        }

        EXPECT_TOKEN(TokenType::PAREN_CLOSE);

        check_scope();
    }
}

void SyntaxAnalyzer::check_structure()
{
    if (peek().type == TokenType::KW_STRUCT || peek().type == TokenType::KW_NAMESPACE)
    {
        consume();

        EXPECT_TOKEN(TokenType::IDENTIFIER);
        EXPECT_TOKEN(TokenType::BRACE_OPEN);

        while (peek().type != TokenType::BRACE_CLOSE)
        {
            auto prev_pos = pos;

            if (peek().type == TokenType::EOF_)
            {
                break;
            }

            if (peek().type == TokenType::KW_FUNC || peek().type == TokenType::KW_PROPERTY)
            {
                check_func();
                check_decl();

                if (prev_pos == pos)
                {
                    // If this runs, it means that something went wrong above
                    // So, there's no point of throwing another error
                    consume(); // Ensure progress
                }

                continue;
            }

            REPORT_ERROR("Expected function or property declaration inside declaration-only structure");
        }

        EXPECT_TOKEN(TokenType::BRACE_CLOSE);
    }
}

void SyntaxAnalyzer::match()
{
    check_invalid_token();
    check_spec_char();
    check_ident_token();
    check_decl();
    check_ret();
    check_func();
    check_structure();
}

bool SyntaxAnalyzer::analyze()
{
    while (pos < container.tokens.size())
    {
        auto prev_pos = pos;  // Track position to detect non-consuming `match` calls
        match();

        // Ensure progress is made
        if (pos == prev_pos)
        {
            consume();  // Skip unprocessed tokens to prevent infinite loop
        }
    }

    return failed;
}
