// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_LEXSTATE_H
#define VIA_LEXSTATE_H

#include "common.h"
#include "error.h"
#include "mem.h"
#include "heapbuf.h"
#include "lextoken.h"
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

// Advances the file cursor by one and returns the character it started on.
char lexer_advance(LexState& L);

// Returns the character at a given offset from the file cursor.
char lexer_peek(const LexState& L, int count);

// Tokenizes `LexState::file` and returns it as a buffer of token pointers owned by LexState.
TokenBuf lexer_tokenize(LexState& L);

// Dumps the given token buffer into standard output.
void dump_ttree(const TokenBuf& B);

} // namespace via

#endif
