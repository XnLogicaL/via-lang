/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "macro.h"
#include "preproc.h"

namespace via::Tokenization
{

// TODO: Add safety mechanisms such as;
// - Expansion depth limit
// - Restricted access to keywords, specifically the preprocessor keywords
void Preprocessor::expand_macro(const Macro &macro)
{
    std::vector<Token> &toks = container.tokens;

    while (pos < toks.size())
    {
        const Token &tok = peek();

        // Match macro_name!( pattern
        if (tok.value == macro.name && pos + 2 < toks.size() && peek(1).type == TokenType::EXCLAMATION && peek(2).type == TokenType::PAREN_OPEN)
        {
            size_t start_pos = pos; // Save the macro invocation start position
            consume(3);             // Consume macro_name!(

            // Parse macro arguments
            std::vector<std::vector<Token>> macro_args;
            std::vector<Token> current_arg;
            size_t depth = 1;

            while (pos < toks.size() && depth > 0)
            {
                if (peek().type == TokenType::PAREN_OPEN)
                    depth++;
                else if (peek().type == TokenType::PAREN_CLOSE)
                {
                    depth--;
                    if (depth == 0)
                        break;
                }

                if (toks.at(pos).type == TokenType::COMMA && depth == 1)
                {
                    macro_args.push_back(current_arg);
                    current_arg.clear();
                }
                else
                    current_arg.push_back(peek());

                consume();
            }

            if (!current_arg.empty())
                macro_args.push_back(current_arg);

            // Check for argument count mismatch
            if (macro_args.size() != macro.params.size())
                PREPROCESSOR_ERROR(
                    std::format("Macro '{}' expected {} arguments, but {} were provided", macro.name, macro.params.size(), macro_args.size())
                );

            // Map parameter names to their arguments
            std::unordered_map<std::string, std::vector<Token>> arg_map;
            for (size_t i = 0; i < macro.params.size(); ++i)
                arg_map[macro.params[i]] = macro_args[i];

            // Replace parameters in the macro body
            std::vector<Token> expanded_body;
            for (const Token &body_tok : macro.body)
            {
                if (body_tok.type == TokenType::IDENTIFIER && arg_map.count(body_tok.value))
                {
                    // Replace parameter with corresponding argument tokens
                    const std::vector<Token> &replacement = arg_map[body_tok.value];
                    expanded_body.insert(expanded_body.end(), replacement.begin(), replacement.end());
                }
                else
                    expanded_body.push_back(body_tok);
            }

            // Replace macro invocation with the expanded body
            toks.erase(toks.begin() + start_pos, toks.begin() + pos + 1);
            toks.insert(toks.begin() + start_pos, expanded_body.begin(), expanded_body.end());

            // Reset position to account for expanded tokens
            pos = start_pos + expanded_body.size();
        }
        else
            consume();
    }
}

Macro Preprocessor::parse_macro()
{
    std::vector<Token> &toks = container.tokens;

    // Consume 'macro' keyword
    pos++;

    if (pos >= toks.size() || peek().type != TokenType::IDENTIFIER)
        PREPROCESSOR_ERROR("Expected macro identifier after 'macro' keyword.");

    Macro mac;
    mac.name = consume().value;

    // Expect opening parenthesis
    if (pos >= toks.size() || peek().type != TokenType::PAREN_OPEN)
        PREPROCESSOR_ERROR("Expected '(' after macro name.");

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
            PREPROCESSOR_ERROR("Invalid macro parameter name.");

        mac.params.push_back(consume().value);
    }

    // Expect closing parenthesis
    if (pos >= toks.size() || peek().type != TokenType::PAREN_CLOSE)
        PREPROCESSOR_ERROR("Expected ')' after macro parameters.");

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

    pos++; // Consume '}'

    return mac;
}

} // namespace via::Tokenization