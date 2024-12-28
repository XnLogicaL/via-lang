/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "lexer.h"
#include "token.h"

// Macro for quickly construction tokens
// Uses arena allocator for emplacing the newly created token
#define TOKEN(type, val, line, off) *alloc.emplace<Token>(type, val, line, off)

// We use this rather than `using namespace via::Tokenization`
namespace via::Tokenization
{

Token Tokenizer::read_number()
{
    // Specify default type as integer literal
    // This is because there's no way to know if the literal is a float before reading it
    // Therefore it is assumed to be an integer literal until proven otherwise
    TokenType type = TokenType::LIT_INT;
    // Record starting offset of the number
    size_t start_offset = offset;
    // Value of the number, as a string for convenience
    std::string value;

    // Read the number until the current character isn't numeric
    while (pos < source.size() && isdigit(source.at(pos)))
    {
        value.push_back(source.at(pos));
        pos++;
        offset++;
    }

    // Check for floating point
    if (pos < source.size() && source.at(pos) == '.')
    {
        value.push_back(source.at(pos));
        // Since it's proven that there is a floating point in the number literal
        // We can safely categorize it as a float literal
        type = TokenType::LIT_FLOAT;
        pos++;
        offset++;

        while (pos < source.size() && isdigit(source.at(pos)))
        {
            value.push_back(source.at(pos));
            pos++;
            offset++;
        }
    }

    // Use start_offset here to denote where the token begins
    return TOKEN(type, value, line, start_offset);
}

Token Tokenizer::read_ident()
{
    // List of allowed special characters that can be included in an identifier
    static const std::vector<char> allowed_identifier_spec_chars = {
        '_', '!' /* For macros */
    };

    // Default type, this is because this might be an identifier, keyword or boolean literal
    // We can't know in advance which.
    TokenType type = TokenType::IDENTIFIER;
    size_t start_offset = offset; // Record starting offset of the identifier
    std::string identifier;

    // Lambda for checking if a character is allowed within an identifier
    auto is_allowed = [this](char ch) -> bool
    {
        auto allow_list = allowed_identifier_spec_chars;
        bool is_alnum = isalnum(ch);
        // I love std::find and the ridiculous iterator template
        // Like, who the fuck thought it was a good idea to make people write .begin() and .end() every single fuckin time?
        bool is_allowed = std::find(allow_list.begin(), allow_list.end(), source.at(pos)) != allow_list.end();
        return is_alnum || is_allowed;
    };

    // Read identifier while position is inside bounds and the current character is allowed within an identifier
    while (pos < source.size() && is_allowed(source.at(pos)))
    {
        identifier.push_back(source.at(pos));
        pos++;
        offset++;
    }

    // Have to turn off clang-format here because it make a tower out of these
    // Sorry but I'm not clang-format literate!
    // clang-format off
    static const std::unordered_map<std::string, TokenType> keyword_map = { 
        { "do", TokenType::KW_DO }, { "in", TokenType::KW_IN },
        { "local", TokenType::KW_LOCAL }, { "global", TokenType::KW_GLOBAL },
        { "as", TokenType::KW_AS }, { "const", TokenType::KW_CONST },
        { "if", TokenType::KW_IF }, { "else", TokenType::KW_ELSE },
        { "elseif", TokenType::KW_ELIF }, { "while", TokenType::KW_WHILE },
        { "for", TokenType::KW_FOR }, { "return", TokenType::KW_RETURN },
        { "func", TokenType::KW_FUNC }, { "break", TokenType::KW_BREAK },
        { "continue", TokenType::KW_CONTINUE }, { "switch", TokenType::KW_SWITCH },
        { "case", TokenType::KW_CASE }, { "default", TokenType::KW_DEFAULT },
        { "delete", TokenType::KW_DELETE }, { "new", TokenType::KW_NEW },
        { "and", TokenType::KW_AND }, { "not", TokenType::KW_NOT },
        { "or", TokenType::KW_OR }, { "struct", TokenType::KW_STRUCT },
        { "namespace", TokenType::KW_NAMESPACE }, { "property", TokenType::KW_PROPERTY },
        { "import", TokenType::KW_IMPORT }, { "export", TokenType::KW_EXPORT },
        { "macro", TokenType::KW_MACRO }, { "define", TokenType::KW_DEFINE }
    };
    // clang-format on

    // Checks if the identifier is a keyword or not
    auto it = keyword_map.find(identifier);
    if (it != keyword_map.end())
        // If so, overwrite the identifier type with the respective keyword type
        type = it->second;

    // Checks if the identifier is a boolean literal
    if (identifier == "true" || identifier == "false")
        // Pretty self-explanatory.
        type = TokenType::LIT_BOOL;

    if (identifier == "nil")
        type = TokenType::LIT_NIL;

    return TOKEN(type, identifier, line, start_offset); // Use start_offset here
}

Token Tokenizer::read_string()
{
    std::string value;
    size_t start_offset = offset; // Record starting offset of the string
    pos++;                        // Skip opening quote
    offset++;

    while (pos < source.size() && source.at(pos) != '"')
    {
        if (source.at(pos) == '\\')
        {
            pos++;
            offset++;

            if (pos < source.size())
            {
                char escape_char = source.at(pos);
                switch (escape_char)
                {
                case 'n':
                    value.push_back('\n');
                    break;
                case 't':
                    value.push_back('\t');
                    break;
                case 'r':
                    value.push_back('\r');
                    break;
                default:
                    value.push_back(escape_char);
                    break;
                }
            }
        }
        else
            value.push_back(source.at(pos));

        pos++;
        offset++;
    }

    pos++; // Skip closing quote
    offset++;

    return TOKEN(TokenType::LIT_STRING, value, line, start_offset); // Use start_offset here
}

Token Tokenizer::get_token()
{
    // Skip whitespace
    while (pos < source.size() && isspace(source.at(pos)))
    {
        // Check if there is a line break
        // If so, increment line and reset offset
        if (source.at(pos) == '\n')
        {
            line++;
            offset = 0;
        }
        else
            offset++;

        pos++; // Ensure this increments regardless of newline
    }

    // Check if the position is at the end of the source string
    // If so, return an EOF token meant as a sentinel
    if (pos >= source.size())
        return {TokenType::EOF_, "", line, offset};

    size_t start_offset = offset; // Record starting offset of each token

    // Handle numbers
    if (isdigit(source.at(pos)))
        return read_number();

    // Handle string literals
    if (source.at(pos) == '"')
        return read_string();

    // Handle identifiers and keywords
    if (isalpha(source.at(pos)))
        return read_ident();

    // Handle special characters (operators, delimiters, etc.)
    char ch = source.at(pos);

    pos++;
    offset++;

    switch (ch)
    {
    case '+':
        return TOKEN(TokenType::OP_ADD, "+", line, start_offset);
    case '-':
        return TOKEN(TokenType::OP_SUB, "-", line, start_offset);
    case '*':
        return TOKEN(TokenType::OP_MUL, "*", line, start_offset);
    case '/':
        return TOKEN(TokenType::OP_DIV, "/", line, start_offset);
    case '%':
        return TOKEN(TokenType::OP_MOD, "%", line, start_offset);
    case '^':
        return TOKEN(TokenType::OP_EXP, "^", line, start_offset);
    case '=':
        if (pos < source.size() && source.at(pos) == '=')
        {
            pos++;
            offset++;
            return TOKEN(TokenType::OP_EQ, "==", line, start_offset);
        }
        return TOKEN(TokenType::OP_ASGN, "=", line, start_offset);
    case '!':
        if (pos < source.size() && source.at(pos) == '=')
        {
            pos++;
            offset++;
            return TOKEN(TokenType::OP_NEQ, "!=", line, start_offset);
        }
        return TOKEN(TokenType::EXCLAMATION, "!", line, start_offset);
    case '<':
        return TOKEN(TokenType::OP_LT, "<", line, start_offset);
    case '>':
        return TOKEN(TokenType::OP_GT, ">", line, start_offset);
    case '&':
        return TOKEN(TokenType::AMPERSAND, "&", line, start_offset);
    case '|':
        return TOKEN(TokenType::PIPE, "|", line, start_offset);
    case ';':
        return TOKEN(TokenType::SEMICOLON, ";", line, start_offset);
    case ',':
        return TOKEN(TokenType::COMMA, ",", line, start_offset);
    case '(':
        return TOKEN(TokenType::PAREN_OPEN, "(", line, start_offset);
    case ')':
        return TOKEN(TokenType::PAREN_CLOSE, ")", line, start_offset);
    case '{':
        return TOKEN(TokenType::BRACE_OPEN, "{", line, start_offset);
    case '}':
        return TOKEN(TokenType::BRACE_CLOSE, "}", line, start_offset);
    case '[':
        return TOKEN(TokenType::BRACKET_OPEN, "[", line, start_offset);
    case ']':
        return TOKEN(TokenType::BRACKET_CLOSE, "]", line, start_offset);
    case '.':
        return TOKEN(TokenType::DOT, ".", line, start_offset);
    case ':':
        return TOKEN(TokenType::COLON, ":", line, start_offset);
    case '@':
        return TOKEN(TokenType::AT, "@", line, start_offset);
    case '?':
        return TOKEN(TokenType::QUESTION, "?", line, start_offset);
    default:
        return TOKEN(TokenType::UNKNOWN, std::string(1, ch), line, start_offset);
    }

    return TOKEN(TokenType::UNKNOWN, "", line, start_offset);
}

SrcContainer Tokenizer::tokenize()
{
    std::vector<Token> tokens;

    while (true)
    {
        auto token = get_token();
        tokens.push_back(token);

        if (token.type == TokenType::EOF_)
            break;
    }

    return {tokens, source, ""};
}

} // namespace via::Tokenization
