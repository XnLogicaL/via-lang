// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_LEXSTATE_H
#define VIA_LEXSTATE_H

#include <common/common.h>
#include <common/heapbuf.h>
#include "lextoken.h"
#include <arena/arena.h>

#define VIA_MAXLEXSIZE 1024 * 1024 * 8

namespace via {

namespace lex {

using FileBuf = HeapBuffer<char>;

struct State {
  const FileBuf& file;
  ArenaAllocator ator{VIA_MAXLEXSIZE};

  inline State(const FileBuf& file)
    : file(file) {}
};

char advance(State* L);
char peek(State* L, int count);
TokenBuf lex(State* L);

void dump_ttree(State* L);

} // namespace lex

} // namespace via

#endif
