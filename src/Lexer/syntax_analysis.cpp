/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "syntax_analysis.h"

// Macro for reporting a syntax error
// Yes, this is yet another abstraction over io
#define REPORT_ERROR(message) \
    { \
        emitter.out(container, pos, message, Emitter::Severity::ERROR); \
        failed = true; \
    } \
    while (0)

// Asserts <this-token-type> == <expected-type>
// Reports a generic syntax error and returns if the assertion fails
// Consumes the current token if not
#define EXPECT_TOKEN(expected_type) \
    do \
    { \
        if (peek().type == expected_type) \
            consume(); \
        else \
        { \
            REPORT_ERROR(std::format("Unexpected token '{}', Expected type {}", peek().value, magic_enum::enum_name(expected_type))); \
            return; \
        } \
    } while (0)

namespace via::Tokenization
{

// Returns the primitive value type of a literal
// Internal use only
const std::string get_value_type(const Token &tok) noexcept
{
    switch (tok.type)
    {
    case TokenType::LIT_INT:
    case TokenType::LIT_FLOAT:
        return "viaNumber";
    case TokenType::LIT_BOOL:
        return "Bool";
    case TokenType::LIT_STRING:
        return "String";
    case TokenType::LIT_NIL:
        return "Nil";
    default:
        return "Any";
    }
}

// Documented in syntax.h
constexpr bool SyntaxAnalyzer::in_bounds() noexcept
{
    // Since `pos` is unsigned, we don't need to check if it's larger than or equal to 0
    return pos < container.tokens.size();
}

// Returns the token that's <ahead> ahead of the current position
Token SyntaxAnalyzer::peek(size_t ahead) noexcept // This should never throw, in theory
{
    // Bound check
    if (pos + ahead >= container.tokens.size())
    {
        // Return a special token if out of bounds
        return Token(TokenType::EOF_, "", 0, 0);
    }

    return container.tokens.at(pos + ahead);
}

// Consumes all the tokens until <pos> + <ahead>
Token SyntaxAnalyzer::consume(size_t ahead) noexcept
{
    // Increment position by <ahead>
    pos += ahead;

    // Bound check before return
    if (!in_bounds())
    {
        // Return a special token if out of bounds
        return Token(TokenType::EOF_, "", 0, 0);
    }

    return container.tokens.at(pos);
}

// Returns if the a sequence of tokens starting from the current one can form a syntactically correct expression
// Used for validating expressions
bool SyntaxAnalyzer::is_valid_expression()
{
    // Check if the lhs (left-hand side) of the expression is a valid term
    if (!is_valid_term())
    {
        // Returns false (illformed expression)
        return false;
    }

    // Loop until the current token is not an operator
    // This check is mainly for binary operations
    while (peek().is_operator())
    {
        // Consume the operator
        consume();

        // Check if the now rhs (right-hand side)
        if (!is_valid_term())
        {
            // Report the syntax error and return false (illformed expression)
            REPORT_ERROR("Invalid right-hand side of binary expression");
            return false;
        }
    }

    // Return true if everything goes correctly
    return true;
}

// Returns if the a sequence of tokens starting from the current one can form a syntactically correct type
// Used for validating types
bool SyntaxAnalyzer::is_valid_type()
{
    // Save the initial next token for convenience
    Token next = peek();

    // Since types can only "start" with an identifier, this is the first thing we check
    // eg. Generic<T>
    if (next.type != TokenType::IDENTIFIER)
    {
        // Return false (illformed type)
        return false;
    }

    // Consume the type body (identifier)
    consume();

    // Check if this is a generic type
    if (peek().type == TokenType::OP_LT)
    {
        // If so, consume the opening
        consume();

        // Initialize expectations
        // Technically there doesn't need to be 2 of these since they will never be equal
        bool expecting_type = true;
        bool expecting_comma = false;

        // Loop until we hit the closing
        while (in_bounds() && peek().type != TokenType::OP_GT)
        {
            // This is kinda redundant but we can keep it either way
            if (peek().type == TokenType::EOF_)
            {
                // Return false (illformed type)
                return false;
            }

            // Check if it's expecting a type (generic, literal, ...)
            // eg. Generic<Type>
            //             ^~~~
            if (expecting_type)
            {
                // Recursively check if this type is valid
                if (!is_valid_type())
                {
                    // Return false (illformed type)
                    return false;
                }

                // Debounce the expectations
                expecting_type = false;
                expecting_comma = true;

                // Skip the base-case
                continue;
            }

            // Check if it's expecting a comma
            if (expecting_comma)
            {
                // If a comma is expected but not found, return false
                if (peek().type != TokenType::COMMA)
                {
                    // Return false (illformed type)
                    return false;
                }

                // Debounce expectations
                expecting_type = true;
                expecting_comma = false;

                // Consume comma
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

// Returns if the a sequence of tokens starting from the current one can form a syntactically correct term
// Used for validating terms
bool SyntaxAnalyzer::is_valid_term()
{
    // Store the current token for use later
    Token next = peek();

    // Check if the term is a literal or identifier, if so return true
    if (next.is_literal() || next.type == TokenType::IDENTIFIER)
    {
        // Consume the term and return true
        consume();
        return true;
    }
    // Check if the term is a grouped expression
    else if (next.type == TokenType::PAREN_OPEN)
    {
        // Consume '('
        consume();

        // Recursively parse the expression inside parentheses
        if (!is_valid_expression())
        {
            // Return false an report error
            REPORT_ERROR("Invalid expression inside parentheses");
            return false;
        }
        if (peek().type == TokenType::PAREN_CLOSE)
        {
            // Consume ')' and return true
            consume();
            return true;
        }

        // Report missing closing parantheses
        REPORT_ERROR("Expected closing parenthesis");
    }

    // Return false (illformed term) (base case)
    return false;
}

// Returns if the a sequence of tokens starting from the current one can form a syntactically correct call statement/expression
void SyntaxAnalyzer::check_fun_call()
{
    consume(); // Consume call identifier
    consume(); // Consume '('

    // Setup expectations
    bool expecting_arg = true;
    bool expecting_comma = false;

    // Loop through arguments until it finds ')' or goes out of bounds
    while (in_bounds() && peek().type != TokenType::PAREN_CLOSE)
    {
        // Check if it's expecting an argument
        if (expecting_arg)
        {
            // If so, validate argument
            check_argument();

            // Debounce expectations
            expecting_arg = false;
            expecting_comma = true;

            // Skip base-case
            continue;
        }

        // Check if it's expecting a comma for arg seperation
        if (expecting_comma)
        {
            // If so, assert the presence of a comma
            EXPECT_TOKEN(TokenType::COMMA);

            // Debounce expectations
            expecting_arg = true;
            expecting_comma = false;
        }
    }

    // Check if the parantheses were closed after a comma
    // This is invalid syntax, as defined in {root}/GRAMMAR.md
    // eg. function(argument0, argument1,)
    if (peek().type == TokenType::COMMA)
    {
        // Report error and return
        REPORT_ERROR("Function call arguments closed with ','");
        return;
    }

    // If all goes well, this is where it should end up
    // Ensure we have a closing parenthesis
    EXPECT_TOKEN(TokenType::PAREN_CLOSE);
    return;
}

// Returns if the a sequence of tokens starting from the current one can form a syntactically correct argument
// Basically just validate expression but wrapped for verbosity
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

// Returns if the a sequence of tokens starting from the current one can form a syntactically correct identifier expression/statement
// These include;
// - Index statements (calls, assignments, etc.)
// - Function calls
// - Variable assignments
void SyntaxAnalyzer::check_ident_token()
{
    // Check if the current token is an identifier
    // This is necessary to ensure the control flow skips this check if the token isn't an identifier
    if (peek().type == TokenType::IDENTIFIER)
    {
        // Check for function calls
        if (peek(1).type == TokenType::PAREN_OPEN)
        {
            check_fun_call();
        }
        // Check for index expressions/statements
        else if (peek(1).type == TokenType::DOT)
        {
            // Consume identifier
            consume();
            // Consume '.'
            consume();
            // Expect another identifier after '.'
            EXPECT_TOKEN(TokenType::IDENTIFIER);
        }
        // Check for assignements
        else if (peek(1).type == TokenType::OP_ASGN)
        {
            // Consume identifier
            consume();
            // Consume '='
            consume();
            // Check if the assigned value is a valid expression
            if (!is_valid_expression())
            {
                // Report error if it's an illformed/invalid expression
                REPORT_ERROR("Invalid expression assigned to variable");
            }
        }
        // Use else just to not use return statements everywhere
        else
        {
            // Report unknown identifier expression/statement
            REPORT_ERROR(std::format("Incomplete statement '{}', expected function call, index or assignment", peek().value));
        }
    }
}

// Checks for special characters that aren't supposed to be lying around
void SyntaxAnalyzer::check_spec_char()
{
    // Store the current token for use
    // Not really necessary
    Token current = peek();

    switch (current.type)
    {
    // Handle scope openings and grouped statements
    case TokenType::PAREN_OPEN:
        // These character(s) are allowed
        break;
    default:
        // Check if it's actually a special character
        // Which wasn't handled before
        if (is_special_character(current.type))
        {
            // If it is,
            REPORT_ERROR(std::format("Unexpected token '{}' expected statement or term", current.value));
        }
        break;
    }
}

// Check if the current token is invalid
void SyntaxAnalyzer::check_invalid_token()
{
    // Handle unknown token
    if (peek().type == TokenType::UNKNOWN)
    {
        // Report error and return
        REPORT_ERROR(std::format("Invalid token '{}'", peek().value));
        return;
    }
}

// Return if the given token type is a special character type
bool SyntaxAnalyzer::is_special_character(TokenType type)
{
    return type == TokenType::PAREN_CLOSE || type == TokenType::BRACE_CLOSE || type == TokenType::BRACKET_OPEN || type == TokenType::BRACKET_CLOSE ||
           type == TokenType::AMPERSAND || type == TokenType::AT || type == TokenType::BACKTICK || type == TokenType::COLON ||
           type == TokenType::COMMA || type == TokenType::DOLLAR || type == TokenType::DOT || type == TokenType::DOUBLE_QUOTE ||
           type == TokenType::PIPE || type == TokenType::SEMICOLON || type == TokenType::TILDE || type == TokenType::OP_ADD ||
           type == TokenType::OP_DEC || type == TokenType::OP_DIV || type == TokenType::OP_EQ || type == TokenType::OP_EXP ||
           type == TokenType::OP_GEQ || type == TokenType::OP_GT || type == TokenType::OP_INC || type == TokenType::OP_LEQ ||
           type == TokenType::OP_LT || type == TokenType::OP_MOD || type == TokenType::OP_MUL || type == TokenType::OP_NEQ ||
           type == TokenType::OP_SUB;
}

// Check if a sequence of tokens starting from pos form a valid declaration statement
void SyntaxAnalyzer::check_decl()
{
    // Handle declaration
    // Determined by the keyword
    if (peek().type == TokenType::KW_LOCAL || peek().type == TokenType::KW_GLOBAL || peek().type == TokenType::KW_PROPERTY)
    {
        // Initialize declaration fields
        bool is_global = peek().type == TokenType::KW_GLOBAL;
        bool is_prop = peek().type == TokenType::KW_PROPERTY;

        // Consume keyword
        consume();

        if (peek().type == TokenType::KW_FUNC)
        {
            check_func();
            return;
        }

        // Check for the `const` keyword
        // If present mark declaration as constant
        if (peek().type == TokenType::KW_CONST)
        {
            // Check if the constant declaration is also global
            // This is redundant because global declarations are constant by default
            // To utilize "static memory" correctly
            if (is_global)
            {
                // Emit a warning
                emitter.out(container, pos, "Redundant usage of 'const'; global declarations are implicitly constant", Emitter::Severity::WARNING);
            }

            // Consume const
            consume();
        }

        // Expect variable identifier
        EXPECT_TOKEN(TokenType::IDENTIFIER); // Expect an identifier

        // Check for type annotation
        if (peek().type == TokenType::COLON)
        {
            // Consume ':'
            consume();

            // Validate denoted type
            if (!is_valid_type())
            {
                // Report error if it's illformed
                REPORT_ERROR("Expected valid type for declaration");
            }
        }
        // Handle other cases where a type is not explicitly denoted
        else
        {
            // Check if the declaration is a property declaration
            // If so, report an error
            // This is because properties may not have values during initialization
            // Which cannot be automatically type deduced by the compiler
            // Therefore, an explicit type annotation is REQUIRED
            if (is_prop)
            {
                REPORT_ERROR("Property declarations require explicit type declaration");
                return;
            }

            // Otherwise, report an "info"
            // This is because explicit type declarations are not required in non-property declarations
            emitter.out(
                container,
                pos - 1,
                std::format(
                    "Type not explicitly specified for variable '{}'; automatically deduced type '{}'",
                    container.tokens.at(pos - 1).value,
                    get_value_type(peek(1))
                ),
                Emitter::Severity::INFO
            );
        }

        // Expect '='
        EXPECT_TOKEN(TokenType::OP_ASGN);

        // Check if the assigned expression is not illformed
        // Also if the declaration is a property declaration
        if (!is_valid_expression() && !is_prop)
        {
            REPORT_ERROR("Expected valid expression to assign to declaration");
            return;
        }
    }
}

// Check if a series of a tokens starting from pos can form a valid return statement
void SyntaxAnalyzer::check_ret()
{
    // This is necessary because of how the control flow works
    if (peek().type == TokenType::KW_RETURN)
    {
        // Consume 'return'
        consume();

        // Check if the return statement is empty
        // Return if so
        if (peek().type == TokenType::BRACE_CLOSE)
        {
            return;
        }
        // Check if the expression is illformed
        // An expression is the only valid possible thing after a return statement
        if (!is_valid_expression())
        {
            REPORT_ERROR("Expected valid expression for return statement");
        }
    }
}

// Check if a series of tokens starting from pos can form a valid scope statement
void SyntaxAnalyzer::check_scope()
{
    // We expect '{' first because this function is meant to check any type of statement
    // Therefore if we included the 'do' keyword, it would not work for function bodies, if blocks, etc.
    EXPECT_TOKEN(TokenType::BRACE_OPEN);

    // Keep checking statements until we find the closing brace
    while (in_bounds() && peek().type != TokenType::BRACE_CLOSE)
    {
        auto prev_pos = pos;

        if (peek().type == TokenType::EOF_)
        {
            break;
        }

        // Actual call for checking the current statement
        match();

        if (prev_pos == pos)
        {
            consume(); // Ensure progress
        }
    }

    EXPECT_TOKEN(TokenType::BRACE_CLOSE);
}

// Checks if a sequence of tokens starting from pos can form a valid function declaration
void SyntaxAnalyzer::check_func()
{
    // Check for the sentinel function keyword
    if (peek().type == TokenType::KW_FUNC)
    {
        // Consume 'func'
        consume();

        // Check for const type qualifier
        if (peek().type == TokenType::KW_CONST)
        {
            // Consune 'const' if present
            consume();
        }

        // Expect function identifier
        EXPECT_TOKEN(TokenType::IDENTIFIER);
        // Expect opening '(' token
        EXPECT_TOKEN(TokenType::PAREN_OPEN);

        // Initialize expectations
        bool expecting_arg = true;
        bool expecting_comma = false;

        // Check if the parameters aren't empty
        if (peek().type != TokenType::PAREN_CLOSE)
        {
            // Validate parameters until it hits the closing ')'
            while (in_bounds() && peek().type != TokenType::PAREN_CLOSE)
            {
                // Check for EOF sentinel token
                // This means that the parameters weren't properly closed
                if (peek().type == TokenType::EOF_)
                {
                    break;
                }

                // Check if it's expecting an parameter
                if (expecting_arg)
                {
                    // Expect parameter identifier
                    EXPECT_TOKEN(TokenType::IDENTIFIER);

                    // Check for type annotation
                    if (peek().type == TokenType::COLON)
                    {
                        // Consume ':'
                        consume();

                        // Validate type
                        if (!is_valid_type())
                        {
                            REPORT_ERROR("Expected valid type for explicit type declaration for function parameter");
                            break;
                        }
                    }

                    // Debounce expectations
                    expecting_arg = false;
                    expecting_comma = true;

                    // Skip base-case
                    continue;
                }

                // Check if it's expecting a comma
                if (expecting_comma)
                {
                    // Expect the said comma
                    EXPECT_TOKEN(TokenType::COMMA);

                    // Debounce expectations
                    expecting_arg = true;
                    expecting_comma = false;
                }
            }

            // Check if parameters were closed with a comma before the parantheses
            // This is forbidden, just like function calls
            if (container.tokens.at(pos - 1).type == TokenType::COMMA)
            {
                REPORT_ERROR("Function parameters closed with ','");
                return;
            }
        }

        // Expect the closing parentheses
        EXPECT_TOKEN(TokenType::PAREN_CLOSE);

        // Check function body
        check_scope();
    }
}

// Check if a sequence of tokens starting from pos can form a valid struct declaration
// This is used for both namespaces and structs
void SyntaxAnalyzer::check_structure()
{
    // Check for the sentinel keyword
    if (peek().type == TokenType::KW_STRUCT || peek().type == TokenType::KW_NAMESPACE)
    {
        // Consume said keyword
        consume();

        // Expect struct identifier
        EXPECT_TOKEN(TokenType::IDENTIFIER);
        // Expect struct block
        EXPECT_TOKEN(TokenType::BRACE_OPEN);

        // Parse statements inside struct until it hits a closing brace
        while (in_bounds() && peek().type != TokenType::BRACE_CLOSE)
        {
            // Save starting position for later use
            auto prev_pos = pos;

            // Check for unexpected EOF token
            if (peek().type == TokenType::EOF_)
            {
                break;
            }

            // Check for either function (method) or property declarations
            // These are the only allowed statements inside structs, anything else will report an error and fail subsequently
            if (peek().type == TokenType::KW_FUNC || peek().type == TokenType::KW_PROPERTY)
            {
                // Check function and property declarations
                check_func();
                check_decl();

                // Check for progress
                if (prev_pos == pos)
                {
                    // If this runs, it means that something went wrong above
                    // Therefore there's no point of throwing another error
                    consume(); // Ensure progress
                }

                continue;
            }

            // Report an error that informs the rules of structs
            REPORT_ERROR("Expected method or property declaration inside struct declaration");
        }

        // Expect the closing curly brace for the block
        EXPECT_TOKEN(TokenType::BRACE_CLOSE);
    }
}

// Function for performing every check on a sequence of tokens starting from pos
void SyntaxAnalyzer::match()
{
    // Perform checks
    check_invalid_token();
    check_spec_char();
    check_ident_token();
    check_decl();
    check_ret();
    check_func();
    check_structure();
}

// Function for performing full syntactic analysis on a source file
bool SyntaxAnalyzer::analyze()
{
    while (pos < container.tokens.size())
    {
        // Track position to detect non-consuming `match` calls
        auto prev_pos = pos;
        match();

        // Ensure progress is made
        if (pos == prev_pos)
        {
            // Skip unprocessed tokens to prevent infinite loop
            consume();
        }
    }

    return failed;
}

} // namespace via::Tokenization
