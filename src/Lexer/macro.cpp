// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "macro.h"
#include "preproc.h"

VIA_NAMESPACE_BEGIN

// TODO: Add safety mechanisms such as;
// - Expansion depth limit
// - Restricted access to keywords, specifically the preprocessor keywords
void Preprocessor::expand_macro(const Macro& macro) {
    TokenStream& token_stream = *program.token_stream;

    while (pos < token_stream.size()) {
        const Token& tok = peek();

        // Match macro_name!( pattern
        if (tok.lexeme.ends_with('!') &&
            tok.lexeme.substr(0, tok.lexeme.length() - 1) == macro.name &&
            pos + 1 < token_stream.size() && peek(1).type == TokenType::PAREN_OPEN) {
            size_t start_pos = pos;
            consume(2); // Consume macro_name! and the opening parenthesis

            // Parse macro arguments
            std::vector<std::vector<Token>> macro_args;
            std::vector<Token>              current_arg;
            size_t                          depth = 1;

            // Loop to parse arguments within parentheses
            while (pos < token_stream.size()) {
                const Token& current = peek();

                // Handle nested parentheses
                if (current.type == TokenType::PAREN_OPEN)
                    depth++;
                else if (current.type == TokenType::PAREN_CLOSE) {
                    depth--;
                    if (depth == 0)
                        break; // End of arguments
                }
                else if (current.type == TokenType::COMMA && depth == 1) {
                    // End of current argument
                    if (!current_arg.empty()) {
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
                if (pos < token_stream.size())
                    consume();
                else
                    break; // Prevent consuming EOF
            }

            // Check for unmatched parentheses (error handling)
            if (depth > 0) {
                PREPROCESSOR_ERROR(pos, "Unmatched parentheses in macro invocation");
                return;
            }

            if (!current_arg.empty())
                macro_args.push_back(current_arg);

            // Check for argument count mismatch
            if (macro_args.size() != macro.params.size())
                PREPROCESSOR_ERROR(
                    pos,
                    std::format(
                        "Macro '{}' expected {} arguments, but {} were provided",
                        macro.name,
                        macro.params.size(),
                        macro_args.size()
                    )
                );

            // Map parameter names to their arguments
            std::unordered_map<std::string, std::vector<Token>> arg_map;
            for (size_t i = 0; i < macro.params.size(); ++i)
                arg_map[macro.params[i]] = macro_args[i];

            // Replace parameters in the macro body
            std::vector<Token> expanded_body;
            for (const Token& body_tok : macro.body) {
                if (body_tok.type == TokenType::IDENTIFIER && arg_map.count(body_tok.lexeme)) {
                    const std::vector<Token>& replacement = arg_map[body_tok.lexeme];
                    expanded_body.insert(
                        expanded_body.end(), replacement.begin(), replacement.end()
                    );
                }
                else
                    expanded_body.push_back(body_tok);
            }

            auto& tokens = token_stream.get();

            // Replace macro invocation with expanded body
            tokens.erase(tokens.begin() + start_pos, tokens.begin() + pos + 1);
            tokens.insert(tokens.begin() + start_pos, expanded_body.begin(), expanded_body.end());

            // Reset pos to start of the macro replacement
            pos = start_pos;
            return;
        }
        else
            consume();
    }
}

Macro Preprocessor::parse_macro() {
    TokenStream& token_stream = *program.token_stream;
    // Consume 'macro' keyword
    pos++;

    if (pos >= token_stream.size() || peek().type != TokenType::IDENTIFIER) {
        PREPROCESSOR_ERROR(pos, "Expected macro identifier after 'macro' keyword");
    }

    auto it = macro_table.find(peek().lexeme);
    if (it != macro_table.end())
        PREPROCESSOR_ERROR(
            pos,
            std::format(
                "Redefinition of macro '{}', previously defined on line {}",
                peek().lexeme,
                it->second.line
            )
        );

    Macro mac;
    mac.line  = peek().line;
    mac.begin = pos - 1;
    mac.name  = consume().lexeme;

    // Expect opening parenthesis
    if (pos >= token_stream.size() || peek().type != TokenType::PAREN_OPEN)
        PREPROCESSOR_ERROR(pos, "Expected '(' after macro name");

    consume(); // Consume '('

    // Parse macro parameters
    while (pos < token_stream.size() && peek().type != TokenType::PAREN_CLOSE) {
        if (peek().type == TokenType::COMMA) {
            consume();
            continue;
        }

        if (peek().type != TokenType::IDENTIFIER)
            PREPROCESSOR_ERROR(pos, "Invalid macro parameter name");

        mac.params.push_back(consume().lexeme);
    }

    // Expect closing parenthesis
    if (pos >= token_stream.size() || peek().type != TokenType::PAREN_CLOSE)
        PREPROCESSOR_ERROR(pos, "Expected ')' after macro parameters");

    consume(); // Consume ')'

    // Expect opening brace for macro body
    if (pos >= token_stream.size() || peek().type != TokenType::BRACE_OPEN)
        PREPROCESSOR_ERROR(pos, "Expected '{' to start macro body.");

    consume(); // Consume '{'

    // Parse macro body
    while (pos < token_stream.size() && peek().type != TokenType::BRACE_CLOSE)
        mac.body.push_back(token_stream.at(pos++));

    // Expect closing brace
    if (pos >= token_stream.size() || peek().type != TokenType::BRACE_CLOSE)
        PREPROCESSOR_ERROR(pos, "Expected '}' to close macro body.");

    consume(); // Consume '}'

    mac.end               = pos;
    macro_table[mac.name] = mac;

    return mac;
}

VIA_NAMESPACE_END
