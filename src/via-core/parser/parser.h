// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_PARSER_H_
#define VIA_CORE_PARSER_H_

#include "common.h"
#include "memory.h"
#include "common/diag.h"
#include "ast.h"
#include "lexer/token.h"
#include "lexer/lexer.h"
#include <mimalloc.h>

namespace via {

namespace core {

using AstBuf = Buffer<parser::ast::StmtNode*>;

namespace parser {

struct ParseState {
  const lex::LexState& L;

  lex::Token** cursor;
  HeapAllocator al;
  DiagContext& dctx;

  inline explicit ParseState(const lex::LexState& L, const TokenBuf& B, DiagContext& dctx)
    : L(L),
      cursor(B.data),
      dctx(dctx) {}
};

AstBuf parser_parse(ParseState& P);

void dump_ast(const AstBuf& B);

} // namespace parser

} // namespace core

} // namespace via

#endif
