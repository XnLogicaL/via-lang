// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_PARSER_H
#define VIA_PARSER_H

#include "common.h"
#include "error.h"
#include "ast.h"
#include <arena/arena.h>

#define VIA_MAXAST (4096 * 40) // [max node count] * [average size]

namespace via {

struct ParseState {
  Token** cur;
  ArenaAllocator al;

  inline explicit ParseState(const TokenBuf& B)
    : cur(B.data),
      al(VIA_MAXAST) {
    al.register_handler([]() { error_fatal("out of memory: parsing aborted"); });
  }
};

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
NodeStmtForEach* parse_foreach(ParseState& P);
NodeStmtFor* parse_for(ParseState& P);

// helpers
bool is_expr_start(TokenKind kind);
int bin_prec(TokenKind kind);

} // namespace via

#endif
