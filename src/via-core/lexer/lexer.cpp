/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "lexer.h"
#include <cstring>

using enum via::TokenKind;

// max 3-char symbol lookahead
struct TokenReprPair
{
    const char* str;
    via::TokenKind kind;
};

static constexpr TokenReprPair KEYWORDS[] = {
    {"var", KW_VAR},       {"const", KW_CONST},   {"fn", KW_FN},
    {"type", KW_TYPE},     {"while", KW_WHILE},   {"for", KW_FOR},
    {"if", KW_IF},         {"in", KW_IN},         {"of", KW_OF},
    {"else", KW_ELSE},     {"do", KW_DO},         {"and", KW_AND},
    {"or", KW_OR},         {"not", KW_NOT},       {"return", KW_RETURN},
    {"as", KW_AS},         {"import", KW_IMPORT}, {"mod", KW_MODULE},
    {"struct", KW_STRUCT}, {"enum", KW_ENUM},     {"using", KW_USING},
    {"bool", KW_BOOL},     {"int", KW_INT},       {"float", KW_FLOAT},
    {"string", KW_STRING},
};

static constexpr TokenReprPair OPERATORS[] = {
    {".", PERIOD},
    {",", COMMA},
    {";", SEMICOLON},
    {":", COLON},
    {"::", COLON_COLON},
    {"->", ARROW},
    {"?", QUESTION},
    {"+", OP_PLUS},
    {"-", OP_MINUS},
    {"*", OP_STAR},
    {"/", OP_SLASH},
    {"**", OP_STAR_STAR},
    {"%", OP_PERCENT},
    {"&", OP_AMP},
    {"~", OP_TILDE},
    {"^", OP_CARET},
    {"|", OP_PIPE},
    {"<<", OP_SHL},
    {">>", OP_SHR},
    {"!", OP_BANG},
    {"++", OP_PLUS_PLUS},
    {"--", OP_MINUS_MINUS},
    {"<", OP_LT},
    {">", OP_GT},
    {"..", OP_DOT_DOT},
    {"(", PAREN_OPEN},
    {")", PAREN_CLOSE},
    {"[", BRACKET_OPEN},
    {"]", BRACKET_CLOSE},
    {"{", BRACE_OPEN},
    {"}", BRACE_CLOSE},
    {"=", OP_EQ},
    {"==", OP_EQ_EQ},
    {"+=", OP_PLUS_EQ},
    {"*=", OP_STAR_EQ},
    {"/=", OP_SLASH_EQ},
    {"**=", OP_STAR_STAR_EQ},
    {"%=", OP_PERCENT_EQ},
    {"&=", OP_AMP_EQ},
    {"^=", OP_CARET_EQ},
    {"|=", OP_PIPE_EQ},
    {"<<=", OP_SHL_EQ},
    {">>=", OP_SHR_EQ},
    {"!=", OP_BANG_EQ},
    {"<=", OP_LT_EQ},
    {">=", OP_GT_EQ},
    {"..=", OP_DOT_DOT_EQ},
};

static consteval size_t string_length(const char* str)
{
    size_t len = 0;
    while (str[len] != '\0') {
        ++len;
    }

    return len;
}

static consteval size_t max_operator_length()
{
    size_t maxSize = 0;

    for (const auto& symbol: OPERATORS) {
        size_t size = string_length(symbol.str);
        if (size > maxSize) {
            maxSize = size;
        }
    }

    return maxSize;
}

static bool is_numeric(via::TokenKind* kind, char c)
{
    switch (*kind) {
    case LIT_INT:
        return isdigit(c) || (c == '.' && *kind != LIT_FLOAT); // decimal
    case LIT_XINT:
        return isxdigit(c); // hexadecimal
    case LIT_BINT:
        return c == '0' || c == '1'; // binary
    default:
        break;
    }

    return false;
}

static bool is_identifier_start(char c)
{
    return isalpha(c) || c == '_';
}

static bool is_identifier(char c)
{
    return isalnum(c) || c == '_';
}

static bool is_string_delim(char c)
{
    return c == '"' || c == '\'' || c == '`';
}

char via::Lexer::advance(size_t ahead)
{
    char c = *m_cursor;
    m_cursor += ahead;
    return m_cursor < m_end ? c : '\0';
}

char via::Lexer::peek(ssize_t ahead)
{
    return m_cursor + ahead < m_end ? *(m_cursor + ahead) : '\0';
}

via::Token* via::Lexer::read_number()
{
    Token* token = m_alloc.emplace<Token>();
    token->kind = LIT_INT;
    token->lexeme = m_cursor;
    token->size = 0;

    if (peek() == '0') {
        if (peek(1) == 'x')
            token->kind = LIT_XINT;
        else if (peek(1) == 'b')
            token->kind = LIT_BINT;
        else
            goto decimal;

        token->size = 2;
        advance(2); // 0b/0x
    }

decimal:
    char c;
    while ((c = peek()), is_numeric(&token->kind, c)) {
        if (c == '.') {
            if (token->kind == LIT_INT)
                token->kind = LIT_FLOAT;
            else {
                token->kind = ILLEGAL;
                break;
            }
        }

        advance();
        token->size++;
    }

    return token;
}

via::Token* via::Lexer::read_string()
{
    Token* token = m_alloc.emplace<Token>();
    token->kind = LIT_STRING;
    token->lexeme = m_cursor;

    char del = advance();
    token->size = 1;

    char c;
    bool closed = false;
    while ((c = advance()) != '\0') {
        token->size++;

        if (c == '\\') {
            if (peek() != '\0') {
                advance();
                token->size++;
            }
        } else if (c == del) {
            closed = true;
            break;
        }
    }

    if (!closed) {
        token->size = 1;
        token->kind = ILLEGAL;
    }

    return token;
}

via::Token* via::Lexer::read_identifier()
{
    Token* token = m_alloc.emplace<Token>();
    token->kind = IDENTIFIER;
    token->lexeme = m_cursor;
    token->size = 0;

    char c;
    while ((c = peek()), is_identifier(c)) {
        advance();
        token->size++;
    }

    for (const auto& kw: KEYWORDS) {
        if (strlen(kw.str) != token->size)
            continue;

        if (strncmp(kw.str, token->lexeme, token->size) == 0) {
            token->kind = kw.kind;
            break;
        }
    }

    if (token->size == strlen("nil") && strncmp(token->lexeme, "nil", token->size) == 0)
        token->kind = LIT_NIL;
    else if (token->size == strlen("true") &&
             strncmp(token->lexeme, "true", token->size) == 0)
        token->kind = LIT_TRUE;
    else if (token->size == strlen("false") &&
             strncmp(token->lexeme, "false", token->size) == 0)
        token->kind = LIT_FALSE;

    return token;
}

via::Token* via::Lexer::read_operator()
{
    Token* token = m_alloc.emplace<Token>();
    token->lexeme = m_cursor;
    token->kind = ILLEGAL;
    token->size = 1;

    size_t match_size = 0;
    auto match_kind = ILLEGAL;

    char buf[4] = {};

    for (size_t len = max_operator_length(); len >= 1; --len) {
        for (size_t i = 0; i < len; ++i) {
            buf[i] = m_cursor[i];
        }

        buf[len] = '\0';

        for (const auto& symbol: OPERATORS) {
            if (len == strlen(symbol.str) && strcmp(buf, symbol.str) == 0) {
                match_kind = symbol.kind;
                match_size = len;
                goto found;
            }
        }
    }

found:
    if (match_kind != ILLEGAL) {
        token->kind = match_kind;
        token->size = match_size;

        for (size_t i = 0; i < match_size; ++i) {
            advance();
        }
    } else {
        advance(); // advance one char if no match
    }

    return token;
}

bool via::Lexer::skip_comment()
{
    if (peek() != '/') {
        return false;
    }

    char next = peek(1);
    if (next == '/') {
        advance(2); // consume first '//'

        while (char c = peek()) {
            if (c == '\n' || c == '\0')
                break;

            advance();
        }

        return true;
    }

    if (next == '*') {
        advance(2); // consume '/*'

        while (true) {
            char c = peek();
            if (c == '\0')
                break; // EOF without closing */

            if (c == '*' && peek(1) == '/') {
                advance(2); // consume '*/'
                break;
            }

            advance();
        }

        return true;
    }

    return false;
}

via::TokenTree via::Lexer::tokenize()
{
    TokenTree toks;

    char c;
    while ((c = peek()), c != '\0') {
        if (isspace(c)) {
            advance();
            continue;
        }

        if (skip_comment())
            continue;

        Token* token;

        if (std::isdigit(c))
            token = read_number();
        else if (is_identifier_start(c))
            token = read_identifier();
        else if (is_string_delim(c))
            token = read_string();
        else
            token = read_operator();

        toks.push_back(token);
    }

    Token* eof = m_alloc.emplace<Token>();
    eof->kind = EOF_;
    eof->lexeme = m_cursor;
    eof->size = 0;

    toks.push_back(eof);
    return toks;
}

[[nodiscard]] std::string via::debug::to_string(const via::TokenTree& tt)
{
    std::ostringstream oss;
    for (const auto* tk: tt) {
        oss << tk->to_string() << "\n";
    }

    return oss.str();
}
