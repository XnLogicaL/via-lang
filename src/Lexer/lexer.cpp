// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
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

SIZE Tokenizer::source_size()
{
    return program.source.size();
}

char Tokenizer::peek(SIZE ahead)
{
    if (pos + ahead >= source_size()) {
        return '\0';
    }

    return program.source.at(pos + ahead);
}

char Tokenizer::consume(SIZE ahead)
{
    if (pos + ahead >= source_size()) {
        return '\0';
    }

    return program.source.at(pos += ahead);
}

Token Tokenizer::read_number(SIZE position)
{
    TokenType   type         = LIT_INT; // Specify default type as integer literal
    SIZE        start_offset = offset;  // Record starting offset of the number
    std::string value;                  // Value of the number, as a string for convenience
    char        delimiter;

    // Check for binary or hex literals
    if (peek() == '0' && (peek(1) == 'x' || peek(1) == 'b')) {
        consume(); // Consume '0'

        if (peek() == 'b') {
            type = LIT_BINARY;
        }
        else if (peek() == 'x') {
            type = LIT_HEX;
        }

        delimiter = consume(); // Consume 'b' or 'x'
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

    if (type == LIT_HEX || type == LIT_BINARY) {
        value = std::string("0") + delimiter + value;
    }

    // Use start_offset here to denote where the token begins
    return Token(type, value, line, start_offset, position);
}

Token Tokenizer::read_ident(SIZE position)
{
    // List of allowed special characters that can be included in an identifier
    static const std::vector<char> allowed_identifier_spec_chars = {
        '_', '!' /* For macros */
    };

    // Default type, this is because this might be an identifier, keyword or boolean literal
    // We can't know in advance which.
    TokenType   type         = IDENTIFIER;
    SIZE        start_offset = offset; // Record starting offset of the identifier
    std::string identifier;

    // Lambda for checking if a character is allowed within an identifier
    auto is_allowed = [this](char chr) -> bool {
        auto allow_list = allowed_identifier_spec_chars;
        bool is_alnum   = isalnum(chr);
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
        {"do", KW_DO},         {"in", KW_IN},           {"local", KW_LOCAL},
        {"global", KW_GLOBAL}, {"as", KW_AS},           {"const", KW_CONST},
        {"if", KW_IF},         {"else", KW_ELSE},       {"elif", KW_ELIF},
        {"while", KW_WHILE},   {"for", KW_FOR},         {"return", KW_RETURN},
        {"func", KW_FUNC},     {"break", KW_BREAK},     {"continue", KW_CONTINUE},
        {"switch", KW_MATCH},  {"case", KW_CASE},       {"default", KW_DEFAULT},
        {"new", KW_NEW},       {"and", KW_AND},         {"not", KW_NOT},
        {"or", KW_OR},         {"struct", KW_STRUCT},   {"namespace", KW_NAMESPACE},
        {"import", KW_IMPORT}, {"export", KW_EXPORT},   {"macro", KW_MACRO},
        {"define", KW_DEFINE}, {"defined", KW_DEFINED},
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

    return Token(type, identifier, line, start_offset, position); // Use start_offset here
}

Token Tokenizer::read_string(SIZE position)
{
    std::string lexeme;
    SIZE        start_offset = offset; // Record starting offset of the string
    pos++;                             // Skip opening quote
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

    return Token(LIT_STRING, lexeme, line, start_offset, position); // Use start_offset here
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

    SIZE position = program.tokens->tokens.size();

    // Check if the position is at the end of the program.source string
    // If so, return an EOF token meant as a sentinel
    if (pos >= source_size()) {
        return {EOF_, "\0", line, offset, position};
    }

    SIZE start_offset = offset; // Record starting offset of each token

    // Handle numbers
    if (std::isdigit(peek())) {
        return read_number(position);
    }

    // Handle string literals
    if (peek() == '"') {
        return read_string(position);
    }

    // Handle identifiers and keywords
    if (std::isalpha(peek()) || peek() == '_') {
        return read_ident(position);
    }

    // Handle special characters (operators, delimiters, etc.)
    char chr = peek();

    pos++;
    offset++;

    switch (chr) {
    case '+':
        return Token(OP_ADD, "+", line, start_offset, position);
    case '-':
        return Token(OP_SUB, "-", line, start_offset, position);
    case '*':
        return Token(OP_MUL, "*", line, start_offset, position);
    case '/':
        return Token(OP_DIV, "/", line, start_offset, position);
    case '%':
        return Token(OP_MOD, "%", line, start_offset, position);
    case '^':
        return Token(OP_EXP, "^", line, start_offset, position);
    case '=':
        if (pos < source_size() && peek() == '=') {
            pos++;
            offset++;
            return Token(OP_EQ, "==", line, start_offset, position);
        }
        return Token(EQUAL, "=", line, start_offset, position);
    case '!':
        if (pos < source_size() && peek() == '=') {
            pos++;
            offset++;
            return Token(OP_NEQ, "!=", line, start_offset, position);
        }
        return Token(EXCLAMATION, "!", line, start_offset, position);
    case '<':
        return Token(OP_LT, "<", line, start_offset, position);
    case '>':
        return Token(OP_GT, ">", line, start_offset, position);
    case '&':
        return Token(AMPERSAND, "&", line, start_offset, position);
    case '|':
        return Token(PIPE, "|", line, start_offset, position);
    case ';':
        return Token(SEMICOLON, ";", line, start_offset, position);
    case ',':
        return Token(COMMA, ",", line, start_offset, position);
    case '(':
        return Token(PAREN_OPEN, "(", line, start_offset, position);
    case ')':
        return Token(PAREN_CLOSE, ")", line, start_offset, position);
    case '{':
        return Token(BRACE_OPEN, "{", line, start_offset, position);
    case '}':
        return Token(BRACE_CLOSE, "}", line, start_offset, position);
    case '[':
        return Token(BRACKET_OPEN, "[", line, start_offset, position);
    case ']':
        return Token(BRACKET_CLOSE, "]", line, start_offset, position);
    case '.':
        return Token(DOT, ".", line, start_offset, position);
    case ':':
        return Token(COLON, ":", line, start_offset, position);
    case '@':
        return Token(AT, "@", line, start_offset, position);
    case '?':
        return Token(QUESTION, "?", line, start_offset, position);
    default:
        return Token(UNKNOWN, std::string(1, chr), line, start_offset, position);
    }

    return Token(UNKNOWN, "\0", line, start_offset, position);
}

void Tokenizer::tokenize()
{
    TokenHolder *tokens = program.tokens;

    while (true) {
        Token token = get_token();
        tokens->tokens.push_back(token);

        if (token.type == EOF_) {
            break;
        }
    }
}

} // namespace via
