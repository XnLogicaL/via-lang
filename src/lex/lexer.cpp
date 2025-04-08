//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================
#include "Lexer.h"
#include "token.h"

namespace via {

using enum TokenType;

// IFunction that returns whether if a character is allowed within a hexadecimal literal
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
  while (pos < source_size() && (std::isdigit(peek()) || (type == LIT_HEX && is_hex_char(peek())))
  ) {
    value.push_back(peek());
    pos++;
    offset++;
  }

  // Check for floating point
  if (pos < source_size() && peek() == '.') {
    value.push_back(peek());
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
    value = std::format("0{}{}", delimiter, value);
  }

  return Token(type, value, line, start_offset, position);
}

#if VIA_COMPILER == C_GCC
#pragma GCC diagnostic pop
#endif

Token Lexer::read_ident(size_t position) {
  // List of allowed special characters that can be included in an identifier
  static const std::vector<char> allowed_identifier_spec_chars = {'_', '!'};

  // Default type, this is because this might be an identifier, keyword or boolean literal
  // We can't know in advance which.
  TokenType type = IDENTIFIER;
  size_t start_offset = offset;
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
    {"do", KW_DO},           {"in", KW_IN},         {"var", KW_LOCAL},
    {"glb", KW_GLOBAL},      {"as", KW_AS},         {"const", KW_CONST},
    {"if", KW_IF},           {"else", KW_ELSE},     {"elif", KW_ELIF},
    {"while", KW_WHILE},     {"for", KW_FOR},       {"return", KW_RETURN},
    {"fn", KW_FUNC},         {"break", KW_BREAK},   {"continue", KW_CONTINUE},
    {"switch", KW_MATCH},    {"case", KW_CASE},     {"default", KW_DEFAULT},
    {"new", KW_NEW},         {"and", KW_AND},       {"not", KW_NOT},
    {"or", KW_OR},           {"struct", KW_STRUCT}, {"import", KW_IMPORT},
    {"export", KW_EXPORT},   {"macro", KW_MACRO},   {"define", KW_DEFINE},
    {"defined", KW_DEFINED}, {"type", KW_TYPE},     {"pragma", KW_PRAGMA},
    {"enum", KW_ENUM},       {"try", KW_TRY},       {"catch", KW_CATCH},
    {"raise", KW_RAISE},     {"trait", KW_TRAIT},   {"auto", KW_AUTO},
  };

  // Checks if the identifier is a keyword or not
  auto it = keyword_map.find(identifier);
  if (it != keyword_map.end()) {
    type = it->second;
  }

  // Checks if the identifier is a boolean literal
  if (identifier == "true" || identifier == "false") {
    type = LIT_BOOL;
  }

  if (identifier == "nil") {
    type = LIT_NIL;
  }

  return Token(type, identifier, line, start_offset, position);
}

Token Lexer::read_string(size_t position) {
  std::string lexeme;
  size_t start_offset = offset;

  pos++; // Skip opening quote
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

  return Token(LIT_STRING, lexeme, line, start_offset, position);
}

Token Lexer::get_token() {
  while (pos < source_size()) {
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

    break;
  }

  size_t position = pos;

  // Check if the position is at the end of the unit_ctx.file_source string
  // If so, return an EOF Token meant as a sentinel
  if (pos >= source_size()) {
    return {EOF_, "\0", line, offset, position};
  }

  size_t start_offset = offset; // Record starting offset of each Token

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
    if (pos < source_size() && peek() == '+') {
      pos++;
      offset++;
      return Token(OP_INCREMENT, "++", line, start_offset, position);
    }
    return Token(OP_ADD, "+", line, start_offset, position);
  case '-':
    if (pos < source_size() && peek() == '>') {
      pos++;
      offset++;
      return Token(RETURNS, "->", line, start_offset, position);
    }
    else if (pos < source_size() && peek() == '-') {
      pos++;
      offset++;
      return Token(OP_DECREMENT, "--", line, start_offset, position);
    }
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
    return Token(EQ, "=", line, start_offset, position);
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

void Lexer::tokenize() {
  auto& tokens = unit_ctx.tokens;

  while (true) {
    Token Token = get_token();
    tokens->push(Token);

    if (Token.type == EOF_) {
      break;
    }
  }
}

} // namespace via
