// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_LEXER_H_
#define VIA_CORE_LEXER_H_

#include <via/config.h>
#include <util/memory.h>
#include <util/buffer.h>
#include "token.h"
#include <mimalloc.h>

namespace via {

namespace core {

namespace lex {

using FileBuf = Buffer<char>;

struct LexState {
  const FileBuf& file;
  HeapAllocator al;

  inline LexState(const FileBuf& file)
    : file(file) {}
};

TokenBuf lexer_tokenize(LexState& L);
void dump_ttree(const TokenBuf& B);

} // namespace lex

} // namespace core

} // namespace via

#endif
