/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "macro.h"

using namespace via::Tokenization;

// TODO: Add safety mechanisms such as;
// - Expansion depth limit
// - Restricted access to keywords, specifically the preprocessor keywords
void Preprocessing::expand_macro(std::vector<Token> *toks, const Macro &macro)
{
    size_t i = 0;

    // Iterate through the token list
    while (i < toks->size())
    {
        const Token &tok = toks->at(i);

        // Check for macro invocation: `macro_name!(` pattern
        if (tok.value == macro.name && i + 2 < toks->size() && toks->at(i + 1).type == TokenType::EXCLAMATION &&
            toks->at(i + 2).type == TokenType::PAREN_OPEN)
        {
            // Parse macro arguments
            std::vector<std::vector<Token>> macro_args;
            size_t arg_start = i + 3;
            size_t depth = 1;

            // Parse arguments until the closing parenthesis
            std::vector<Token> current_arg;

            for (size_t j = arg_start; j < toks->size(); ++j)
            {
                if (toks->at(j).type == TokenType::PAREN_OPEN)
                    depth++;
                else if (toks->at(j).type == TokenType::PAREN_CLOSE)
                {
                    depth--;

                    if (depth == 0)
                    {
                        if (!current_arg.empty())
                            macro_args.push_back(std::move(current_arg));

                        arg_start = j + 1;
                        break;
                    }
                }
                else if (toks->at(j).type == TokenType::COMMA && depth == 1)
                {
                    macro_args.push_back(std::move(current_arg));
                    current_arg.clear();
                }
                else
                    current_arg.push_back(toks->at(j));
            }

            // Check for argument count mismatch
            if (macro_args.size() != macro.params.size())
                throw PreprocessorException(std::format("Macro argument count mismatch for macro '{}'", macro.name));

            // Replace the macro call with its expanded body
            std::vector<Token> expanded_body = macro.body;
            std::unordered_map<std::string, std::vector<Token>> arg_map;

            // Map parameter names to their arguments
            for (size_t k = 0; k < macro.params.size(); ++k)
                arg_map[macro.params[k]] = macro_args[k];

            // Replace parameters in the macro body with their arguments
            for (Token &body_tok : expanded_body)
                if (body_tok.type == TokenType::IDENTIFIER && arg_map.count(body_tok.value))
                    // Expand parameter with corresponding argument tokens
                    body_tok = arg_map[body_tok.value][0];

            // Remove the macro invocation and insert the expanded body
            toks->erase(toks->begin() + i, toks->begin() + arg_start);
            toks->insert(toks->begin() + i, expanded_body.begin(), expanded_body.end());

            // Adjust index to account for the expanded tokens
            i += expanded_body.size();
        }
        else
            i++;
    }
}

Preprocessing::Macro Preprocessing::parse_macro(std::vector<Token> *toks, size_t &pos)
{
    // Consume 'macro' keyword
    pos++;

    Preprocessing::Macro mac;
    mac.name = toks->at(pos++).value;

    // Consume opening parantheses
    pos++;

    // Initialize parameter index for keeping track of macro parameters
    size_t parameter_idx = 0;
    std::string parameter;

    // Loop through macro parameters until closing parantheses
    while (toks->at(pos).type != TokenType::PAREN_CLOSE)
    {
        if (toks->at(pos).type == TokenType::COMMA)
        {
            // Just in case the parameter string doesn't get cloned properly
            std::string param_clone = parameter;

            mac.params.push_back(parameter);
            parameter.clear();

            // Consume comma and increment param index
            parameter_idx++;
            pos++;

            continue;
        }

        // Add the current tokens value to the parameter identifier
        parameter += toks->at(pos).value;
        pos++;
    }

    // Consume ) and {
    pos++;
    pos++;

    while (toks->at(pos).type != TokenType::BRACE_CLOSE)
    {
        mac.body.push_back(toks->at(pos));
        pos++;
    }

    // Consume }
    pos++;

    return mac;
}