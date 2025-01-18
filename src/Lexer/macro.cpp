/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "macro.h"
#include "preproc.h"

namespace via
{

// TODO: Add safety mechanisms such as;
// - Expansion depth limit
// - Restricted access to keywords, specifically the preprocessor keywords
void Preprocessor::expand_macro(const Macro &macro)
{
    std::vector<Token> toks = program.tokens->tokens;

    while (pos < toks.size())
    {
        const Token &tok = peek();

        // Match macro_name!( pattern
        if (tok.value.ends_with('!') && tok.value.substr(0, tok.value.length() - 1) == macro.name && pos + 1 < toks.size() &&
            peek(1).type == TokenType::PAREN_OPEN)
        {
            size_t start_pos = pos;
            consume(2); // Consume macro_name! and the opening parenthesis

            // Parse macro arguments
            std::vector<std::vector<Token>> macro_args;
            std::vector<Token> current_arg;
            size_t depth = 1;

            // Loop to parse arguments within parentheses
            while (pos < toks.size())
            {
                const Token &current = peek();

                // Handle nested parentheses
                if (current.type == TokenType::PAREN_OPEN)
                    depth++;
                else if (current.type == TokenType::PAREN_CLOSE)
                {
                    depth--;
                    if (depth == 0)
                        break; // End of arguments
                }
                else if (current.type == TokenType::COMMA && depth == 1)
                {
                    // End of current argument
                    if (!current_arg.empty())
                    {
                        macro_args.push_back(current_arg);
                        current_arg.clear();
                    }
                    consume(); // Consume the comma
                    continue;
                }

                // Add token to current argument
                if (depth > 0)
                    current_arg.push_back(current);

                // Avoid consuming past EOF
                if (pos < toks.size())
                    consume();
                else
                    break; // Prevent consuming EOF
            }

            // Check for unmatched parentheses (error handling)
            if (depth > 0)
            {
                PREPROCESSOR_ERROR("Unmatched parentheses in macro invocation");
                return;
            }

            if (!current_arg.empty())
                macro_args.push_back(current_arg);

            // Check for argument count mismatch
            if (macro_args.size() != macro.params.size())
                PREPROCESSOR_ERROR(
                    std::format("Macro '{}' expected {} arguments, but {} were provided", macro.name, macro.params.size(), macro_args.size())
                );

            // Map parameter names to their arguments
            HashMap<std::string, std::vector<Token>> arg_map;
            for (size_t i = 0; i < macro.params.size(); ++i)
                arg_map[macro.params[i]] = macro_args[i];

            // Replace parameters in the macro body
            std::vector<Token> expanded_body;
            for (const Token &body_tok : macro.body)
            {
                if (body_tok.type == TokenType::IDENTIFIER && arg_map.count(body_tok.value))
                {
                    const std::vector<Token> &replacement = arg_map[body_tok.value];
                    expanded_body.insert(expanded_body.end(), replacement.begin(), replacement.end());
                }
                else
                    expanded_body.push_back(body_tok);
            }

            // Replace macro invocation with expanded body
            toks.erase(toks.begin() + start_pos, toks.begin() + pos + 1);
            toks.insert(toks.begin() + start_pos, expanded_body.begin(), expanded_body.end());

            // Reset pos to start of the macro replacement
            pos = start_pos;
            return;
        }
        else
            consume();
    }
}

Macro Preprocessor::parse_macro()
{
    std::vector<Token> toks = program.tokens->tokens;
    // Consume 'macro' keyword
    pos++;

    if (pos >= toks.size() || peek().type != TokenType::IDENTIFIER)
        PREPROCESSOR_ERROR("Expected macro identifier after 'macro' keyword");

    auto it = macro_table.find(peek().value);
    if (it != macro_table.end())
        PREPROCESSOR_ERROR(std::format("Redefinition of macro '{}', previously defined on line {}", peek().value, it->second.line));

    Macro mac;
    mac.line = peek().line;
    mac.begin = pos - 1;
    mac.name = consume().value;

    // Expect opening parenthesis
    if (pos >= toks.size() || peek().type != TokenType::PAREN_OPEN)
        PREPROCESSOR_ERROR("Expected '(' after macro name");

    consume(); // Consume '('

    // Parse macro parameters
    while (pos < toks.size() && peek().type != TokenType::PAREN_CLOSE)
    {
        if (peek().type == TokenType::COMMA)
        {
            consume();
            continue;
        }

        if (peek().type != TokenType::IDENTIFIER)
            PREPROCESSOR_ERROR("Invalid macro parameter name");

        mac.params.push_back(consume().value);
    }

    // Expect closing parenthesis
    if (pos >= toks.size() || peek().type != TokenType::PAREN_CLOSE)
        PREPROCESSOR_ERROR("Expected ')' after macro parameters");

    consume(); // Consume ')'

    // Expect opening brace for macro body
    if (pos >= toks.size() || peek().type != TokenType::BRACE_OPEN)
        PREPROCESSOR_ERROR("Expected '{' to start macro body.");

    consume(); // Consume '{'

    // Parse macro body
    while (pos < toks.size() && peek().type != TokenType::BRACE_CLOSE)
        mac.body.push_back(toks.at(pos++));

    // Expect closing brace
    if (pos >= toks.size() || peek().type != TokenType::BRACE_CLOSE)
        PREPROCESSOR_ERROR("Expected '}' to close macro body.");

    consume(); // Consume '}'

    mac.end = pos;
    macro_table[mac.name] = mac;

    return mac;
}

} // namespace via