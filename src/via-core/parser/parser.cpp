// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "parser.h"

#include <fmt/core.h>

#define SAVE_FIRST()       \
  auto* first = advance(); \
  auto loc = first->location(m_source);

namespace via {

using enum Token::Kind;
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

static bool is_expr_start(Token::Kind kind) {
  switch (kind) {
    case Token::Kind::INT:
    case Token::Kind::BINT:
    case Token::Kind::XINT:
    case Token::Kind::NIL:
    case Token::Kind::FP:
    case Token::Kind::STRING:
    case Token::Kind::IDENT:
    case Token::Kind::LPAREN:
    case Token::Kind::MINUS:
    case Token::Kind::KW_NOT:
    case Token::Kind::TILDE:
      return true;
    default:
      return false;
  }
}

static int bin_prec(Token::Kind kind) {
  switch (kind) {
    case Token::Kind::KW_OR:
      return 0;
    case Token::Kind::KW_AND:
      return 1;
    case Token::Kind::DBEQUALS:
    case Token::Kind::BANGEQUALS:
    case Token::Kind::LESSTHAN:
    case Token::Kind::LESSTHANEQUALS:
    case Token::Kind::GREATERTHAN:
    case Token::Kind::GREATERTHANEQUALS:
      return 2;
    case Token::Kind::AMPERSAND:
      return 3;
    case Token::Kind::CARET:
      return 4;
    case Token::Kind::PIPE:
      return 5;
    case Token::Kind::KW_SHL:
    case Token::Kind::KW_SHR:
      return 6;
    case Token::Kind::PLUS:
    case Token::Kind::MINUS:
      return 7;
    case Token::Kind::ASTERISK:
    case Token::Kind::FSLASH:
    case Token::Kind::PERCENT:
      return 8;
    case Token::Kind::POW:
      return 9;
    default:
      return -1;
  }
}

Token* Parser::peek(int ahead) {
  return m_cursor[ahead];
}

Token* Parser::advance() {
  return *(m_cursor++);
}

bool Parser::match(Token::Kind kind, int ahead) {
  return peek(ahead)->kind == kind;
}

bool Parser::optional(Token::Kind kind) {
  if (match(kind)) {
    advance();
    return true;
  }

  return false;
}

Token* Parser::expect(Token::Kind kind, const char* task) {
  if (!match(kind)) {
    const Token& unexp = *peek();
    throw ParserError(unexp.location(m_source),
                      "Unexpected token '{}' while {}", unexp.to_string(),
                      task);
  }

  return advance();
}

TupleBinding* Parser::parse_tuple_binding() {
  SAVE_FIRST()

  auto tpb = m_alloc.emplace<TupleBinding>();

  while (!match(Token::Kind::RBRACKET)) {
    Token* id = advance();
    auto id_loc = id->location(m_source);

    if (id->kind != Token::Kind::IDENT)
      throw ParserError(id_loc,
                        "Unexpected token '{}' while parsing tuple binding",
                        id->to_string());

    auto sym = m_alloc.emplace<NodeExprSym>();
    sym->loc = id->location(m_source);
    sym->tok = id;

    tpb->binds.push_back(sym);

    if (!match(Token::Kind::RBRACKET))
      expect(Token::Kind::COMMA, "parsing tuple binding");
  }

  tpb->loc = {loc.begin, advance()->location(m_source).end};
  return tpb;
}

LValue* Parser::parse_lvalue() {
  auto lval = m_alloc.emplace<LValue>();

  if (match(Token::Kind::IDENT)) {
    Token* id = advance();

    auto sym = m_alloc.emplace<NodeExprSym>();
    sym->loc = id->location(m_source);
    sym->tok = id;

    lval->kind = LValue::Kind::Symbol;
    lval->sym = sym;
  } else if (match(Token::Kind::LBRACKET)) {
    auto tpb = parse_tuple_binding();
    lval->kind = LValue::Kind::Tpb;
    lval->tpb = tpb;
  } else {
    Token* bad = peek();
    throw ParserError(
        bad->location(m_source),
        "Unexpected token '{}' while parsing variable declaration statement",
        bad->to_string());
  }

  return lval;
}

ExprNode* Parser::parse_primary() {
  SAVE_FIRST()

  switch (first->kind) {
    case Token::Kind::INT:
    case Token::Kind::BINT:
    case Token::Kind::XINT:
    case Token::Kind::NIL:
    case Token::Kind::FP:
    case Token::Kind::TRUE:
    case Token::Kind::FALSE:
    case Token::Kind::STRING: {
      advance();

      auto* lit = m_alloc.emplace<NodeExprLit>();
      lit->tok = first;
      lit->loc = loc;
      return lit;
    }
    case Token::Kind::IDENT: {
      advance();

      auto* sym = m_alloc.emplace<NodeExprSym>();
      sym->tok = first;
      sym->loc = loc;

      return sym;
    }
    case Token::Kind::LPAREN: {
      advance();

      AbsLocation start = loc;
      ExprNode* first = parse_expr();

      if (match(Token::Kind::COMMA)) {
        Vec<ExprNode*> vals;
        vals.push_back(first);

        while (match(Token::Kind::COMMA)) {
          advance();
          vals.push_back(parse_expr());
        }

        expect(Token::Kind::RPAREN, "parsing tuple expression");

        auto* tup = m_alloc.emplace<NodeExprTuple>();
        tup->vals = std::move(vals);
        tup->loc = {start.begin, peek(-1)->location(m_source).end};

        return tup;
      }

      expect(Token::Kind::RPAREN, "parsing grouping expression");

      auto* group = m_alloc.emplace<NodeExprGroup>();
      group->expr = first;
      group->loc = {start.begin, peek(-1)->location(m_source).end};

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
    case Token::Kind::KW_NOT:
    case Token::Kind::MINUS:
    case Token::Kind::TILDE: {
      auto* un = m_alloc.emplace<NodeExprUn>();
      un->op = advance();
      un->expr = parse_unary_or_postfix();
      un->loc = {un->op->location(m_source).begin, un->expr->loc.end};
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
      case Token::Kind::LPAREN: {  // Function call
        advance();                 // consume '('

        Vec<ExprNode*> args;

        if (!match(Token::Kind::RPAREN)) {
          do
            args.push_back(parse_expr());
          while (match(Token::Kind::COMMA) && advance());

          expect(Token::Kind::RPAREN, "parsing function call");
        } else
          advance();  // consume ')'

        auto* call = m_alloc.emplace<NodeExprCall>();
        call->lval = expr;
        call->args = std::move(args);
        call->loc = {expr->loc.begin, peek(-1)->location(m_source).end};
        expr = call;
        break;
      }

      case Token::Kind::LBRACKET: {  // Subscript
        advance();                   // consume '['

        ExprNode* idx = parse_expr();

        expect(Token::Kind::RBRACKET, "parsing subscript expression");

        auto* subs = m_alloc.emplace<NodeExprSubs>();
        subs->lval = expr;
        subs->idx = idx;
        subs->loc = {expr->loc.begin, peek(-1)->location(m_source).end};
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
    auto bin = m_alloc.emplace<NodeExprBin>();
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
  auto loc = first->location(m_source);

  auto scope = m_alloc.emplace<NodeStmtScope>();

  if (first->kind == Token::Kind::COLON) {
    scope->stmts.push_back(parse_stmt());
    scope->loc = {loc.begin, scope->stmts.back()->loc.end};
  } else if (first->kind == Token::Kind::LCURLY) {
    while (!match(Token::Kind::RCURLY))
      scope->stmts.push_back(parse_stmt());

    advance();
  } else
    throw ParserError(loc, "Expected ':' or '{{' while parsing scope, got '{}'",
                      first->to_string());

  optional(Token::Kind::SEMICOLON);
  return scope;
}

NodeStmtVar* Parser::parse_var() {
  SAVE_FIRST()

  auto vars = m_alloc.emplace<NodeStmtVar>();
  vars->lval = parse_lvalue();

  expect(Token::Kind::EQUALS, "parsing variable declaration");

  ExprNode* rval = parse_expr();
  vars->rval = rval;
  vars->loc = {loc.begin, rval->loc.end};

  optional(Token::Kind::SEMICOLON);
  return vars;
}

NodeStmtFor* Parser::parse_for() {
  SAVE_FIRST()

  auto fors = m_alloc.emplace<NodeStmtFor>();
  fors->init = parse_var();

  expect(Token::Kind::COMMA, "parsing for statement");

  fors->target = parse_expr();

  expect(Token::Kind::COMMA, "parsing for statement");

  fors->step = parse_expr();
  fors->br = parse_scope();
  fors->loc = {loc.begin, fors->br->loc.end};

  return fors;
}

NodeStmtForEach* Parser::parse_foreach() {
  SAVE_FIRST()

  auto fors = m_alloc.emplace<NodeStmtForEach>();
  fors->lval = parse_lvalue();

  expect(Token::Kind::KW_IN, "parsing for each statement");

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

  auto ifs = m_alloc.emplace<NodeStmtIf>();
  ifs->brs.push_back(br);
  ifs->loc = {loc.begin, br.br->loc.end};

  optional(Token::Kind::SEMICOLON);
  return ifs;
}

NodeStmtWhile* Parser::parse_while() {
  SAVE_FIRST()

  auto whs = m_alloc.emplace<NodeStmtWhile>();
  whs->cnd = parse_expr();
  whs->br = parse_scope();
  whs->loc = {loc.begin, whs->br->loc.end};

  return whs;
}

StmtNode* Parser::parse_stmt() {
  if (match(Token::Kind::KW_IF))
    return parse_if();
  else if (match(Token::Kind::KW_WHILE))
    return parse_while();
  else if (match(Token::Kind::KW_VAR))
    return parse_var();
  else if (match(Token::Kind::KW_DO)) {
    advance();
    return parse_scope();
  } else if (match(Token::Kind::KW_FOR)) {
    // generic for loop
    if (match(Token::Kind::KW_VAR, 1))
      return parse_for();

    // for each loop
    return parse_foreach();
  }

  if (match(Token::Kind::SEMICOLON)) {
    auto empty = m_alloc.emplace<NodeStmtEmpty>();
    empty->loc = advance()->location(m_source);
    return empty;
  }

  Token* first;
  if ((first = peek(), !is_expr_start(first->kind)))
    throw ParserError(first->location(m_source),
                      "Unexpected token '{}' while parsing statement",
                      first->to_string());

  auto es = m_alloc.emplace<NodeStmtExpr>();
  es->expr = parse_expr();
  es->loc = es->expr->loc;

  optional(Token::Kind::SEMICOLON);
  return es;
}

SyntaxTree Parser::parse() {
  SyntaxTree nodes;

  while (!match(Token::Kind::EOF_)) {
    try {
      nodes.push_back(parse_stmt());
    } catch (const ParserError& e) {
      m_diag.diagnose<Diagnosis::Kind::Error>(e.loc, e.msg);
      break;
    }
  }

  return nodes;
}

}  // namespace via
