// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_TOKEN_H_
#define VIA_CORE_TOKEN_H_

#include <fmt/ranges.h>
#include <via/config.h>
#include <via/types.h>
#include "buffer.h"
#include "convert.h"
#include "location.h"

namespace via {

struct Token {
  enum class Kind {
    EOF_ = 0,  // end of file
    ILLEGAL,   // unrecognized lexeme

    IDENT,  // identifier
    STRING,
    NIL,     // nil
    MIDENT,  // macro identifier
    INT,     // integer literal
    BINT,    // binary integer literal
    XINT,    // hexadecimal integer literal
    FP,      // floating point literal
    TRUE,    // true literal
    FALSE,   // false literal

    KW_VAR,    // var
    KW_MACRO,  // macro
    KW_FUNC,   // func
    KW_TYPE,   // type
    KW_WHILE,  // while
    KW_FOR,    // for
    KW_IF,     // if
    KW_IN,     // in
    KW_ELSE,   // else
    KW_DO,     // do
    KW_AND,    // and
    KW_OR,     // or
    KW_NOT,    // not
    KW_SHL,    // shl
    KW_SHR,    // shr

    DOT,                // .
    COMMA,              // ,
    SEMICOLON,          // ;
    COLON,              // :
    DBCOLON,            // ::
    ARROW,              // ->
    QUESTION,           // ?
    PLUS,               // +
    MINUS,              // -
    ASTERISK,           // *
    FSLASH,             // /
    POW,                // **
    PERCENT,            // %
    AMPERSAND,          // &
    TILDE,              // ~
    CARET,              // ^
    PIPE,               // |
    BANG,               // !
    INC,                // ++
    DEC,                // --
    LESSTHAN,           // <
    GREATERTHAN,        // >
    CONCAT,             // ..
    LPAREN,             // (
    RPAREN,             // )
    LBRACKET,           // [
    RBRACKET,           // ]
    LCURLY,             // {
    RCURLY,             // }
    EQUALS,             // =
    DBEQUALS,           // ==
    PLUSEQUALS,         // +=
    MINUSEQUALS,        // -=
    ASTERISKEQUALS,     // *=
    FSLASHEQUALS,       // /=
    POWEQUALS,          // **=
    PERCENTEQUALS,      // %=
    AMPERSANDEQUALS,    // &=
    CARETEQUALS,        // ^=
    PIPEEQUALS,         // |=
    BANGEQUALS,         // !=
    LESSTHANEQUALS,     // <=
    GREATERTHANEQUALS,  // >=
    CONCATEQUALS,       // ..=
  } kind;
  const char* lexeme;
  usize size;

  String to_string() const;
  String get_dump() const;

  AbsLocation location(const FileBuf& source) const;
};

using TokenBuf = Buffer<Token*>;

template <>
struct Convert<Token> {
  static String to_string(const Token& tok) {
    return fmt::format("[{:<12} '{}']",
                       Convert<Token::Kind>::to_string(tok.kind),
                       (*tok.lexeme == '\0') ? "<eof>" : tok.to_string());
  }
};

}  // namespace via

#endif
