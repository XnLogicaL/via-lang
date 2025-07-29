// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_PARSER_H
#define VIA_PARSER_H

#include "common.h"
#include "memory.h"
#include "diag.h"
#include "ast.h"
#include "lexer/token.h"
#include "lexer/lexer.h"
#include <mimalloc.h>

namespace via {

using AstBuf = Buffer<StmtNode*>;

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

void dump_ast(const AstBuf& B);

} // namespace via

#endif
