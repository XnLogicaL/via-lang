// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_PARSER_H
#define VIA_PARSER_H

#include "common.h"
#include "mem.h"
#include "error.h"
#include "ast.h"
#include <mimalloc.h>

namespace via {

struct ParseState {
  Token** cursor;
  HeapAllocator al;

  inline explicit ParseState(const TokenBuf& B)
    : cursor(B.data) {}
};

Token* parser_peek(ParseState& P, const int ahead = 0);
Token* parser_advance(ParseState& P);

bool parser_match(ParseState& P, const TokenKind kind);
bool parser_expect(ParseState& P, const TokenKind kind);

ExprNode* parse_expr(ParseState& P);
ExprNode* parse_primary(ParseState& P);
ExprNode* parse_unary(ParseState& P);
ExprNode* parse_binary(ParseState& P, int prec = 0);
ExprNode* parse_group(ParseState& P);
ExprNode* parse_postfix(ParseState& P, ExprNode* lhs);

StmtNode* parse_stmt(ParseState& P);
NodeStmtScope* parse_scope(ParseState& P);
NodeStmtIf* parse_if(ParseState& P);
NodeStmtWhile* parse_while(ParseState& P);
NodeStmtFor* parse_for(ParseState& P);
NodeStmtForEach* parse_foreach(ParseState& P);

// helpers
bool is_expr_start(TokenKind kind);
int bin_prec(TokenKind kind);

} // namespace via

#endif
