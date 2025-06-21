// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_LEXSTATE_H
#define VIA_LEXSTATE_H

#include "common.h"
#include "lextoken.h"
#include "heapbuf.h"
#include <arena/arena.h>

namespace via {

namespace lex {

struct State {
  const char* cursor = NULL;
  HeapBuffer<char> file;
  ArenaAllocator tok_ator; // Token allocator
};

char advance(State* L);
char peek(State* L, int count);
TokenBuf lex(State* L);

} // namespace lex

} // namespace via

#endif
