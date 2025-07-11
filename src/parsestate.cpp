// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "parsestate.h"

namespace via {

Token* parser_peek(ParseState& P, const int ahead) {
  return P.cursor[ahead];
}

Token* parser_advance(ParseState& P) {
  P.cursor++;
  return parser_peek(P, -1);
}

bool parser_match(ParseState& P, const TokenKind kind) {
  Token* tok = parser_peek(P);
  return tok->kind == kind || tok->kind == TK_EOF;
}

bool parser_expect(ParseState& P, const TokenKind kind) {
  if (!parser_match(P, kind)) {
    const Token& unexp = *parser_peek(P);
    const AbsLocation loc = token_abs_location(P.L, unexp);

    diagf<DK_ERROR>(P.dctx, loc, "Unexpected token '{}'", String(unexp.lexeme, unexp.size));
    return false;
  }

  return true;
}

ExprNode* parse_primary(ParseState& P) {
  Token* tok = parser_peek(P);
  AbsLocation loc = token_abs_location(P.L, *tok);

  if (parser_match(P, TK_INT)) {
    parser_advance(P); // consume token
    auto* lit = heap_emplace<NodeExprLit>(P.al);
    lit->tok = tok;
    lit->loc = loc;
    return lit;
  }

  if (parser_match(P, TK_IDENT)) {
    parser_advance(P); // consume token
    auto* sym = heap_emplace<NodeExprSym>(P.al);
    sym->tok = tok;
    sym->loc = loc;
    return sym;
  }

  if (parser_match(P, TK_LPAREN)) {
    parser_advance(P); // consume '('
    AbsLocation start = loc;
    ExprNode* first = parse_expr(P);

    if (parser_match(P, TK_COMMA)) {
      // Tuple
      Vec<ExprNode*> vals;
      vals.push_back(first);

      while (parser_match(P, TK_COMMA)) {
        parser_advance(P); // consume comma
        vals.push_back(parse_expr(P));
      }

      if (!parser_expect(P, TK_RPAREN))
        return NULL;

      Token* last = parser_peek(P, -1);
      AbsLocation end = token_abs_location(P.L, *last);

      auto* tup = heap_emplace<NodeExprTuple>(P.al);
      tup->vals = std::move(vals);
      tup->loc = {start.begin, end.end};
      return tup;
    }

    // Otherwise: must be a grouped expression
    if (!parser_expect(P, TK_RPAREN))
      return NULL;

    Token* last = parser_peek(P, -1);
    AbsLocation end = token_abs_location(P.L, *last);

    auto* group = heap_emplace<NodeExprGroup>(P.al);
    group->expr = first;
    group->loc = {start.begin, end.end};
    return group;
  }

  diagf<DK_ERROR>(
    P.dctx,
    loc,
    "Unexpected token '{}' while parsing primary expression",
    String(tok->lexeme, tok->size)
  );

  return NULL;
}

ExprNode* parse_expr(ParseState& P) {
  return parse_binary(P);
}

ExprNode* parse_binary(ParseState& P, int prec) {
  ExprNode* lhs = parse_unary(P);

  while (true) {
    TokenKind op = parser_peek(P)->kind;
    int op_prec = bin_prec(op);
    if (op_prec < prec)
      break;

    Token* oper = parser_advance(P);
    ExprNode* rhs = parse_binary(P, op_prec + 1);

    auto bin = heap_emplace<NodeExprBin>(P.al);
    bin->op = oper;
    bin->lhs = lhs;
    bin->rhs = rhs;
    bin->loc = {lhs->loc.begin, rhs->loc.end};
    lhs = bin;
  }

  return lhs;
}

ExprNode* parse_unary(ParseState& P) {
  if (parser_match(P, TK_MINUS) || parser_match(P, TK_BANG)) {
    Token* op = parser_peek(P);
    ExprNode* rhs = parse_unary(P);
    AbsLocation oploc = token_abs_location(P.L, *op);

    auto un = heap_emplace<NodeExprUn>(P.al);
    un->op = op;
    un->expr = rhs;
    un->loc = {oploc.begin, rhs->loc.end};
    return un;
  }

  return parse_primary(P);
}

} // namespace via
