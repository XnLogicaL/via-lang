// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef vl_has_header_token_h
#define vl_has_header_token_h

#include "common-defs.h"

namespace via {

enum class token_type {
  // Keywords
  KW_DO,       // do
  KW_IN,       // in
  KW_LOCAL,    // let
  KW_GLOBAL,   // glb
  KW_AS,       // as
  KW_IF,       // if
  KW_ELSE,     // else
  KW_ELIF,     // elif
  KW_WHILE,    // while
  KW_FOR,      // for
  KW_RETURN,   // return
  KW_FUNC,     // fn
  KW_CONST,    // const
  KW_NEW,      // new
  KW_BREAK,    // break
  KW_CONTINUE, // continue
  KW_MATCH,    // switch
  KW_CASE,     // case
  KW_DEFAULT,  // default
  KW_AND,      // and
  KW_NOT,      // not
  KW_OR,       // or
  KW_STRUCT,   // struct
  KW_IMPORT,   // import
  KW_EXPORT,   // export
  KW_MACRO,    // macro
  KW_DEFINE,   // define
  KW_TYPE,     // type
  KW_TRAIT,    // trait
  KW_DEFINED,  // defined
  KW_PRAGMA,   // pragma
  KW_ENUM,     // enum
  KW_TRY,      // try
  KW_CATCH,    // catch
  KW_RAISE,    // raise

  // Operators
  OP_ADD,       // +
  OP_SUB,       // -
  OP_MUL,       // *
  OP_DIV,       // /
  OP_EXP,       // ^
  OP_MOD,       // %
  OP_EQ,        // ==
  OP_NEQ,       // !=
  OP_LT,        // <
  OP_GT,        // >
  OP_LEQ,       // <=
  OP_GEQ,       // >=
  OP_INCREMENT, // ++
  OP_DECREMENT, // --
  // Pseudo-operators
  EQUAL,   // =
  RETURNS, // ->

  // Literals
  LIT_INT,    // Integer literals
  LIT_FLOAT,  // Floating-point literals
  LIT_HEX,    // Hexadecimal number literals
  LIT_BINARY, // Binary number literals
  LIT_STRING, // String literals
  LIT_BOOL,   // Boolean literals
  LIT_NIL,    // Nil literal (just "nil" lol)

  // Identifiers
  IDENTIFIER, // Variable and function names

  // Punctuation
  PAREN_OPEN,    // (
  PAREN_CLOSE,   // )
  BRACE_OPEN,    // {
  BRACE_CLOSE,   // }
  BRACKET_OPEN,  // [
  BRACKET_CLOSE, // ]
  COMMA,         // ,
  SEMICOLON,     // ;
  COLON,         // :
  DOT,           // .

  // Miscellaneous
  AT,           // @
  TILDE,        // ~
  QUOTE,        // '
  PIPE,         // |
  DOLLAR,       // $
  BACKTICK,     // `
  AMPERSAND,    // &
  DOUBLE_QUOTE, // "
  EXCLAMATION,  // !
  QUESTION,     // ?

  EOF_,   // End of file
  UNKNOWN // Unknown token
};

using token_lexeme_t = std::string;

struct token {
  token_type type = token_type::UNKNOWN;
  uint64_t line = 0;
  uint64_t offset = 0;
  uint64_t position = 0;
  token_lexeme_t lexeme = "";

  token() = default;
  token(token_type type, std::string lexeme, uint64_t line, uint64_t offset, uint64_t position)
    : type(type),
      line(line),
      offset(offset),
      position(position),
      lexeme(lexeme) {}

  std::string to_string() const;

  bool is_literal() const;

  bool is_operator() const;

  bool is_modifier() const;

  int bin_prec() const;
};

class token_stream {
public:
  using token_vector = std::vector<token>;
  using at_result = std::optional<token>;

  size_t size();

  token& at(size_t);

  void push(const token&);

  token_vector& get();

private:
  token_vector tokens;
};

} // namespace via

#endif
