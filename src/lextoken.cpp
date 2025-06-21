// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "lextoken.h"
#include "lexstate.h"

namespace via {

namespace lex {

Location location(State* L, Token* token) {
  const size_t pos = token->lexeme - L->file.data;
  return {pos, pos + token->size};
}

} // namespace lex

} // namespace via
