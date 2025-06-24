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

using FileBuf = HeapBuffer<char>;

struct LexState {
  const FileBuf& file;
  ArenaAllocator ator{VIA_MAXLEXSIZE};

  inline LexState(const FileBuf& file)
    : file(file) {}
};

char advance(LexState* L);
char peek(LexState* L, int count);
TokenBuf tokenize(LexState* L);
void dump_ttree(const TokenBuf& B);

} // namespace via

#endif
