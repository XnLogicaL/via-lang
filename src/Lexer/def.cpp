// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "def.h"
#include "preproc.h"

VIA_NAMESPACE_BEGIN

Definition Preprocessor::parse_definition() {
    Definition def;
    def.begin               = pos;
    std::vector<Token> toks = program.tokens->tokens;

    // Ensure there are enough tokens to parse
    if (pos >= toks.size())
        PREPROCESSOR_ERROR("Unexpected end of input after 'define'");

    auto it = def_table.find(peek().lexeme);
    if (it != def_table.end())
        PREPROCESSOR_ERROR(
            std::format("Redefinition of definition '{}', previously defined on line {}",
                it->second.identifier, it->second.line));

    // Consume 'define' keyword and extract identifier
    consume();
    def.line       = peek().line;
    def.identifier = consume().lexeme;

    // Check and consume opening parenthesis
    if (++pos >= toks.size() || peek().type != TokenType::PAREN_OPEN)
        PREPROCESSOR_ERROR("Expected '(' after identifier in definition");

    // Collect tokens until closing parenthesis
    while (++pos < toks.size() && peek().type != TokenType::PAREN_CLOSE)
        def.replacement.push_back(toks.at(pos));

    // Ensure a closing parenthesis was found
    if (pos >= toks.size() || peek().type != TokenType::PAREN_CLOSE)
        PREPROCESSOR_ERROR("Missing closing ')' in definition");

    // Consume closing parenthesis
    ++pos;
    def.end                   = pos;
    def_table[def.identifier] = def;

    return def;
}

void Preprocessor::expand_definition(const Definition& def) {
    std::vector<Token> toks = program.tokens->tokens;

    // Iterate through all tokens in the program
    for (SIZE i = 0; i < toks.size(); ++i) {
        const Token& tok = toks.at(i);

        // If we find the definition identifier, replace it
        if (tok.lexeme == def.identifier && tok.type == TokenType::IDENTIFIER) {
            // Erase the identifier token
            toks.erase(toks.begin() + i);
            // Insert the replacement tokens at the same position
            toks.insert(toks.begin() + i, def.replacement.begin(), def.replacement.end());

            // Adjust the index to account for the inserted tokens
            i += def.replacement.size() - 1;
        }
    }
}

VIA_NAMESPACE_END
