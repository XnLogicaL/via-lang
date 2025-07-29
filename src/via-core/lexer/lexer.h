// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_LEXSTATE_H
#define VIA_LEXSTATE_H

#include "common.h"
#include "memory.h"
#include "buffer.h"
#include <lexer/token.h>
#include <mimalloc.h>

namespace via {

using FileBuf = Buffer<char>;

struct LexState {
  const FileBuf& file;
  HeapAllocator al;

  inline LexState(const FileBuf& file)
    : file(file) {}
};

TokenBuf lexer_tokenize(LexState& L);

void dump_ttree(const TokenBuf& B);

} // namespace via

#endif
