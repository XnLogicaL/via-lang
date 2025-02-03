/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "lexer.h"
#include "token.h"

#define BINARY_LITERAL_SENTINEL ('b')
#define HEX_LITERAL_SENTINEL ('x')

// Macro for quickly construction tokens
// Uses arena allocator for emplacing the newly created token
#define CREATE_TOKEN(type, val, line, off) *alloc->emplace<Token>(type, val, line, off)

// We use this rather than `using namespace via`
namespace via
{

// Simple function that returns whether if a character is allowed within a hexadecimal literal
bool Tokenizer::is_hex_char(char chr)
{
    return (chr >= 'A' && chr <= 'F') || (chr >= 'a' && chr <= 'f');
}

size_t Tokenizer::source_size()
{
    return program.source.size();
}

char Tokenizer::peek(size_t ahead)
{
    return program.source.at(pos + ahead);
}

char Tokenizer::consume(size_t ahead)
{
    return program.source.at(pos += ahead);
}

Token Tokenizer::read_number()
{
    TokenType type = TokenType::LIT_INT; // Specify default type as integer literal
    size_t start_offset = offset;        // Record starting offset of the number
    std::string value;                   // Value of the number, as a string for convenience

    // Check for binary or hex literals
    if (peek() == '0' && !std::isdigit(peek(1)))
    {
        consume(); // Consume '0'

        if (peek() == BINARY_LITERAL_SENTINEL)
            type = TokenType::LIT_BINARY;
        else if (peek() == HEX_LITERAL_SENTINEL)
            type = TokenType::LIT_HEX;
        else // Unknown number literal
            type = TokenType::UNKNOWN;

        consume(); // Consume 'b' or 'x'
    }

    // Read the number until the current character isn't numeric
    while (pos < source_size() && (std::isdigit(peek()) || (type == TokenType::LIT_HEX && is_hex_char(peek()))))
    {
        value.push_back(peek());
        pos++;
        offset++;
    }

    // Check for floating point
    if (pos < source_size() && peek() == '.')
    {
        value.push_back(peek());
        // Since it's proven that there is a floating point in the number literal
        // We can safely categorize it as a float literal
        type = TokenType::LIT_FLOAT;
        pos++;
        offset++;

        while (pos < source_size() && std::isdigit(peek()))
        {
            value.push_back(peek());
            pos++;
            offset++;
        }
    }

    // Use start_offset here to denote where the token begins
    return CREATE_TOKEN(type, value, line, start_offset);
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
    auto is_allowed = [this](char chr) -> bool
    {
        auto allow_list = allowed_identifier_spec_chars;
        bool is_alnum = isalnum(chr);
        bool is_allowed = std::ranges::find(allow_list, peek()) != allow_list.end();
        return is_alnum || is_allowed;
    };

    // Read identifier while position is inside bounds and the current character is allowed within an identifier
    while (pos < source_size() && is_allowed(peek()))
    {
        identifier.push_back(peek());
        pos++;
        offset++;
    }

    static const std::unordered_map<std::string, TokenType> keyword_map = {
        {"do", TokenType::KW_DO},
        {"in", TokenType::KW_IN},
        {"local", TokenType::KW_LOCAL},
        {"global", TokenType::KW_GLOBAL},
        {"as", TokenType::KW_AS},
        {"const", TokenType::KW_CONST},
        {"if", TokenType::KW_IF},
        {"else", TokenType::KW_ELSE},
        {"elif", TokenType::KW_ELIF},
        {"while", TokenType::KW_WHILE},
        {"for", TokenType::KW_FOR},
        {"return", TokenType::KW_RETURN},
        {"func", TokenType::KW_FUNC},
        {"break", TokenType::KW_BREAK},
        {"continue", TokenType::KW_CONTINUE},
        {"switch", TokenType::KW_MATCH},
        {"case", TokenType::KW_CASE},
        {"default", TokenType::KW_DEFAULT},
        {"new", TokenType::KW_NEW},
        {"and", TokenType::KW_AND},
        {"not", TokenType::KW_NOT},
        {"or", TokenType::KW_OR},
        {"struct", TokenType::KW_STRUCT},
        {"namespace", TokenType::KW_NAMESPACE},
        {"property", TokenType::KW_PROPERTY},
        {"import", TokenType::KW_IMPORT},
        {"export", TokenType::KW_EXPORT},
        {"macro", TokenType::KW_MACRO},
        {"define", TokenType::KW_DEFINE},
        {"strict", TokenType::KW_STRICT},
        {"meta", TokenType::KW_META},
        {"defined", TokenType::KW_DEFINED},
    };

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

    return CREATE_TOKEN(type, identifier, line, start_offset); // Use start_offset here
}

Token Tokenizer::read_string()
{
    std::string value;
    size_t start_offset = offset; // Record starting offset of the string
    pos++;                        // Skip opening quote
    offset++;

    while (pos < source_size() && peek() != '"')
    {
        if (peek() == '\\')
        {
            pos++;
            offset++;

            if (pos < source_size())
            {
                char escape_char = peek();
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
            value.push_back(peek());

        pos++;
        offset++;
    }

    pos++; // Skip closing quote
    offset++;

    return CREATE_TOKEN(TokenType::LIT_STRING, value, line, start_offset); // Use start_offset here
}

Token Tokenizer::get_token()
{
    // Skip whitespace and single-line comments
    while (pos < source_size())
    {
        // Skip whitespace
        if (isspace(peek()))
        {
            if (peek() == '\n')
            {
                line++;
                offset = 0;
            }
            else
            {
                offset++;
            }
            pos++;
            continue;
        }

        // Skip single-line comments
        if (peek() == '#' && pos + 1 < source_size() && peek(1) == '#')
        {
            pos += 2;
            offset += 2;
            while (pos < source_size() && peek() != '\n')
            {
                pos++;
                offset++;
            }
            continue;
        }

        // Skip block comments
        if (peek() == '#' && pos + 1 < source_size() && peek(1) == '[')
        {
            pos += 2;
            offset += 2;
            while (pos + 1 < source_size() && !(peek() == ']' && peek(1) == '#'))
            {
                if (peek() == '\n')
                {
                    line++;
                    offset = 0;
                }
                pos++;
                offset++;
            }
            if (pos + 1 < source_size())
            {
                pos += 2; // Skip ']#'
                offset += 2;
            }
            continue;
        }

        // If no match, break
        break;
    }

    // Check if the position is at the end of the program.source string
    // If so, return an EOF token meant as a sentinel
    if (pos >= source_size())
        return {TokenType::EOF_, "", line, offset};

    size_t start_offset = offset; // Record starting offset of each token

    // Handle numbers
    if (std::isdigit(peek()))
        return read_number();

    // Handle string literals
    if (peek() == '"')
        return read_string();

    // Handle identifiers and keywords
    if (std::isalpha(peek()) || peek() == '_')
        return read_ident();

    // Handle special characters (operators, delimiters, etc.)
    char chr = peek();

    pos++;
    offset++;

    switch (chr)
    {
    case '+':
        return CREATE_TOKEN(TokenType::OP_ADD, "+", line, start_offset);
    case '-':
        return CREATE_TOKEN(TokenType::OP_SUB, "-", line, start_offset);
    case '*':
        return CREATE_TOKEN(TokenType::OP_MUL, "*", line, start_offset);
    case '/':
        return CREATE_TOKEN(TokenType::OP_DIV, "/", line, start_offset);
    case '%':
        return CREATE_TOKEN(TokenType::OP_MOD, "%", line, start_offset);
    case '^':
        return CREATE_TOKEN(TokenType::OP_EXP, "^", line, start_offset);
    case '=':
        if (pos < source_size() && peek() == '=')
        {
            pos++;
            offset++;
            return CREATE_TOKEN(TokenType::OP_EQ, "==", line, start_offset);
        }
        return CREATE_TOKEN(TokenType::OP_ASGN, "=", line, start_offset);
    case '!':
        if (pos < source_size() && peek() == '=')
        {
            pos++;
            offset++;
            return CREATE_TOKEN(TokenType::OP_NEQ, "!=", line, start_offset);
        }
        return CREATE_TOKEN(TokenType::EXCLAMATION, "!", line, start_offset);
    case '<':
        return CREATE_TOKEN(TokenType::OP_LT, "<", line, start_offset);
    case '>':
        return CREATE_TOKEN(TokenType::OP_GT, ">", line, start_offset);
    case '&':
        return CREATE_TOKEN(TokenType::AMPERSAND, "&", line, start_offset);
    case '|':
        return CREATE_TOKEN(TokenType::PIPE, "|", line, start_offset);
    case ';':
        return CREATE_TOKEN(TokenType::SEMICOLON, ";", line, start_offset);
    case ',':
        return CREATE_TOKEN(TokenType::COMMA, ",", line, start_offset);
    case '(':
        return CREATE_TOKEN(TokenType::PAREN_OPEN, "(", line, start_offset);
    case ')':
        return CREATE_TOKEN(TokenType::PAREN_CLOSE, ")", line, start_offset);
    case '{':
        return CREATE_TOKEN(TokenType::BRACE_OPEN, "{", line, start_offset);
    case '}':
        return CREATE_TOKEN(TokenType::BRACE_CLOSE, "}", line, start_offset);
    case '[':
        return CREATE_TOKEN(TokenType::BRACKET_OPEN, "[", line, start_offset);
    case ']':
        return CREATE_TOKEN(TokenType::BRACKET_CLOSE, "]", line, start_offset);
    case '.':
        return CREATE_TOKEN(TokenType::DOT, ".", line, start_offset);
    case ':':
        return CREATE_TOKEN(TokenType::COLON, ":", line, start_offset);
    case '@':
        return CREATE_TOKEN(TokenType::AT, "@", line, start_offset);
    case '?':
        return CREATE_TOKEN(TokenType::QUESTION, "?", line, start_offset);
    default:
        return CREATE_TOKEN(TokenType::UNKNOWN, std::string(1, chr), line, start_offset);
    }

    return CREATE_TOKEN(TokenType::UNKNOWN, "", line, start_offset);
}

void Tokenizer::tokenize()
{
    TokenHolder *tokens = new TokenHolder(VIA_LEXER_ALLOC_SIZE);
    this->alloc = &tokens->alloc;

    while (true)
    {
        Token token = get_token();
        tokens->tokens.push_back(token);

        if (token.type == TokenType::EOF_)
            break;
    }

    program.tokens = tokens;
}

} // namespace via
