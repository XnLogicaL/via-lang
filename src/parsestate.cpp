// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "parsestate.h"

namespace via {

Token* parser_peek(ParseState& P, const int ahead) {
  return P.cursor[ahead];
}

Token* parser_advance(ParseState& P) {
  P.cursor++;
  return parser_peek(P, -1);
}

bool parser_match(ParseState& P, const TokenKind kind) {
  return parser_peek(P)->kind == kind;
}



} // namespace via
