// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "lexer.h"

#define TOKEN(a, b) Token(a, b, line, start_offset, position)
#define IN_RANGE()  pos < source_size()
#define NEXT_CHAR()                                                                                \
  {                                                                                                \
    offset++;                                                                                      \
    pos++;                                                                                         \
  }

namespace via {

using enum TokenType;

// Function that returns whether if a character is allowed within a hexadecimal literal
bool Lexer::is_hex_char(char chr) {
  return (chr >= 'A' && chr <= 'F') || (chr >= 'a' && chr <= 'f');
}

size_t Lexer::source_size() {
  return unit_ctx.file_source.size();
}

char Lexer::peek(size_t ahead) {
  if (pos + ahead >= source_size()) {
    return '\0';
  }

  return unit_ctx.file_source.at(pos + ahead);
}

char Lexer::consume(size_t ahead) {
  if (pos + ahead >= source_size()) {
    return '\0';
  }

  return unit_ctx.file_source.at(pos += ahead);
}

#if VIA_COMPILER == C_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

Token Lexer::read_number(size_t position) {
  TokenType type = LIT_INT;
  size_t start_offset = offset;
  std::string value;
  char delimiter;

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
  while (IN_RANGE() && (std::isdigit(peek()) || (type == LIT_HEX && is_hex_char(peek())))) {
    value.push_back(peek());
    NEXT_CHAR();
  }

  // Check for floating point
  if (IN_RANGE() && peek() == '.') {
    value.push_back(peek());
    type = LIT_FLOAT;

    NEXT_CHAR();

    while (IN_RANGE() && std::isdigit(peek())) {
      value.push_back(peek());
      pos++;
      offset++;
    }
  }

  if (type == LIT_HEX || type == LIT_BINARY) {
    value = std::format("0{}{}", delimiter, value);
  }

  return TOKEN(type, value);
}

#if VIA_COMPILER == C_GCC
#pragma GCC diagnostic pop
#endif

Token Lexer::read_ident(size_t position) {
  // List of allowed special characters that can be included in an identifier
  static const std::vector<char> allowed_identifier_spec_chars = {'_', '!'};

  // Default type, this is because this might be an identifier, keyword or Bool literal
  // We can't know in advance which.
  TokenType type = IDENTIFIER;
  size_t start_offset = offset;
  std::string lexeme;

  // Lambda for checking if a character is allowed within an identifier
  auto is_allowed = [this](char chr) -> bool {
    auto allow_list = allowed_identifier_spec_chars;
    bool is_alnum = isalnum(chr);
    bool is_allowed = std::ranges::find(allow_list, peek()) != allow_list.end();
    return is_alnum || is_allowed;
  };

  // Read identifier while position is inside bounds and the current character is allowed within
  // an identifier
  while (IN_RANGE() && is_allowed(peek())) {
    lexeme.push_back(peek());
    NEXT_CHAR();
  }

  static const std::unordered_map<std::string, TokenType> keyword_map = {
    {"do", KW_DO},           {"in", KW_IN},         {"var", KW_LOCAL},
    {"glb", KW_GLOBAL},      {"as", KW_AS},         {"const", KW_CONST},
    {"if", KW_IF},           {"else", KW_ELSE},     {"elseif", KW_ELIF},
    {"while", KW_WHILE},     {"for", KW_FOR},       {"return", KW_RETURN},
    {"fn", KW_FUNC},         {"break", KW_BREAK},   {"continue", KW_CONTINUE},
    {"match", KW_MATCH},     {"case", KW_CASE},     {"default", KW_DEFAULT},
    {"new", KW_NEW},         {"and", KW_AND},       {"not", KW_NOT},
    {"or", KW_OR},           {"struct", KW_STRUCT}, {"import", KW_IMPORT},
    {"export", KW_EXPORT},   {"macro", KW_MACRO},   {"define", KW_DEFINE},
    {"defined", KW_DEFINED}, {"type", KW_TYPE},     {"pragma", KW_PRAGMA},
    {"enum", KW_ENUM},       {"try", KW_TRY},       {"catch", KW_CATCH},
    {"raise", KW_RAISE},     {"trait", KW_TRAIT},   {"auto", KW_AUTO},
    {"defer", KW_DEFER},     {"typeof", KW_TYPEOF}, {"nameof", KW_NAMEOF},
  };

  // Checks if the identifier is a keyword or not
  auto it = keyword_map.find(lexeme);
  if (it != keyword_map.end()) {
    type = it->second;
  }

  // Checks if the identifier is a Bool literal
  if (lexeme == "true" || lexeme == "false") {
    type = LIT_BOOL;
  }

  if (lexeme == "nil") {
    type = LIT_NIL;
  }

  return TOKEN(type, lexeme);
}

Token Lexer::read_string(size_t position) {
  std::string lexeme;
  size_t start_offset = offset;

  NEXT_CHAR(); // Skip opening quote

  while (IN_RANGE() && peek() != '"') {
    if (peek() == '\\') {
      NEXT_CHAR();

      if (IN_RANGE()) {
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

    NEXT_CHAR();
  }

  NEXT_CHAR(); // Skip closing quote

  return TOKEN(LIT_STRING, lexeme);
}

Token Lexer::get_token() {
  while (IN_RANGE()) {
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
      NEXT_CHAR(); // Skip '#'
      NEXT_CHAR(); // Skip '#'

      while (IN_RANGE() && peek() != '\n') {
        NEXT_CHAR();
      }

      continue;
    }

    // Skip block comments
    if (peek() == '#' && pos + 1 < source_size() && peek(1) == '[') {
      NEXT_CHAR(); // Skip '#'
      NEXT_CHAR(); // Skip '['

      while (pos + 1 < source_size() && !(peek() == ']' && peek(1) == '#')) {
        if (peek() == '\n') {
          line++;
          offset = 0;
        }

        NEXT_CHAR();
      }

      if (pos + 1 < source_size()) {
        NEXT_CHAR(); // Skip ']'
        NEXT_CHAR(); // Skip '#'
      }

      continue;
    }

    break;
  }

  size_t position = pos;

  // Check if the position is at the end of the unit_ctx.file_source String
  // If so, return an EOF Token meant as a sentinel
  if (pos >= source_size()) {
    return Token(EOF_, "\0", line, offset, position);
  }

  size_t start_offset = offset; // Record starting offset of each Token

  // Handle numbers
  if (std::isdigit(peek())) {
    return read_number(position);
  }

  // Handle String literals
  if (peek() == '"') {
    return read_string(position);
  }

  // Handle identifiers and keywords
  if (std::isalpha(peek()) || peek() == '_') {
    return read_ident(position);
  }

  // Handle special characters (operators, delimiters, etc.)
  char chr = peek();

  NEXT_CHAR();

  switch (chr) {
  case '+':
    if (IN_RANGE() && peek() == '+') {
      NEXT_CHAR();
      return TOKEN(OP_INC, "++");
    }

    return TOKEN(OP_ADD, "+");
  case '-':
    if (IN_RANGE() && peek() == '>') {
      pos++;
      offset++;
      return TOKEN(RETURNS, "->");
    }
    else if (IN_RANGE() && peek() == '-') {
      pos++;
      offset++;
      return TOKEN(OP_DEC, "--");
    }

    return TOKEN(OP_SUB, "-");
  case '*':
    return TOKEN(OP_MUL, "*");
  case '/':
    return TOKEN(OP_DIV, "/");
  case '%':
    return TOKEN(OP_MOD, "%");
  case '^':
    return TOKEN(OP_EXP, "^");
  case '=':
    if (IN_RANGE() && peek() == '=') {
      NEXT_CHAR();
      return TOKEN(OP_EQ, "==");
    }

    return TOKEN(EQ, "=");
  case '!':
    if (IN_RANGE() && peek() == '=') {
      NEXT_CHAR();
      return TOKEN(OP_NEQ, "!=");
    }

    return TOKEN(EXCLAMATION, "!");
  case '<':
    if (IN_RANGE() && peek() == '>') {
      NEXT_CHAR();
      return TOKEN(OP_GEQ, ">=");
    }
    else if (IN_RANGE() && peek() == '<') {
      NEXT_CHAR();
      return TOKEN(OP_LEQ, "<=");
    }

    return TOKEN(OP_LT, "<");
  case '>':
    return TOKEN(OP_GT, ">");
  case '&':
    return TOKEN(AMPERSAND, "&");
  case '|':
    return TOKEN(PIPE, "|");
  case ';':
    return TOKEN(SEMICOLON, ";");
  case ',':
    return TOKEN(COMMA, ",");
  case '(':
    return TOKEN(PAREN_OPEN, "(");
  case ')':
    return TOKEN(PAREN_CLOSE, ")");
  case '{':
    return TOKEN(BRACE_OPEN, "{");
  case '}':
    return TOKEN(BRACE_CLOSE, "}");
  case '[':
    return TOKEN(BRACKET_OPEN, "[");
  case ']':
    return TOKEN(BRACKET_CLOSE, "]");
  case '.':
    return TOKEN(DOT, ".");
  case ':':
    return TOKEN(COLON, ":");
  case '@':
    return TOKEN(AT, "@");
  case '?':
    return TOKEN(QUESTION, "?");
  case '#':
    return TOKEN(OP_LEN, "#");
  default:
    return TOKEN(UNKNOWN, std::string(1, chr));
  }

  return TOKEN(UNKNOWN, "\0");
}

void Lexer::tokenize() {
  while (true) {
    Token tok = get_token();
    unit_ctx.tokens.push_back(tok);

    if (tok.type == EOF_) {
      break;
    }
  }
}

} // namespace via
