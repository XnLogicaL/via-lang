//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================
#include "lexer.h"
#include "token.h"

namespace via {

using enum token_type;

// Function that returns whether if a character is allowed within a hexadecimal literal
bool lexer::is_hex_char(char chr) {
  return (chr >= 'A' && chr <= 'F') || (chr >= 'a' && chr <= 'f');
}

size_t lexer::source_size() {
  return unit_ctx.file_source.size();
}

char lexer::peek(size_t ahead) {
  if (pos + ahead >= source_size()) {
    return '\0';
  }

  return unit_ctx.file_source.at(pos + ahead);
}

char lexer::consume(size_t ahead) {
  if (pos + ahead >= source_size()) {
    return '\0';
  }

  return unit_ctx.file_source.at(pos += ahead);
}

#if VIA_COMPILER == C_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

token lexer::read_number(size_t position) {
  token_type type = LIT_INT;
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

  return token(type, value, line, start_offset, position);
}

#if VIA_COMPILER == C_GCC
#pragma GCC diagnostic pop
#endif

token lexer::read_ident(size_t position) {
  // List of allowed special characters that can be included in an identifier
  static const std::vector<char> allowed_identifier_spec_chars = {'_', '!'};

  // Default type, this is because this might be an identifier, keyword or boolean literal
  // We can't know in advance which.
  token_type type = IDENTIFIER;
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

  static const std::unordered_map<std::string, token_type> keyword_map = {
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
    {"raise", KW_RAISE},     {"trait", KW_TRAIT}
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

  return token(type, identifier, line, start_offset, position);
}

token lexer::read_string(size_t position) {
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

  return token(LIT_STRING, lexeme, line, start_offset, position);
}

token lexer::get_token() {
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
  // If so, return an EOF token meant as a sentinel
  if (pos >= source_size()) {
    return {EOF_, "\0", line, offset, position};
  }

  size_t start_offset = offset; // Record starting offset of each token

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
      return token(OP_INCREMENT, "++", line, start_offset, position);
    }
    return token(OP_ADD, "+", line, start_offset, position);
  case '-':
    if (pos < source_size() && peek() == '>') {
      pos++;
      offset++;
      return token(RETURNS, "->", line, start_offset, position);
    }
    else if (pos < source_size() && peek() == '-') {
      pos++;
      offset++;
      return token(OP_DECREMENT, "--", line, start_offset, position);
    }
    return token(OP_SUB, "-", line, start_offset, position);
  case '*':
    return token(OP_MUL, "*", line, start_offset, position);
  case '/':
    return token(OP_DIV, "/", line, start_offset, position);
  case '%':
    return token(OP_MOD, "%", line, start_offset, position);
  case '^':
    return token(OP_EXP, "^", line, start_offset, position);
  case '=':
    if (pos < source_size() && peek() == '=') {
      pos++;
      offset++;
      return token(OP_EQ, "==", line, start_offset, position);
    }
    return token(EQ, "=", line, start_offset, position);
  case '!':
    if (pos < source_size() && peek() == '=') {
      pos++;
      offset++;
      return token(OP_NEQ, "!=", line, start_offset, position);
    }
    return token(EXCLAMATION, "!", line, start_offset, position);
  case '<':
    return token(OP_LT, "<", line, start_offset, position);
  case '>':
    return token(OP_GT, ">", line, start_offset, position);
  case '&':
    return token(AMPERSAND, "&", line, start_offset, position);
  case '|':
    return token(PIPE, "|", line, start_offset, position);
  case ';':
    return token(SEMICOLON, ";", line, start_offset, position);
  case ',':
    return token(COMMA, ",", line, start_offset, position);
  case '(':
    return token(PAREN_OPEN, "(", line, start_offset, position);
  case ')':
    return token(PAREN_CLOSE, ")", line, start_offset, position);
  case '{':
    return token(BRACE_OPEN, "{", line, start_offset, position);
  case '}':
    return token(BRACE_CLOSE, "}", line, start_offset, position);
  case '[':
    return token(BRACKET_OPEN, "[", line, start_offset, position);
  case ']':
    return token(BRACKET_CLOSE, "]", line, start_offset, position);
  case '.':
    return token(DOT, ".", line, start_offset, position);
  case ':':
    return token(COLON, ":", line, start_offset, position);
  case '@':
    return token(AT, "@", line, start_offset, position);
  case '?':
    return token(QUESTION, "?", line, start_offset, position);
  default:
    return token(UNKNOWN, std::string(1, chr), line, start_offset, position);
  }

  return token(UNKNOWN, "\0", line, start_offset, position);
}

void lexer::tokenize() {
  auto& tokens = unit_ctx.tokens;

  while (true) {
    token token = get_token();
    tokens->push(token);

    if (token.type == EOF_) {
      break;
    }
  }
}

} // namespace via
