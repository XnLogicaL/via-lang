// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_LEXSTATE_H
#define VIA_LEXSTATE_H

#include "common.h"
#include "error.h"
#include "mem.h"
#include "heapbuf.h"
#include "token.h"
#include <mimalloc.h>

namespace via {

// Null terminated buffer of characters.
using FileBuf = HeapBuffer<char>;

// Lexical analysis state.
struct LexState {
  const FileBuf& file;
  HeapAllocator al;

  inline LexState(const FileBuf& file)
    : file(file) {}
};

// Tokenizes `LexState::file` and returns it as a buffer of token pointers owned by LexState.
TokenBuf lexer_tokenize(LexState& L);

// Dumps the given token buffer into standard output.
void dump_ttree(const TokenBuf& B);

} // namespace via

#endif
