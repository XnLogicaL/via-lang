// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "parser.h"

#include <fmt/core.h>

#define SAVE_FIRST()       \
  auto* first = advance(); \
  auto loc = first->location(m_source);

namespace via
{

using enum Token::Kind;
using namespace ast;

struct ParserError
{
 public:
  SourceLoc loc;
  String msg;

  explicit ParserError(SourceLoc loc, String msg) : loc(loc), msg(msg) {}

  template <typename... Args>
  explicit ParserError(SourceLoc loc, Fmt<Args...> form, Args... args)
      : loc(loc), msg(fmt::format(form, std::forward<Args>(args)...))
  {}
};

static bool is_expr_start(Token::Kind kind)
{
  switch (kind) {
    case INT:
    case BINT:
    case XINT:
    case NIL:
    case FP:
    case STRING:
    case IDENT:
    case LPAREN:
    case MINUS:
    case KW_NOT:
    case TILDE:
      return true;
    default:
      return false;
  }
}

static int bin_prec(Token::Kind kind)
{
  switch (kind) {
    case KW_OR:
      return 0;
    case KW_AND:
      return 1;
    case DBEQUALS:
    case BANGEQUALS:
    case LESSTHAN:
    case LESSTHANEQUALS:
    case GREATERTHAN:
    case GREATERTHANEQUALS:
      return 2;
    case AMPERSAND:
      return 3;
    case CARET:
      return 4;
    case PIPE:
      return 5;
    case KW_SHL:
    case KW_SHR:
      return 6;
    case PLUS:
    case MINUS:
      return 7;
    case ASTERISK:
    case FSLASH:
    case PERCENT:
      return 8;
    case POW:
      return 9;
    default:
      return -1;
  }
}

Token* Parser::peek(int ahead)
{
  return m_cursor[ahead];
}

Token* Parser::advance()
{
  return *(m_cursor++);
}

bool Parser::match(Token::Kind kind, int ahead)
{
  return peek(ahead)->kind == kind;
}

bool Parser::optional(Token::Kind kind)
{
  if (match(kind)) {
    advance();
    return true;
  }

  return false;
}

Token* Parser::expect(Token::Kind kind, const char* task)
{
  if (!match(kind)) {
    const Token& unexp = *peek();
    throw ParserError(unexp.location(m_source),
                      "Unexpected token '{}' ({}) while {} ({} expected)",
                      unexp.to_string(), magic_enum::enum_name(unexp.kind),
                      task, magic_enum::enum_name(kind));
  }

  return advance();
}

TupleBinding* Parser::parse_tuple_binding()
{
  SAVE_FIRST()

  auto* tpb = m_alloc.emplace<TupleBinding>();

  while (!match(RBRACKET)) {
    Token* id = expect(IDENT, "parsing tuple binding");
    auto id_loc = id->location(m_source);

    tpb->binds.push_back(id);

    if (!match(RBRACKET))
      expect(COMMA, "parsing tuple binding");
  }

  tpb->loc = {loc.begin, advance()->location(m_source).end};
  return tpb;
}

Path* Parser::parse_static_path()
{
  auto* sp = m_alloc.emplace<Path>();

  while (true) {
    sp->path.push_back(expect(IDENT, "parsing static path"));

    if (match(DBCOLON)) {
      advance();
    } else {
      break;
    }
  }

  sp->loc = {sp->path.front()->location(m_source).begin,
             sp->path.back()->location(m_source).end};
  return sp;
}

Path* Parser::parse_dynamic_path()
{
  auto* dp = m_alloc.emplace<Path>();

  while (true) {
    dp->path.push_back(expect(IDENT, "parsing dynamic path"));

    if (match(DOT)) {
      advance();
    } else {
      break;
    }
  }

  dp->loc = {dp->path.front()->location(m_source).begin,
             dp->path.back()->location(m_source).end};
  return dp;
}

LValue* Parser::parse_lvalue()
{
  auto* lval = m_alloc.emplace<LValue>();

  if (match(IDENT)) {
    if (match(DOT, 1)) {
      lval->kind = LValue::Kind::DP;
      lval->path = parse_dynamic_path();
      lval->loc = lval->path->loc;
    } else if (match(DBCOLON, 1)) {
      lval->kind = LValue::Kind::SP;
      lval->path = parse_static_path();
      lval->loc = lval->path->loc;
    } else {
      lval->kind = LValue::Kind::SYM;
      lval->sym = advance();
      lval->loc = lval->sym->location(m_source);
    }
  } else if (match(LBRACKET)) {
    auto tpb = parse_tuple_binding();
    lval->kind = LValue::Kind::TPB;
    lval->tpb = tpb;
    lval->loc = lval->tpb->loc;
  } else {
    Token* bad = peek();
    throw ParserError(bad->location(m_source),
                      "Unexpected token '{}' ({}) while parsing lvalue "
                      "(expected identifier or '[')",
                      magic_enum::enum_name(bad->kind), bad->to_string());
  }

  return lval;
}

PlValue* Parser::parse_plvalue()
{
  auto* plval = m_alloc.emplace<PlValue>();

  if (expect(IDENT, "parsing plvalue")) {
    if (match(DOT, 1)) {
      plval->kind = PlValue::Kind::DP;
      plval->path = parse_dynamic_path();
      plval->loc = plval->path->loc;
    } else if (match(DBCOLON, 1)) {
      plval->kind = PlValue::Kind::SP;
      plval->path = parse_static_path();
      plval->loc = plval->path->loc;
    } else {
      plval->kind = PlValue::Kind::SYM;
      plval->sym = advance();
      plval->loc = plval->sym->location(m_source);
    }
  }

  return plval;
}

Parameter* Parser::parse_parameter()
{
  SAVE_FIRST()

  auto* par = m_alloc.emplace<Parameter>();
  par->sym = first;

  if (optional(COLON)) {
    par->type = parse_type();
    par->loc = {loc.begin, par->type->loc.end};
  } else {
    par->loc = loc;
  }

  return par;
}

AttributeGroup* Parser::parse_attribute_group()
{
  SAVE_FIRST()
  expect(LBRACKET, "parsing attribute group");

  auto* atg = m_alloc.emplace<AttributeGroup>();

  while (true) {
    atg->ats.push_back(AttributeGroup::Attribute{
        .sp = parse_static_path(),
        .args = {},
    });

    if (match(RBRACKET)) {
      break;
    } else {
      expect(COMMA, "parsing attribute group");
    }
  }

  Token* last = expect(RBRACKET, "terminating attribute group");
  atg->loc = {loc.begin, last->location(m_source).end};
  return atg;
}

Expr* Parser::parse_expr_primary()
{
  SAVE_FIRST()

  switch (first->kind) {
    case INT:
    case BINT:
    case XINT:
    case NIL:
    case FP:
    case TRUE:
    case FALSE:
    case STRING: {
      auto* lit = m_alloc.emplace<ExprLit>();
      lit->tok = first;
      lit->loc = loc;
      return lit;
    }
    case IDENT: {
      auto* sym = m_alloc.emplace<ExprSymbol>();
      sym->sym = first;
      sym->loc = loc;
      return sym;
    }
    case LPAREN: {
      SourceLoc start = loc;
      Expr* first = parse_expr();

      if (match(COMMA)) {
        Vec<const Expr*> vals;
        vals.push_back(first);

        while (match(COMMA)) {
          advance();
          vals.push_back(parse_expr());
        }

        expect(RPAREN, "parsing tuple expression");

        auto* tup = m_alloc.emplace<ExprTuple>();
        tup->vals = std::move(vals);
        tup->loc = {start.begin, peek(-1)->location(m_source).end};

        return tup;
      }

      expect(RPAREN, "parsing grouping expression");

      auto* group = m_alloc.emplace<ExprGroup>();
      group->expr = first;
      group->loc = {start.begin, peek(-1)->location(m_source).end};

      return group;
    }
    default:
      throw ParserError(loc,
                        "Unexpected token '{}' ({}) while parsing primary "
                        "expression (expected literal, identifier or '(')",
                        magic_enum::enum_name(first->kind), first->to_string());
  }
}

Expr* Parser::parse_expr_unary_or_postfix()
{
  Expr* expr;

  switch (peek()->kind) {
    case KW_NOT:
    case MINUS:
    case TILDE: {
      auto* un = m_alloc.emplace<ExprUnary>();
      un->op = advance();
      un->expr = parse_expr_unary_or_postfix();
      un->loc = {un->op->location(m_source).begin, un->expr->loc.end};
      expr = un;
      break;
    }
    default:
      expr = parse_expr_primary();
      break;
  }

  while (true) {
    Token* first = peek();

    switch (first->kind) {
      case KW_AS: {
        advance();

        auto* cast = m_alloc.emplace<ExprCast>();
        cast->expr = expr;
        cast->type = parse_type();
        cast->loc = {cast->expr->loc.begin, cast->type->loc.end};
        expr = cast;
        break;
      }

      case LPAREN: {  // Function call
        advance();    // consume '('

        Vec<const Expr*> args;

        if (!match(RPAREN)) {
          do
            args.push_back(parse_expr());
          while (match(COMMA) && advance());

          expect(RPAREN, "parsing function call");
        } else
          advance();  // consume ')'

        auto* call = m_alloc.emplace<ExprCall>();
        call->lval = expr;
        call->args = std::move(args);
        call->loc = {expr->loc.begin, peek(-1)->location(m_source).end};
        expr = call;
        break;
      }

      case LBRACKET: {  // Subscript
        advance();      // consume '['

        Expr* idx = parse_expr();

        expect(RBRACKET, "parsing subscript expression");

        auto* subs = m_alloc.emplace<ExprSubscript>();
        subs->lval = expr;
        subs->idx = idx;
        subs->loc = {expr->loc.begin, peek(-1)->location(m_source).end};
        expr = subs;
        break;
      }

      case DOT: {   // Dynamic access
        advance();  // consume '.'

        auto* da = m_alloc.emplace<ExprDynAccess>();
        da->expr = expr;
        da->index = expect(IDENT, "parsing dynamic access expression");
        da->loc = {da->expr->loc.begin, da->index->location(m_source).end};
        expr = da;
        break;
      }

      case DBCOLON: {  // Static access
        advance();     // consume '::'

        auto* sa = m_alloc.emplace<ExprStaticAccess>();
        sa->expr = expr;
        sa->index = expect(IDENT, "parsing static access expression");
        sa->loc = {sa->expr->loc.begin, sa->index->location(m_source).end};
        expr = sa;
        break;
      }

      default:
        return expr;
    }
  }
}

Expr* Parser::parse_expr(int min_prec)
{
  Expr* lhs = parse_expr_unary_or_postfix();

  int prec;
  while ((prec = bin_prec(peek()->kind), prec >= min_prec)) {
    auto bin = m_alloc.emplace<ExprBinary>();
    bin->op = advance();
    bin->lhs = lhs;
    bin->rhs = parse_expr(prec + 1);
    bin->loc = {lhs->loc.begin, bin->rhs->loc.end};
    lhs = bin;
  }

  return lhs;
}

TypeBuiltin* Parser::parse_type_builtin()
{
  SAVE_FIRST()

  auto* bt = m_alloc.emplace<TypeBuiltin>();
  bt->tok = first;
  bt->loc = loc;
  return bt;
}

TypeArray* Parser::parse_type_array()
{
  SAVE_FIRST();

  auto* at = m_alloc.emplace<TypeArray>();
  at->type = parse_type();

  Token* end = expect(RBRACKET, "terminating array type");

  at->loc = {loc.begin, end->location(m_source).end};
  return at;
}

TypeDict* Parser::parse_type_dict()
{
  SAVE_FIRST();

  auto* dt = m_alloc.emplace<TypeDict>();
  dt->key = parse_type();

  expect(COLON, "parsing dictionary type");

  dt->val = parse_type();

  Token* end = expect(RCURLY, "terminating dictionary type");

  dt->loc = {
      first->location(m_source).begin,
      end->location(m_source).end,
  };

  return dt;
}

TypeFunc* Parser::parse_type_func()
{
  SAVE_FIRST()
  expect(LPAREN, "parsing function type parameter list");

  auto* fn = m_alloc.emplace<TypeFunc>();

  while (!match(RPAREN)) {
    fn->params.push_back(parse_parameter());
    expect(COMMA, "terminating function type parameter");
  }

  expect(ARROW, "parsing function type return type");

  fn->ret = parse_type();
  fn->loc = {loc.begin, fn->ret->loc.end};
  return fn;
}

Type* Parser::parse_type()
{
  Token* tok = peek();
  switch (tok->kind) {
    case NIL:
    case KW_BOOL:
    case KW_INT:
    case KW_FLOAT:
    case KW_STRING:
      return parse_type_builtin();
    case LBRACKET:
      return parse_type_array();
    case LCURLY:
      return parse_type_dict();
    case KW_FN:
      return parse_type_func();
    default:
      throw ParserError(tok->location(m_source),
                        "Unexpected token '{}' ({}) while parsing type "
                        "(expected builtin type, '[', '{{' or 'fn')",
                        magic_enum::enum_name(tok->kind), tok->to_string());
  }
}

StmtScope* Parser::parse_stmt_scope()
{
  SAVE_FIRST()

  auto scope = m_alloc.emplace<StmtScope>();

  if (first->kind == COLON) {
    scope->stmts.push_back(parse_stmt());
    scope->loc = {loc.begin, scope->stmts.back()->loc.end};
  } else if (first->kind == LCURLY) {
    while (!match(RCURLY)) {
      scope->stmts.push_back(parse_stmt());
    }

    advance();
  } else
    throw ParserError(loc, "Expected ':' or '{{' while parsing scope, got '{}'",
                      first->to_string());

  return scope;
}

StmtVarDecl* Parser::parse_stmt_var()
{
  SAVE_FIRST()

  auto vars = m_alloc.emplace<StmtVarDecl>();
  vars->lval = parse_lvalue();

  if (optional(COLON)) {
    vars->type = parse_type();
  }

  expect(EQUALS, "parsing variable declaration");

  vars->rval = parse_expr();
  vars->loc = {loc.begin, vars->rval->loc.end};

  optional(SEMICOLON);
  return vars;
}

StmtFor* Parser::parse_stmt_for()
{
  SAVE_FIRST()

  auto fors = m_alloc.emplace<StmtFor>();
  fors->init = parse_stmt_var();

  expect(COMMA, "parsing for statement");

  fors->target = parse_expr();

  expect(COMMA, "parsing for statement");

  fors->step = parse_expr();
  fors->br = parse_stmt_scope();
  fors->loc = {loc.begin, fors->br->loc.end};
  return fors;
}

StmtForEach* Parser::parse_stmt_foreach()
{
  SAVE_FIRST()

  auto fors = m_alloc.emplace<StmtForEach>();
  fors->lval = parse_lvalue();

  expect(KW_IN, "parsing for each statement");

  fors->iter = parse_expr();
  fors->br = parse_stmt_scope();
  fors->loc = {loc.begin, fors->br->loc.end};
  return fors;
}

StmtIf* Parser::parse_stmt_if()
{
  using Branch = StmtIf::Branch;

  SAVE_FIRST()

  Branch br;
  br.cnd = parse_expr();
  br.br = parse_stmt_scope();

  auto* ifs = m_alloc.emplace<StmtIf>();
  ifs->brs.push_back(br);
  ifs->loc = {loc.begin, br.br->loc.end};
  return ifs;
}

StmtWhile* Parser::parse_stmt_while()
{
  SAVE_FIRST()

  auto* whs = m_alloc.emplace<StmtWhile>();
  whs->cnd = parse_expr();
  whs->br = parse_stmt_scope();
  whs->loc = {loc.begin, whs->br->loc.end};
  return whs;
}

StmtAssign* Parser::parse_stmt_assign(Expr* lhs)
{
  auto as = m_alloc.emplace<StmtAssign>();
  as->lval = lhs;
  as->op = advance();
  as->rval = parse_expr();
  as->loc = {as->lval->loc.begin, as->rval->loc.end};
  optional(SEMICOLON);
  return as;
}

StmtReturn* Parser::parse_stmt_return()
{
  SAVE_FIRST()

  auto* ret = m_alloc.emplace<StmtReturn>();

  if (is_expr_start(peek()->kind)) {
    ret->expr = parse_expr();
    ret->loc = {loc.begin, ret->expr->loc.end};
  } else {
    ret->expr = nullptr;
    ret->loc = loc;
  }

  optional(SEMICOLON);
  return ret;
}

StmtEnum* Parser::parse_stmt_enum()
{
  SAVE_FIRST()

  auto ens = m_alloc.emplace<StmtEnum>();
  ens->sym = advance();

  if (optional(KW_OF)) {
    ens->type = parse_type();
  }

  expect(LCURLY, "parsing enumerator list");

  while (!match(RCURLY)) {
    Token* sym = advance();
    expect(EQUALS, "parsing enumerator pair");

    ens->pairs.push_back({
        .sym = sym,
        .expr = parse_expr(),
    });

    expect(COMMA, "parsing enumerator pair");
  }

  ens->loc = {loc.begin, advance()->location(m_source).end};
  return ens;
}

StmtModule* Parser::parse_stmt_module()
{
  SAVE_FIRST()

  auto* mod = m_alloc.emplace<StmtModule>();
  mod->sym = advance();

  expect(LCURLY, "parsing module body");

  while (true) {
    Token* tok = peek();
    if (tok->kind == RCURLY) {
      break;
    }

    switch (tok->kind) {
      case KW_VAR:
        mod->scp.push_back(parse_stmt_var());
        break;
      case KW_FN:
        mod->scp.push_back(parse_stmt_func());
        break;
      case KW_STRUCT:
        mod->scp.push_back(parse_stmt_struct());
        break;
      case KW_TYPE:
        mod->scp.push_back(parse_stmt_type());
        break;
      case KW_MODULE:
        mod->scp.push_back(parse_stmt_module());
        break;
      case KW_USING:
        mod->scp.push_back(parse_stmt_using());
        break;
      case KW_ENUM:
        mod->scp.push_back(parse_stmt_enum());
        break;
      default:
        throw ParserError(tok->location(m_source),
                          "Unexpected token '{}' ({}) while parsing module "
                          "statement (expected 'var', 'fn', 'struct', 'type', "
                          "'module', 'using' or 'enum')",
                          magic_enum::enum_name(tok->kind), tok->to_string());
    }
  }

  Token* last = expect(RCURLY, "terminating module body");
  mod->loc = {loc.begin, last->location(m_source).end};
  return mod;
}

StmtImport* Parser::parse_stmt_import()
{
  using TailKind = StmtImport::TailKind;

  SAVE_FIRST()

  usize end;
  auto imp = m_alloc.emplace<StmtImport>();
  imp->kind = TailKind::Import;

  while (true) {
    Token* tok = advance();

    if (tok->kind == IDENT) {
      imp->path.push_back(tok);

      if (match(DBCOLON)) {
        advance();
      } else {
        end = tok->location(m_source).end;
        break;
      }
    } else if (tok->kind == LCURLY) {
      imp->kind = TailKind::ImportCompound;

      while (!match(RCURLY)) {
        Token* member = expect(IDENT, "parsing compound import member");
        imp->tail.push_back(member);
        expect(COMMA, "parsing compound import");
      }

      Token* last = expect(RCURLY, "terminating compound import");
      end = last->location(m_source).end;
      break;
    } else if (tok->kind == ASTERISK) {
      imp->kind = TailKind::ImportAll;
      end = tok->location(m_source).end;
      break;
    } else {
      throw ParserError(tok->location(m_source),
                        "Unexpected token '{}' ({}) while parsing import path "
                        "(expected identifier, '{{' or '*')",
                        magic_enum::enum_name(tok->kind), tok->to_string());
    }
  }

  imp->loc = {loc.begin, end};
  optional(SEMICOLON);
  return imp;
}

StmtFunctionDecl* Parser::parse_stmt_func()
{
  SAVE_FIRST()

  auto* fn = m_alloc.emplace<StmtFunctionDecl>();
  fn->name = expect(IDENT, "parsing function name");

  expect(LPAREN, "parsing function parameter list");

  while (!match(RPAREN)) {
    fn->parms.push_back(parse_parameter());

    if (match(RPAREN)) {
      optional(COMMA);
      break;
    } else {
      expect(COMMA, "terminating function parameter");
    }
  }

  expect(RPAREN, "terminating function parameter list");

  if (optional(ARROW)) {
    fn->ret = parse_type();
  }

  fn->scp = parse_stmt_scope();
  fn->loc = {loc.begin, fn->scp->loc.end};
  return fn;
}

StmtStructDecl* Parser::parse_stmt_struct()
{
  SAVE_FIRST()

  auto* strc = m_alloc.emplace<StmtStructDecl>();
  strc->sp = parse_static_path();

  expect(LCURLY, "parsing struct body");

  while (!match(RCURLY)) {
    Token* tok = peek();
    switch (tok->kind) {
      case KW_VAR:
        strc->scp.push_back(parse_stmt_var());
        break;
      case KW_FN:
        strc->scp.push_back(parse_stmt_func());
        break;
      case KW_TYPE:
        strc->scp.push_back(parse_stmt_type());
        break;
      case KW_USING:
        strc->scp.push_back(parse_stmt_using());
        break;
      case KW_ENUM:
        strc->scp.push_back(parse_stmt_enum());
        break;
      default:
        throw ParserError(
            tok->location(m_source),
            "Unexpected token '{}' ({}) while parsing struct body (expected "
            "'var', 'fn', 'type', 'using' or 'enum')",
            tok->to_string(), magic_enum::enum_name(tok->kind));
    }
  }

  Token* last = expect(RCURLY, "terminating struct body");
  strc->loc = {loc.begin, last->location(m_source).end};
  return strc;
}

StmtTypeDecl* Parser::parse_stmt_type()
{
  SAVE_FIRST()

  auto* ty = m_alloc.emplace<StmtTypeDecl>();
  ty->sym = advance();

  expect(EQUALS, "parsing type declaration");

  ty->type = parse_type();
  ty->loc = {loc.begin, ty->type->loc.end};

  optional(SEMICOLON);
  return ty;
}

StmtUsing* Parser::parse_stmt_using()
{
  SAVE_FIRST();

  auto* usn = m_alloc.emplace<StmtUsing>();
  usn->sp = parse_static_path();
  usn->scp = parse_stmt_scope();
  usn->loc = {loc.begin, usn->scp->loc.end};
  return usn;
}

Stmt* Parser::parse_stmt()
{
  switch (peek()->kind) {
    case KW_IF:
      return parse_stmt_if();
    case KW_WHILE:
      return parse_stmt_while();
    case KW_VAR:
      return parse_stmt_var();
    case KW_DO:
      advance();
      return parse_stmt_scope();
    case KW_FOR:
      // generic for loop
      if (match(KW_VAR, 1))
        return parse_stmt_for();

      // for each loop
      return parse_stmt_foreach();
    case KW_RETURN:
      return parse_stmt_return();
    case KW_ENUM:
      return parse_stmt_enum();
    case KW_MODULE:
      return parse_stmt_module();
    case KW_IMPORT:
      return parse_stmt_import();
    case KW_FN:
      return parse_stmt_func();
    case KW_STRUCT:
      return parse_stmt_struct();
    case KW_TYPE:
      return parse_stmt_type();
    case KW_USING:
      return parse_stmt_using();
    case SEMICOLON: {
      auto empty = m_alloc.emplace<StmtEmpty>();
      empty->loc = advance()->location(m_source);
      return empty;
    }
    default:
      break;
  }

  Token* first;
  if ((first = peek(), !is_expr_start(first->kind)))
    throw ParserError(first->location(m_source),
                      "Unexpected token '{}' ({}) while parsing statement",
                      first->to_string(), magic_enum::enum_name(first->kind));

  Expr* expr = parse_expr();

  switch (peek()->kind) {
    case EQUALS:
    case PLUSEQUALS:
    case MINUSEQUALS:
    case ASTERISKEQUALS:
    case FSLASHEQUALS:
    case POWEQUALS:
    case PERCENTEQUALS:
    case PIPEEQUALS:
    case AMPERSANDEQUALS:
      return parse_stmt_assign(expr);
    default: {
      auto es = m_alloc.emplace<StmtExpr>();
      es->expr = expr;
      es->loc = es->expr->loc;
      optional(SEMICOLON);
      return es;
    }
  }
}

SyntaxTree Parser::parse()
{
  SyntaxTree nodes;

  while (!match(EOF_)) {
    try {
      nodes.push_back(parse_stmt());
    } catch (const ParserError& e) {
      m_diag.report<Diagnosis::Kind::Error>(e.loc, e.msg);
      break;
    }
  }

  return nodes;
}

}  // namespace via
