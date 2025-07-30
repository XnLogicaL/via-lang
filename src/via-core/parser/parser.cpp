// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "parser.h"

#include <fmt/core.h>

#define SAVE_FIRST()       \
  auto* first = advance(); \
  auto loc = first->location(source);

namespace via {

namespace core {

namespace parser {

using lex::Token;
using lex::TokenKind;
using enum lex::TokenKind;
using namespace ast;

struct ParserError {
 public:
  AbsLocation loc;
  String msg;

  explicit ParserError(AbsLocation loc, String msg) : loc(loc), msg(msg) {}

  template <typename... Args>
  explicit ParserError(AbsLocation loc, Fmt<Args...> form, Args... args)
      : loc(loc), msg(fmt::format(form, std::forward<Args>(args)...)) {}
};

static bool is_expr_start(TokenKind kind) {
  switch (kind) {
    case TokenKind::INT:
    case TokenKind::BINT:
    case TokenKind::XINT:
    case TokenKind::NIL:
    case TokenKind::FP:
    case TokenKind::STRING:
    case TokenKind::IDENT:
    case TokenKind::LPAREN:
    case TokenKind::MINUS:
    case TokenKind::KW_NOT:
    case TokenKind::TILDE:
      return true;
    default:
      return false;
  }
}

static int bin_prec(TokenKind kind) {
  switch (kind) {
    case TokenKind::KW_OR:
      return 0;
    case TokenKind::KW_AND:
      return 1;
    case TokenKind::DBEQUALS:
    case TokenKind::BANGEQUALS:
    case TokenKind::LESSTHAN:
    case TokenKind::LESSTHANEQUALS:
    case TokenKind::GREATERTHAN:
    case TokenKind::GREATERTHANEQUALS:
      return 2;
    case TokenKind::AMPERSAND:
      return 3;
    case TokenKind::CARET:
      return 4;
    case TokenKind::PIPE:
      return 5;
    case TokenKind::KW_SHL:
    case TokenKind::KW_SHR:
      return 6;
    case TokenKind::PLUS:
    case TokenKind::MINUS:
      return 7;
    case TokenKind::ASTERISK:
    case TokenKind::FSLASH:
    case TokenKind::PERCENT:
      return 8;
    case TokenKind::POW:
      return 9;
    default:
      return -1;
  }
}

Token* Parser::peek(int ahead) {
  return cursor[ahead];
}

Token* Parser::advance() {
  cursor++;
  return peek(-1);
}

bool Parser::match(TokenKind kind, int ahead) {
  return peek(ahead)->kind == kind;
}

bool Parser::optional(TokenKind kind) {
  if (match(kind)) {
    advance();
    return true;
  }

  return false;
}

Token* Parser::expect(TokenKind kind, const char* task) {
  if (!match(kind)) {
    const Token& unexp = *peek();
    throw ParserError(unexp.location(source), "Unexpected token '{}' while {}",
                      unexp.to_string(), task);
  }

  return advance();
}

TupleBinding* Parser::parse_tuple_binding() {
  SAVE_FIRST()

  auto tpb = heap_emplace<TupleBinding>(alloc);

  while (!match(TokenKind::RBRACKET)) {
    Token* id = advance();
    auto id_loc = id->location(source);

    if (id->kind != TokenKind::IDENT)
      throw ParserError(id_loc,
                        "Unexpected token '{}' while parsing tuple binding",
                        id->to_string());

    auto sym = heap_emplace<NodeExprSym>(alloc);
    sym->loc = id->location(source);
    sym->tok = id;

    tpb->binds.push_back(sym);

    if (!match(TokenKind::RBRACKET))
      expect(TokenKind::COMMA, "parsing tuple binding");
  }

  tpb->loc = {loc.begin, advance()->location(source).end};
  return tpb;
}

LValue* Parser::parse_lvalue() {
  auto lval = heap_emplace<LValue>(alloc);

  if (match(TokenKind::IDENT)) {
    Token* id = advance();

    auto sym = heap_emplace<NodeExprSym>(alloc);
    sym->loc = id->location(source);
    sym->tok = id;

    lval->kind = LValue::Symbol;
    lval->sym = sym;
  } else if (match(TokenKind::LBRACKET)) {
    auto tpb = parse_tuple_binding();
    lval->kind = LValue::Tpb;
    lval->tpb = tpb;
  } else {
    Token* bad = peek();
    throw ParserError(
        bad->location(source),
        "Unexpected token '{}' while parsing variable declaration statement",
        bad->to_string());
  }

  return lval;
}

ExprNode* Parser::parse_primary() {
  SAVE_FIRST()

  switch (first->kind) {
    case TokenKind::INT:
    case TokenKind::BINT:
    case TokenKind::XINT:
    case TokenKind::NIL:
    case TokenKind::FP:
    case TokenKind::TRUE:
    case TokenKind::FALSE:
    case TokenKind::STRING: {
      advance();

      auto* lit = heap_emplace<NodeExprLit>(alloc);
      lit->tok = first;
      lit->loc = loc;

      return lit;
    }
    case TokenKind::IDENT: {
      advance();

      auto* sym = heap_emplace<NodeExprSym>(alloc);
      sym->tok = first;
      sym->loc = loc;

      return sym;
    }
    case TokenKind::LPAREN: {
      advance();

      AbsLocation start = loc;
      ExprNode* first = parse_expr();

      if (match(TokenKind::COMMA)) {
        Vec<ExprNode*> vals;
        vals.push_back(first);

        while (match(TokenKind::COMMA)) {
          advance();
          vals.push_back(parse_expr());
        }

        expect(TokenKind::RPAREN, "parsing tuple expression");

        auto* tup = heap_emplace<NodeExprTuple>(alloc);
        tup->vals = std::move(vals);
        tup->loc = {start.begin, peek(-1)->location(source).end};

        return tup;
      }

      expect(TokenKind::RPAREN, "parsing grouping expression");

      auto* group = heap_emplace<NodeExprGroup>(alloc);
      group->expr = first;
      group->loc = {start.begin, peek(-1)->location(source).end};

      return group;
    }
    default:
      throw ParserError(
          loc, "Unexpected token '{}' while parsing primary expression",
          first->to_string());
  }
}

ExprNode* Parser::parse_unary_or_postfix() {
  ExprNode* expr;

  switch (peek()->kind) {
    case TokenKind::KW_NOT:
    case TokenKind::MINUS:
    case TokenKind::TILDE: {
      auto* un = heap_emplace<NodeExprUn>(alloc);
      un->op = advance();
      un->expr = parse_unary_or_postfix();
      un->loc = {un->op->location(source).begin, un->expr->loc.end};
      expr = un;
      break;
    }
    default:
      expr = parse_primary();
      break;
  }

  while (true) {
    Token* first = peek();

    switch (first->kind) {
      case TokenKind::LPAREN: {  // Function call
        advance();               // consume '('

        Vec<ExprNode*> args;

        if (!match(TokenKind::RPAREN)) {
          do
            args.push_back(parse_expr());
          while (match(TokenKind::COMMA) && advance());

          expect(TokenKind::RPAREN, "parsing function call");
        } else
          advance();  // consume ')'

        auto* call = heap_emplace<NodeExprCall>(alloc);
        call->lval = expr;
        call->args = std::move(args);
        call->loc = {expr->loc.begin, peek(-1)->location(source).end};
        expr = call;
        break;
      }

      case TokenKind::LBRACKET: {  // Subscript
        advance();                 // consume '['

        ExprNode* idx = parse_expr();

        expect(TokenKind::RBRACKET, "parsing subscript expression");

        auto* subs = heap_emplace<NodeExprSubs>(alloc);
        subs->lval = expr;
        subs->idx = idx;
        subs->loc = {expr->loc.begin, peek(-1)->location(source).end};
        expr = subs;
        break;
      }

      default:
        return expr;
    }
  }
}

ExprNode* Parser::parse_expr(int min_prec) {
  ExprNode* lhs = parse_unary_or_postfix();

  int prec;
  while ((prec = bin_prec(peek()->kind), prec >= min_prec)) {
    auto bin = heap_emplace<NodeExprBin>(alloc);
    bin->op = advance();
    bin->lhs = lhs;
    bin->rhs = parse_expr(prec + 1);
    bin->loc = {lhs->loc.begin, bin->rhs->loc.end};
    lhs = bin;
  }

  return lhs;
}

NodeStmtScope* Parser::parse_scope() {
  auto* first = advance();
  auto loc = first->location(source);

  auto scope = heap_emplace<NodeStmtScope>(alloc);

  if (first->kind == TokenKind::COLON) {
    scope->stmts.push_back(parse_stmt());
    scope->loc = {loc.begin, scope->stmts.back()->loc.end};
  } else if (first->kind == TokenKind::LCURLY) {
    while (!match(TokenKind::RCURLY))
      scope->stmts.push_back(parse_stmt());

    advance();
  } else
    throw ParserError(loc, "Expected ':' or '{{' while parsing scope, got '{}'",
                      first->to_string());

  optional(TokenKind::SEMICOLON);
  return scope;
}

NodeStmtVar* Parser::parse_var() {
  SAVE_FIRST()

  auto vars = heap_emplace<NodeStmtVar>(alloc);
  vars->lval = parse_lvalue();

  expect(TokenKind::EQUALS, "parsing variable declaration");

  ExprNode* rval = parse_expr();
  vars->rval = rval;
  vars->loc = {loc.begin, rval->loc.end};

  optional(TokenKind::SEMICOLON);
  return vars;
}

NodeStmtFor* Parser::parse_for() {
  SAVE_FIRST()

  auto fors = heap_emplace<NodeStmtFor>(alloc);
  fors->init = parse_var();

  expect(TokenKind::COMMA, "parsing for statement");

  fors->target = parse_expr();

  expect(TokenKind::COMMA, "parsing for statement");

  fors->step = parse_expr();
  fors->br = parse_scope();
  fors->loc = {loc.begin, fors->br->loc.end};

  return fors;
}

NodeStmtForEach* Parser::parse_foreach() {
  SAVE_FIRST()

  auto fors = heap_emplace<NodeStmtForEach>(alloc);
  fors->lval = parse_lvalue();

  expect(TokenKind::KW_IN, "parsing for each statement");

  fors->iter = parse_expr();
  fors->br = parse_scope();
  fors->loc = {loc.begin, fors->br->loc.end};

  return fors;
}

NodeStmtIf* Parser::parse_if() {
  using Branch = NodeStmtIf::Branch;

  SAVE_FIRST()

  Branch br;
  br.cnd = parse_expr();
  br.br = parse_scope();

  auto ifs = heap_emplace<NodeStmtIf>(alloc);
  ifs->brs.push_back(br);
  ifs->loc = {loc.begin, br.br->loc.end};

  optional(TokenKind::SEMICOLON);
  return ifs;
}

NodeStmtWhile* Parser::parse_while() {
  SAVE_FIRST()

  auto whs = heap_emplace<NodeStmtWhile>(alloc);
  whs->cnd = parse_expr();
  whs->br = parse_scope();
  whs->loc = {loc.begin, whs->br->loc.end};

  return whs;
}

StmtNode* Parser::parse_stmt() {
  if (match(TokenKind::KW_IF))
    return parse_if();
  else if (match(TokenKind::KW_WHILE))
    return parse_while();
  else if (match(TokenKind::KW_VAR))
    return parse_var();
  else if (match(TokenKind::KW_DO)) {
    advance();
    return parse_scope();
  } else if (match(TokenKind::KW_FOR)) {
    // generic for loop
    if (match(TokenKind::KW_VAR, 1))
      return parse_for();

    // for each loop
    return parse_foreach();
  }

  if (match(TokenKind::SEMICOLON)) {
    auto empty = heap_emplace<NodeStmtEmpty>(alloc);
    empty->loc = advance()->location(source);
    return empty;
  }

  Token* first;
  if ((first = peek(), !is_expr_start(first->kind)))
    throw ParserError(first->location(source),
                      "Unexpected token '{}' while parsing statement",
                      first->to_string());

  auto es = heap_emplace<NodeStmtExpr>(alloc);
  es->expr = parse_expr();
  es->loc = es->expr->loc;

  optional(TokenKind::SEMICOLON);
  return es;
}

AstBuf Parser::parse() {
  Vec<StmtNode*> nodes;

  while (!match(TokenKind::EOF_)) {
    try {
      nodes.push_back(parse_stmt());
    } catch (const ParserError& e) {
      diag.diagnose<Diag::Error>(e.loc, e.msg);
      break;
    }
  }

  return AstBuf(nodes.data(),
                nodes.data() + (nodes.size() * sizeof(StmtNode*)));
}

void dump_ast(const AstBuf& B) {
  usize depth = 0;

  for (StmtNode** stmt = B.data; stmt < B.data + B.size; stmt++)
    fmt::println("{}", (*stmt)->get_dump(depth));
}

}  // namespace parser

}  // namespace core

}  // namespace via
