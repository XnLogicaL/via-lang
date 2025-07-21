// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "parser.h"

namespace via {

struct ParserError {
public:
  AbsLocation loc;
  String msg;

  explicit ParserError(AbsLocation loc, String msg)
    : loc(loc),
      msg(msg) {}

  template<typename... Args>
  explicit ParserError(AbsLocation loc, Fmt<Args...> fmt, Args... args)
    : loc(loc),
      msg(std::format(fmt, std::forward<Args>(args)...)) {}
};

static bool is_expr_start(TokenKind kind) {
  switch (kind) {
  case TK_INT:
  case TK_BINT:
  case TK_XINT:
  case TK_NIL:
  case TK_FP:
  case TK_STRING:
  case TK_IDENT:
  case TK_LPAREN:
  case TK_MINUS:
  case TK_BANG:
  case TK_TILDE:
    return true;
  default:
    return false;
  }
}

static int bin_prec(TokenKind kind) {
  switch (kind) {
  case TK_KW_OR:
    return 0;
  case TK_KW_AND:
    return 1;
  case TK_DBEQUALS:
  case TK_BANGEQUALS:
  case TK_LESSTHAN:
  case TK_LESSTHANEQUALS:
  case TK_GREATERTHAN:
  case TK_GREATERTHANEQUALS:
    return 2;
  case TK_AMPERSAND:
    return 3;
  case TK_CARET:
    return 4;
  case TK_PIPE:
    return 5;
  case TK_LSHIFT:
  case TK_RSHIFT:
    return 6;
  case TK_PLUS:
  case TK_MINUS:
    return 7;
  case TK_ASTERISK:
  case TK_FSLASH:
  case TK_PERCENT:
    return 8;
  default:
    return -1;
  }
}

static Token* parser_peek(ParseState& P, const int ahead = 0) {
  return P.cursor[ahead];
}

static Token* parser_advance(ParseState& P) {
  P.cursor++;
  return parser_peek(P, -1);
}

static bool parser_match(ParseState& P, const TokenKind kind, int ahead = 0) {
  return parser_peek(P, ahead)->kind == kind;
}

static bool parser_optional(ParseState& P, const TokenKind kind) {
  if (parser_match(P, kind)) {
    parser_advance(P);
    return true;
  }

  return false;
}

static Token* parser_expect(ParseState& P, const TokenKind kind) {
  if (!parser_match(P, kind)) {
    const Token& unexp = *parser_peek(P);
    throw ParserError(
      token_abs_location(P.L, unexp), "Unexpected token '{}'", String(unexp.lexeme, unexp.size)
    );
  }

  return parser_advance(P);
}

static TupleBinding* parse_tuple_binding(ParseState& P) {
  Token* tok = parser_advance(P);
  AbsLocation loc = token_abs_location(P.L, *tok);

  auto tpb = heap_emplace<TupleBinding>(P.al);

  while (!parser_match(P, TK_RBRACKET)) {
    Token* id = parser_advance(P);
    AbsLocation id_location = token_abs_location(P.L, *id);

    if (id->kind != TK_IDENT)
      throw ParserError(
        id_location,
        "Unexpected tone '{}' while parsing tuple binding",
        String(id->lexeme, id->size)
      );

    auto sym = heap_emplace<NodeExprSym>(P.al);
    sym->loc = id_location;
    sym->tok = id;

    tpb->binds.push_back(sym);

    if (!parser_match(P, TK_RBRACKET))
      parser_expect(P, TK_COMMA);
  }

  Token* rb = parser_advance(P);
  AbsLocation rbloc = token_abs_location(P.L, *rb);

  tpb->loc = {loc.begin, rbloc.end};
  return tpb;
}

static LValue* parse_lvalue(ParseState& P) {
  auto lval = heap_emplace<LValue>(P.al);

  if (parser_match(P, TK_IDENT)) {
    Token* id = parser_advance(P);

    auto sym = heap_emplace<NodeExprSym>(P.al);
    sym->loc = token_abs_location(P.L, *id);
    sym->tok = id;

    lval->kind = LValue::LVK_SYM;
    lval->sym = sym;
  }
  else if (parser_match(P, TK_LBRACKET)) {
    auto tpb = parse_tuple_binding(P);
    lval->kind = LValue::LVK_TPB;
    lval->tpb = tpb;
  }
  else {
    Token* bad = parser_peek(P);
    throw ParserError(
      token_abs_location(P.L, *bad),
      "Unexpected token '{}' while parsing 'var' statement",
      String(bad->lexeme, bad->size)
    );
  }

  return lval;
}

ExprNode* parse_expr(ParseState& P, int min_prec = 0);

static ExprNode* parse_primary(ParseState& P) {
  Token* tok = parser_peek(P);
  AbsLocation loc = token_abs_location(P.L, *tok);

  switch (tok->kind) {
  case TK_INT:
  case TK_BINT:
  case TK_XINT:
  case TK_NIL:
  case TK_FP:
  case TK_TRUE:
  case TK_FALSE:
  case TK_STRING: {
    parser_advance(P);

    auto* lit = heap_emplace<NodeExprLit>(P.al);
    lit->tok = tok;
    lit->loc = loc;

    return lit;
  }
  case TK_IDENT: {
    parser_advance(P);

    auto* sym = heap_emplace<NodeExprSym>(P.al);
    sym->tok = tok;
    sym->loc = loc;

    return sym;
  }
  case TK_LPAREN: {
    parser_advance(P);

    AbsLocation start = loc;
    ExprNode* first = parse_expr(P);

    if (parser_match(P, TK_COMMA)) {
      Vec<ExprNode*> vals;
      vals.push_back(first);

      while (parser_match(P, TK_COMMA)) {
        parser_advance(P);
        vals.push_back(parse_expr(P));
      }

      parser_expect(P, TK_RPAREN);

      auto* tup = heap_emplace<NodeExprTuple>(P.al);
      tup->vals = std::move(vals);
      tup->loc = {start.begin, token_abs_location(P.L, *parser_peek(P, -1)).end};

      return tup;
    }

    parser_expect(P, TK_RPAREN);

    auto* group = heap_emplace<NodeExprGroup>(P.al);
    group->expr = first;
    group->loc = {start.begin, token_abs_location(P.L, *parser_peek(P, -1)).end};

    return group;
  }
  default:
    throw ParserError(
      loc, "Unexpected token '{}' while parsing primary expression", String(tok->lexeme, tok->size)
    );
  }
}

static ExprNode* parse_unary_or_postfix(ParseState& P) {
  ExprNode* expr;

  switch (parser_peek(P)->kind) {
  case TK_BANG:
  case TK_MINUS:
  case TK_TILDE: {
    auto* un = heap_emplace<NodeExprUn>(P.al);
    un->op = parser_advance(P);
    un->expr = parse_unary_or_postfix(P);
    un->loc = {token_abs_location(P.L, *un->op).begin, un->expr->loc.end};
    expr = un;
    break;
  }
  default:
    expr = parse_expr(P);
    break;
  }

  while (true) {
    Token* tok = parser_peek(P);

    switch (tok->kind) {
    case TK_LPAREN: {    // Function call
      parser_advance(P); // consume '('

      Vec<ExprNode*> args;

      if (!parser_match(P, TK_RPAREN)) {
        do
          args.push_back(parse_expr(P));
        while (parser_match(P, TK_COMMA) && parser_advance(P));

        parser_expect(P, TK_RPAREN);
      }
      else
        parser_advance(P); // consume ')'

      auto* call = heap_emplace<NodeExprCall>(P.al);
      call->lval = expr;
      call->args = std::move(args);
      call->loc = {expr->loc.begin, token_abs_location(P.L, *parser_peek(P, -1)).end};
      expr = call;
      break;
    }

    case TK_LBRACKET: {  // Subscript
      parser_advance(P); // consume '['

      ExprNode* idx = parse_expr(P);

      parser_expect(P, TK_RBRACKET);

      auto* subs = heap_emplace<NodeExprSubs>(P.al);
      subs->lval = expr;
      subs->idx = idx;
      subs->loc = {expr->loc.begin, token_abs_location(P.L, *parser_peek(P, -1)).end};
      expr = subs;
      break;
    }

    default:
      return expr;
    }
  }
}

ExprNode* parse_expr(ParseState& P, int min_prec) {
  ExprNode* lhs = parse_unary_or_postfix(P);

  int prec;
  while ((prec = bin_prec(parser_peek(P)->kind), prec >= min_prec)) {
    auto bin = heap_emplace<NodeExprBin>(P.al);
    bin->op = parser_advance(P);
    bin->lhs = lhs;
    bin->rhs = parse_expr(P, prec + 1);
    bin->loc = {lhs->loc.begin, bin->rhs->loc.end};
    lhs = bin;
  }

  return lhs;
}

StmtNode* parse_stmt(ParseState& P);

static NodeStmtScope* parse_scope(ParseState& P) {
  Token* tok = parser_advance(P);
  AbsLocation loc = token_abs_location(P.L, *tok);

  auto scope = heap_emplace<NodeStmtScope>(P.al);

  if (tok->kind == TK_COLON) {
    scope->stmts.push_back(parse_stmt(P));
    scope->loc = {loc.begin, scope->stmts.back()->loc.end};
  }
  else if (tok->kind == TK_LCURLY) {
    while (!parser_match(P, TK_RCURLY))
      scope->stmts.push_back(parse_stmt(P));

    parser_advance(P);
  }
  else
    throw ParserError(
      loc, "Expected ':' or '{{' while parsing scope, got '{}'", String(tok->lexeme, tok->size)
    );

  parser_optional(P, TK_SEMICOLON);
  return scope;
}

static NodeStmtVar* parse_var(ParseState& P) {
  Token* tok = parser_advance(P);
  AbsLocation loc = token_abs_location(P.L, *tok);

  auto vars = heap_emplace<NodeStmtVar>(P.al);
  vars->lval = parse_lvalue(P);

  parser_expect(P, TK_EQUALS);

  ExprNode* rval = parse_expr(P);
  vars->rval = rval;
  vars->loc = {loc.begin, rval->loc.end};

  parser_optional(P, TK_SEMICOLON);
  return vars;
}

static NodeStmtFor* parse_for(ParseState& P) {
  Token* tok = parser_advance(P);
  AbsLocation loc = token_abs_location(P.L, *tok);

  auto fors = heap_emplace<NodeStmtFor>(P.al);
  fors->init = parse_var(P);

  parser_expect(P, TK_COMMA);

  fors->target = parse_expr(P);

  parser_expect(P, TK_COMMA);

  fors->step = parse_expr(P);
  fors->br = parse_scope(P);
  fors->loc = {loc.begin, fors->br->loc.end};

  return fors;
}

static NodeStmtForEach* parse_foreach(ParseState& P) {
  Token* tok = parser_advance(P);
  AbsLocation loc = token_abs_location(P.L, *tok);

  auto fors = heap_emplace<NodeStmtForEach>(P.al);
  fors->lval = parse_lvalue(P);

  parser_expect(P, TK_KW_IN);

  fors->iter = parse_expr(P);
  fors->br = parse_scope(P);
  fors->loc = {loc.begin, fors->br->loc.end};

  return fors;
}

static NodeStmtIf* parse_if(ParseState& P) {
  using Branch = NodeStmtIf::Branch;

  Token* tok = parser_advance(P);
  AbsLocation loc = token_abs_location(P.L, *tok);

  Branch br;
  br.cnd = parse_expr(P);
  br.br = parse_scope(P);

  auto ifs = heap_emplace<NodeStmtIf>(P.al);
  ifs->brs.push_back(br);
  ifs->loc = {loc.begin, br.br->loc.end};

  parser_optional(P, TK_SEMICOLON);
  return ifs;
}

static NodeStmtWhile* parse_while(ParseState& P) {
  Token* tok = parser_advance(P);
  AbsLocation loc = token_abs_location(P.L, *tok);

  auto whs = heap_emplace<NodeStmtWhile>(P.al);
  whs->cnd = parse_expr(P);
  whs->br = parse_scope(P);
  whs->loc = {loc.begin, whs->br->loc.end};

  return whs;
}

StmtNode* parse_stmt(ParseState& P) {
  if (parser_match(P, TK_KW_IF))
    return parse_if(P);
  else if (parser_match(P, TK_KW_WHILE))
    return parse_while(P);
  else if (parser_match(P, TK_KW_VAR))
    return parse_var(P);
  else if (parser_match(P, TK_KW_DO)) {
    parser_advance(P);
    return parse_scope(P);
  }
  else if (parser_match(P, TK_KW_FOR)) {
    // generic for loop
    if (parser_match(P, TK_KW_VAR, 1))
      return parse_for(P);

    // for each loop
    return parse_foreach(P);
  }

  if (parser_match(P, TK_SEMICOLON)) {
    auto empty = heap_emplace<NodeStmtEmpty>(P.al);
    empty->loc = token_abs_location(P.L, *parser_advance(P));
    return empty;
  }

  Token* tok;
  if ((tok = parser_peek(P), !is_expr_start(tok->kind)))
    throw ParserError(
      token_abs_location(P.L, *tok),
      "Unexpected token '{}' while parsing statement",
      String(tok->lexeme, tok->size)
    );

  auto es = heap_emplace<NodeStmtExpr>(P.al);
  es->expr = parse_expr(P);
  es->loc = es->expr->loc;

  parser_optional(P, TK_SEMICOLON);
  return es;
}

AstBuf parser_parse(ParseState& P) {
  Vec<StmtNode*> nodes;

  while (!parser_match(P, TK_EOF)) {
    try {
      nodes.push_back(parse_stmt(P));
    }
    catch (const ParserError& e) {
      diag<DK_ERROR>(P.dctx, e.loc, e.msg);
      break;
    }
  }

  return AstBuf(nodes.data(), nodes.data() + (nodes.size() * sizeof(StmtNode*)));
}

void dump_ast(const AstBuf& B) {
  usize depth = 0;

  for (StmtNode** stmt = B.data; stmt < B.data + B.size; stmt++)
    dump_stmt(*stmt, depth);
}

} // namespace via
