// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_PARSER_H
#define VIA_PARSER_H

#include "common.h"
#include "mem.h"
#include "diag.h"
#include "error.h"
#include "ast.h"
#include <mimalloc.h>

namespace via {

using AstBuf = HeapBuffer<StmtNode*>;

struct ParseState {
  const LexState& L;

  Token** cursor;
  HeapAllocator al;
  DiagContext& dctx;

  inline explicit ParseState(const LexState& L, const TokenBuf& B, DiagContext& dctx)
    : L(L),
      cursor(B.data),
      dctx(dctx) {}
};

AstBuf parser_parse(ParseState& P);

} // namespace via

#endif
