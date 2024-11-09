#include "common.h"
#include "lexer.h"

#define TOKEN(type, val, line, off) \
    *m_alloc.emplace<Token>(type, val, line, off) \

using namespace via::Tokenization;

Token Tokenizer::read_number() noexcept
{
    TokenType type = TokenType::LIT_INT;
    std::string value;
    size_t start_offset = offset;  // Record starting offset of the number

    while (pos < source.size() && isdigit(source.at(pos)))
    {
        value.push_back(source.at(pos));
        pos++;
        offset++;
    }

    // Check for floating point
    if (pos < source.size() && source.at(pos) == '.')
    {
        type = TokenType::LIT_FLOAT;
        value.push_back(source.at(pos));
        pos++;
        offset++;

        while (pos < source.size() && isdigit(source.at(pos)))
        {
            value.push_back(source.at(pos));
            pos++;
            offset++;
        }
    }

    return TOKEN(type, value, line, start_offset);  // Use start_offset here
}

Token Tokenizer::read_ident() noexcept
{
    std::string identifier;
    size_t start_offset = offset;  // Record starting offset of the identifier

    while (pos < source.size() && (isalnum(source.at(pos)) || source.at(pos) == '_'))
    {
        identifier.push_back(source.at(pos));
        pos++;
        offset++;
    }

    TokenType type = TokenType::IDENTIFIER;

    static const std::unordered_map<std::string, TokenType> keyword_map = {
        {"do", TokenType::KW_DO},
        {"in", TokenType::KW_IN},
        {"local", TokenType::KW_LOCAL},
        {"global", TokenType::KW_GLOBAL},
        {"as", TokenType::KW_AS},
        {"const", TokenType::KW_CONST},
        {"if", TokenType::KW_IF},
        {"else", TokenType::KW_ELSE},
        {"elseif", TokenType::KW_ELIF},
        {"while", TokenType::KW_WHILE},
        {"for", TokenType::KW_FOR},
        {"return", TokenType::KW_RETURN},
        {"func", TokenType::KW_FUNC},
        {"break", TokenType::KW_BREAK},
        {"continue", TokenType::KW_CONTINUE},
        {"switch", TokenType::KW_SWITCH},
        {"case", TokenType::KW_CASE},
        {"default", TokenType::KW_DEFAULT},
        {"delete", TokenType::KW_DELETE},
        {"new", TokenType::KW_NEW},
        {"and", TokenType::KW_AND},
        {"not", TokenType::KW_NOT},
        {"or", TokenType::KW_OR},
        {"struct", TokenType::KW_STRUCT},
        {"namespace", TokenType::KW_NAMESPACE},
        {"property", TokenType::KW_PROPERTY},
    };

    auto it = keyword_map.find(identifier);
    
    if (it != keyword_map.end()) {
        type = it->second;
    }

    if (identifier == "true" || identifier == "false") {
        type = TokenType::LIT_BOOL;
    }

    return TOKEN(type, identifier, line, start_offset);  // Use start_offset here
}

Token Tokenizer::read_string() noexcept
{
    std::string value;
    size_t start_offset = offset;  // Record starting offset of the string
    pos++; // Skip opening quote
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
                case 'n': value.push_back('\n'); break;
                case 't': value.push_back('\t'); break;
                case 'r': value.push_back('\r'); break;
                default: value.push_back(escape_char); break;
                }
            }
        }
        else
        {
            value.push_back(source.at(pos));
        }
        pos++;
        offset++;
    }

    pos++; // Skip closing quote
    offset++;

    return TOKEN(TokenType::LIT_STRING, value, line, start_offset);  // Use start_offset here
}

Token Tokenizer::get_token() noexcept
{
    while (pos < source.size() && isspace(source.at(pos)))
    {
        if (source.at(pos) == '\n')
        {
            line++;
            offset = 0;
        }
        else
        {
            offset++;
        }
        pos++;  // Ensure this increments regardless of newline
    }

    if (pos >= source.size())
    {
        return { TokenType::EOF_, "", line, offset };
    }

    size_t start_offset = offset;  // Record starting offset of each token

    if (isdigit(source.at(pos)))
    {
        return read_number();
    }

    if (isalpha(source.at(pos)) || source.at(pos) == '_')
    {
        return read_ident();
    }

    if (source.at(pos) == '"')
    {
        return read_string();
    }

    // Handle operators
    char ch = source.at(pos);

    switch (ch)
    {
    case '+': pos++; offset++; return TOKEN(TokenType::OP_ADD, "+", line, start_offset);
    case '-': pos++; offset++; return TOKEN(TokenType::OP_SUB, "-", line, start_offset);
    case '*': pos++; offset++; return TOKEN(TokenType::OP_MUL, "*", line, start_offset);
    case '/': pos++; offset++; return TOKEN(TokenType::OP_DIV, "/", line, start_offset);
    case '%': pos++; offset++; return TOKEN(TokenType::OP_MOD, "%", line, start_offset);
    case '^': pos++; offset++; return TOKEN(TokenType::OP_EXP, "^", line, start_offset);
    case '=':
        if (pos + 1 < source.size() && source.at(pos + 1) == '=')
        {
            pos += 2;
            offset += 2;
            return TOKEN(TokenType::OP_EQ, "==", line, start_offset);
        }

        pos++;
        offset++;

        return TOKEN(TokenType::OP_ASGN, "=", line, start_offset);
    case '!':
        if (pos + 1 < source.size() && source.at(pos + 1) == '=')
        {
            pos += 2;
            offset += 2;
            return TOKEN(TokenType::OP_NEQ, "!=", line, start_offset);
        }
        break;
    case '<': pos++; offset++; return TOKEN(TokenType::OP_LT, "<", line, start_offset);
    case '>': pos++; offset++; return TOKEN(TokenType::OP_GT, ">", line, start_offset);
    case '&': pos++; offset++; return TOKEN(TokenType::AMPERSAND, "&", line, start_offset);
    case '|': pos++; offset++; return TOKEN(TokenType::PIPE, "|", line, start_offset);
    case ';': pos++; offset++; return TOKEN(TokenType::SEMICOLON, ";", line, start_offset);
    case ',': pos++; offset++; return TOKEN(TokenType::COMMA, ",", line, start_offset);
    case '(': pos++; offset++; return TOKEN(TokenType::PAREN_OPEN, "(", line, start_offset);
    case ')': pos++; offset++; return TOKEN(TokenType::PAREN_CLOSE, ")", line, start_offset);
    case '{': pos++; offset++; return TOKEN(TokenType::BRACE_OPEN, "{", line, start_offset);
    case '}': pos++; offset++; return TOKEN(TokenType::BRACE_CLOSE, "}", line, start_offset);
    case '[': pos++; offset++; return TOKEN(TokenType::BRACKET_OPEN, "[", line, start_offset);
    case ']': pos++; offset++; return TOKEN(TokenType::BRACKET_CLOSE, "]", line, start_offset);
    case '.': pos++; offset++; return TOKEN(TokenType::DOT, ".", line, start_offset);
    case ':': pos++; offset++; return TOKEN(TokenType::COLON, ":", line, start_offset);
    case '@': pos++; offset++; return TOKEN(TokenType::AT, "@", line, start_offset);
    default: pos++; offset++; return { TokenType::UNKNOWN, std::string(1, ch), line, start_offset };
    }

    pos++;
    offset++;
    return { TokenType::UNKNOWN, "", line, start_offset };
}

viaSourceContainer Tokenizer::tokenize() noexcept
{
    std::vector<Token> tokens;

    while (true)
    {
        auto token = get_token();
        tokens.push_back(token);

        if (token.type == TokenType::EOF_)
        {
            break;
        }
    }

    return { tokens, source, "" };
}