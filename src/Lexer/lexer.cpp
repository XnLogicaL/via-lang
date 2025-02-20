// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#include "lexer.h"
#include "token.h"

#define BINARY_LITERAL_SENTINEL ('b')
#define HEX_LITERAL_SENTINEL ('x')

namespace via {

using enum TokenType;

// Function that returns whether if a character is allowed within a hexadecimal literal
bool Tokenizer::is_hex_char(char chr)
{
    return (chr >= 'A' && chr <= 'F') || (chr >= 'a' && chr <= 'f');
}

size_t Tokenizer::source_size()
{
    return program->source.size();
}

char Tokenizer::peek(size_t ahead)
{
    if (pos + ahead >= source_size()) {
        return '\0';
    }

    return program->source.at(pos + ahead);
}

char Tokenizer::consume(size_t ahead)
{
    if (pos + ahead >= source_size()) {
        return '\0';
    }

    return program->source.at(pos += ahead);
}

Token Tokenizer::read_number()
{
    TokenType type = LIT_INT;     // Specify default type as integer literal
    size_t start_offset = offset; // Record starting offset of the number
    std::string value;            // Value of the number, as a string for convenience

    // Check for binary or hex literals
    if (peek() == '0' && !std::isdigit(peek(1))) {
        consume(); // Consume '0'

        if (peek() == BINARY_LITERAL_SENTINEL)
            type = LIT_BINARY;
        else if (peek() == HEX_LITERAL_SENTINEL)
            type = LIT_HEX;
        else // Unknown number literal
            type = UNKNOWN;

        consume(); // Consume 'b' or 'x'
    }

    // Read the number until the current character isn't numeric
    while (pos < source_size() && (std::isdigit(peek()) || (type == LIT_HEX && is_hex_char(peek())))
    ) {
        value.push_back(peek());
        pos++;
        offset++;
    }

    // Check for floating point
    if (pos < source_size() && peek() == '.') {
        value.push_back(peek());
        // Since it's proven that there is a floating point in the number literal
        // We can safely categorize it as a float literal
        type = LIT_FLOAT;
        pos++;
        offset++;

        while (pos < source_size() && std::isdigit(peek())) {
            value.push_back(peek());
            pos++;
            offset++;
        }
    }

    // Use start_offset here to denote where the token begins
    return Token(type, value, line, start_offset);
}

Token Tokenizer::read_ident()
{
    // List of allowed special characters that can be included in an identifier
    static const std::vector<char> allowed_identifier_spec_chars = {
        '_', '!' /* For macros */
    };

    // Default type, this is because this might be an identifier, keyword or boolean literal
    // We can't know in advance which.
    TokenType type = IDENTIFIER;
    size_t start_offset = offset; // Record starting offset of the identifier
    std::string identifier;

    // Lambda for checking if a character is allowed within an identifier
    auto is_allowed = [this](char chr) -> bool {
        auto allow_list = allowed_identifier_spec_chars;
        bool is_alnum = isalnum(chr);
        bool is_allowed = std::ranges::find(allow_list, peek()) != allow_list.end();
        return is_alnum || is_allowed;
    };

    // Read identifier while position is inside bounds and the current character is allowed within
    // an identifier
    while (pos < source_size() && is_allowed(peek())) {
        identifier.push_back(peek());
        pos++;
        offset++;
    }

    static const std::unordered_map<std::string, TokenType> keyword_map = {
        {"do", KW_DO},
        {"in", KW_IN},
        {"local", KW_LOCAL},
        {"global", KW_GLOBAL},
        {"as", KW_AS},
        {"const", KW_CONST},
        {"if", KW_IF},
        {"else", KW_ELSE},
        {"elif", KW_ELIF},
        {"while", KW_WHILE},
        {"for", KW_FOR},
        {"return", KW_RETURN},
        {"func", KW_FUNC},
        {"break", KW_BREAK},
        {"continue", KW_CONTINUE},
        {"switch", KW_MATCH},
        {"case", KW_CASE},
        {"default", KW_DEFAULT},
        {"new", KW_NEW},
        {"and", KW_AND},
        {"not", KW_NOT},
        {"or", KW_OR},
        {"struct", KW_STRUCT},
        {"namespace", KW_NAMESPACE},
        {"property", KW_PROPERTY},
        {"import", KW_IMPORT},
        {"export", KW_EXPORT},
        {"macro", KW_MACRO},
        {"define", KW_DEFINE},
        {"strict", KW_STRICT},
        {"meta", KW_META},
        {"defined", KW_DEFINED},
    };

    // Checks if the identifier is a keyword or not
    auto it = keyword_map.find(identifier);
    if (it != keyword_map.end()) {
        // If so, overwrite the identifier type with the respective keyword type
        type = it->second;
    }

    // Checks if the identifier is a boolean literal
    if (identifier == "true" || identifier == "false") {
        // Pretty self-explanatory.
        type = LIT_BOOL;
    }

    if (identifier == "nil") {
        type = LIT_NIL;
    }

    return Token(type, identifier, line, start_offset); // Use start_offset here
}

Token Tokenizer::read_string()
{
    std::string lexeme;
    size_t start_offset = offset; // Record starting offset of the string
    pos++;                        // Skip opening quote
    offset++;

    while (pos < source_size() && peek() != '"') {
        if (peek() == '\\') {
            pos++;
            offset++;

            if (pos < source_size()) {
                char escape_char = peek();
                switch (escape_char) {
                case 'n':
                    lexeme.push_back('\n');
                    break;
                case 't':
                    lexeme.push_back('\t');
                    break;
                case 'r':
                    lexeme.push_back('\r');
                    break;
                default:
                    lexeme.push_back(escape_char);
                    break;
                }
            }
        }
        else {
            char chr = peek();
            lexeme.push_back(chr);
        }

        pos++;
        offset++;
    }

    pos++; // Skip closing quote
    offset++;

    return Token(LIT_STRING, lexeme, line, start_offset); // Use start_offset here
}

Token Tokenizer::get_token()
{
    // Skip whitespace and single-line comments
    while (pos < source_size()) {
        // Skip whitespace
        if (isspace(peek())) {
            if (peek() == '\n') {
                line++;
                offset = 0;
            }
            else {
                offset++;
            }

            pos++;
            continue;
        }

        // Skip single-line comments
        if (peek() == '#' && pos + 1 < source_size() && peek(1) == '#') {
            pos += 2;
            offset += 2;

            while (pos < source_size() && peek() != '\n') {
                pos++;
                offset++;
            }

            continue;
        }

        // Skip block comments
        if (peek() == '#' && pos + 1 < source_size() && peek(1) == '[') {
            pos += 2;
            offset += 2;

            while (pos + 1 < source_size() && !(peek() == ']' && peek(1) == '#')) {
                if (peek() == '\n') {
                    line++;
                    offset = 0;
                }

                pos++;
                offset++;
            }

            if (pos + 1 < source_size()) {
                pos += 2; // Skip ']#'
                offset += 2;
            }

            continue;
        }

        // If no match, break
        break;
    }

    // Check if the position is at the end of the program->source string
    // If so, return an EOF token meant as a sentinel
    if (pos >= source_size()) {
        return {EOF_, "\0", line, offset};
    }

    size_t start_offset = offset; // Record starting offset of each token

    // Handle numbers
    if (std::isdigit(peek())) {
        return read_number();
    }

    // Handle string literals
    if (peek() == '"') {
        return read_string();
    }

    // Handle identifiers and keywords
    if (std::isalpha(peek()) || peek() == '_') {
        return read_ident();
    }

    // Handle special characters (operators, delimiters, etc.)
    char chr = peek();

    pos++;
    offset++;

    switch (chr) {
    case '+':
        return Token(OP_ADD, "+", line, start_offset);
    case '-':
        return Token(OP_SUB, "-", line, start_offset);
    case '*':
        return Token(OP_MUL, "*", line, start_offset);
    case '/':
        return Token(OP_DIV, "/", line, start_offset);
    case '%':
        return Token(OP_MOD, "%", line, start_offset);
    case '^':
        return Token(OP_EXP, "^", line, start_offset);
    case '=':
        if (pos < source_size() && peek() == '=') {
            pos++;
            offset++;
            return Token(OP_EQ, "==", line, start_offset);
        }
        return Token(OP_ASGN, "=", line, start_offset);
    case '!':
        if (pos < source_size() && peek() == '=') {
            pos++;
            offset++;
            return Token(OP_NEQ, "!=", line, start_offset);
        }
        return Token(EXCLAMATION, "!", line, start_offset);
    case '<':
        return Token(OP_LT, "<", line, start_offset);
    case '>':
        return Token(OP_GT, ">", line, start_offset);
    case '&':
        return Token(AMPERSAND, "&", line, start_offset);
    case '|':
        return Token(PIPE, "|", line, start_offset);
    case ';':
        return Token(SEMICOLON, ";", line, start_offset);
    case ',':
        return Token(COMMA, ",", line, start_offset);
    case '(':
        return Token(PAREN_OPEN, "(", line, start_offset);
    case ')':
        return Token(PAREN_CLOSE, ")", line, start_offset);
    case '{':
        return Token(BRACE_OPEN, "{", line, start_offset);
    case '}':
        return Token(BRACE_CLOSE, "}", line, start_offset);
    case '[':
        return Token(BRACKET_OPEN, "[", line, start_offset);
    case ']':
        return Token(BRACKET_CLOSE, "]", line, start_offset);
    case '.':
        return Token(DOT, ".", line, start_offset);
    case ':':
        return Token(COLON, ":", line, start_offset);
    case '@':
        return Token(AT, "@", line, start_offset);
    case '?':
        return Token(QUESTION, "?", line, start_offset);
    default:
        return Token(UNKNOWN, std::string(1, chr), line, start_offset);
    }

    return Token(UNKNOWN, "\0", line, start_offset);
}

void Tokenizer::tokenize()
{
    TokenHolder *tokens = program->tokens;

    while (true) {
        Token token = get_token();
        tokens->tokens.push_back(token);

        if (token.type == EOF_) {
            break;
        }
    }
}

} // namespace via
