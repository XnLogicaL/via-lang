/* This file is a part of the via programming language at
 * https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "container.h"
#include "highlighter.h"
#include "token.h"

namespace via::Tokenization
{

class SyntaxAnalyzer
{
public:
    // Constructs a SyntaxAnalyzer instance with the provided source container
    explicit SyntaxAnalyzer(viaSourceContainer &container)
        : container(container)
    {
    }

    // Analyzes the tokens in the source container for syntactical correctness.
    // Returns true if the analysis is successful, otherwise false.
    bool analyze();

private:
    Emitter emitter;
    viaSourceContainer &container;
    size_t pos = 0;
    bool failed = false;

private:
    // Returns if the internal `pos` variable is in bounds
    constexpr bool in_bounds() noexcept;

    // Returns a view of the token at a specific offset from the current position
    // Does not consume the token, allowing lookahead functionality
    Token peek(size_t ahead = 0) noexcept;

    // Consumes the specified number of tokens starting from the current position
    // Returns the token at the original starting position before consumption
    Token consume(size_t ahead = 1) noexcept;

    // Checks whether the current token, along with subsequent tokens, can form a
    // valid expression Examples: arithmetic operations, function calls, or
    // parenthesized expressions
    bool is_valid_expression();
    // Checks whether the current token, and potentially the tokens following it,
    // can form a valid term A term might include literals, identifiers, or other
    // self-contained syntactical units
    bool is_valid_term();
    // Determines whether the current token sequence matches a valid type
    // declaration or reference Includes primitive types, user-defined types, or
    // type aliases
    bool is_valid_type();
    // Verifies if the current tokens form a valid function call expression
    // Ensures proper syntax for function name and arguments
    void check_fun_call();
    // Validates the syntax and structure of function call arguments
    // Ensures they are valid expressions and adhere to argument-passing rules
    void check_argument();
    // Confirms whether the current token is a valid identifier
    // Identifiers are typically variable names, function names, or other symbolic
    // references
    void check_ident_token();
    // Validates tokens containing special characters
    // Ensures they comply with the allowed syntax for special symbols in
    // identifiers or expressions
    void check_spec_char();
    // Checks whether a token type is classified as a special character
    // Useful for validating symbols in custom identifiers or operators
    bool is_special_character(TokenType type);
    // Verifies the correctness of a variable or constant declaration
    // Checks the syntax for type, name, and optional initialization
    void check_decl();
    // Ensures the current token sequence represents a valid return statement
    // Includes validation for optional return expressions
    void check_ret();
    // Analyzes the structure of a scope block, such as function bodies or
    // conditional blocks Ensures proper use of braces and valid statements within
    // the block
    void check_scope();
    // Confirms whether the current tokens represent a valid function definition
    // Validates the syntax for function name, parameters, and body
    void check_func();
    // Analyzes the current tokens to ensure they represent a valid struct
    // definition Verifies member declarations, keywords, and proper block
    // structure
    void check_structure();
    // Matches the current token with an expected pattern or sequence
    // Advances the position if the match is successful; otherwise, marks an error
    void match();
    // Checks for invalid or unexpected tokens in the source code
    // Ensures proper error handling for syntax violations
    void check_invalid_token();
};

} // namespace via::Tokenization
