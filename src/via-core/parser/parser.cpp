/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "parser.h"
#include <magic_enum/magic_enum.hpp>

#define SAVE_FIRST()       \
  auto* first = advance(); \
  auto loc = first->location(mSource);

#define SAVE_FIRST_DONT_ADVANCE_THO() \
  auto* first = peek();               \
  auto loc = first->location(mSource);

using enum via::Token::Kind;
using namespace via::ast;

struct ParserError
{
 public:
  via::Diagnosis diag;

  template <typename... Args>
    requires std::is_constructible_v<via::Diagnosis, via::Level, Args...>
  explicit ParserError(Args&&... args) : diag(via::Level::ERROR, args...)
  {}
};

static bool isExprInitial(via::Token::Kind kind)
{
  switch (kind) {
    case IDENTIFIER:
    case LIT_INT:
    case LIT_BINT:
    case LIT_XINT:
    case LIT_NIL:
    case LIT_FLOAT:
    case LIT_STRING:
    case KW_NOT:
    case KW_FN:
    case PAREN_OPEN:
    case OP_MINUS:
    case OP_TILDE:
    case OP_AMP:
      return true;
    default:
      return false;
  }
}

static int binPrec(via::Token::Kind kind)
{
  switch (kind) {
    case KW_OR:
      return 0;
    case KW_AND:
      return 1;
    case OP_EQ_EQ:
    case OP_BANG_EQ:
    case OP_LT:
    case OP_LT_EQ:
    case OP_GT:
    case OP_GT_EQ:
      return 2;
    case OP_AMP:
      return 3;
    case OP_CARET:
      return 4;
    case OP_PIPE:
      return 5;
    case OP_SHL:
    case OP_SHR:
      return 6;
    case OP_PLUS:
    case OP_MINUS:
      return 7;
    case OP_STAR:
    case OP_SLASH:
    case OP_PERCENT:
      return 8;
    case OP_STAR_STAR:
      return 9;
    default:
      return -1;
  }
}

const via::Token* via::Parser::peek(int ahead)
{
  return mCursor[ahead];
}

const via::Token* via::Parser::advance()
{
  return *(mCursor++);
}

bool via::Parser::match(Token::Kind kind, int ahead)
{
  return peek(ahead)->kind == kind;
}

bool via::Parser::optional(Token::Kind kind)
{
  if (match(kind)) {
    advance();
    return true;
  }

  return false;
}

const via::Token* via::Parser::expect(Token::Kind kind, const char* task)
{
  if (!match(kind)) {
    const Token& unexp = *peek();
    throw ParserError(
      unexp.location(mSource),
      std::format("Unexpected token '{}' ({}) while {}", unexp.toString(),
                  magic_enum::enum_name(unexp.kind), task));
  }

  return advance();
}

const AccessIdent* via::Parser::parseAccessIdent()
{
  auto* aid = mAlloc.emplace<AccessIdent>();
  aid->symbol = expect(IDENTIFIER, "while parsing access identifier");

  if (match(COLON_COLON) && match(OP_LT, 1)) {
    advance();
    advance();

    if (!match(OP_GT)) {
      while (true) {
        aid->gens.push_back(parseType());

        if (match(OP_GT)) {
          break;
        } else {
          expect(COMMA, "parsing access identifier types");
        }
      }
    }

    auto* last = expect(OP_GT, "terminating access identifier generics");
    aid->inst = true;
    aid->loc = {aid->symbol->location(mSource).begin,
                last->location(mSource).end};
  } else {
    aid->inst = false;
    aid->loc = aid->symbol->location(mSource);
  }

  return aid;
}

const Path* via::Parser::parseStaticPath()
{
  auto* sp = mAlloc.emplace<Path>();

  while (true) {
    sp->path.push_back(expect(IDENTIFIER, "parsing static path"));

    if (match(COLON_COLON)) {
      advance();
    } else {
      break;
    }
  }

  sp->loc = {sp->path.front()->location(mSource).begin,
             sp->path.back()->location(mSource).end};
  return sp;
}

const Expr* via::Parser::parseLValue()
{
  const Expr* expr = parseExpr();
  if (isLValue(expr)) {
    return expr;
  } else {
    throw ParserError(expr->loc, "Unexpected expression while parsing lvalue");
  }
}

const Parameter* via::Parser::parseParameter()
{
  SAVE_FIRST()

  auto* par = mAlloc.emplace<Parameter>();
  par->symbol = first;

  if (optional(COLON)) {
    par->type = parseType();
    par->loc = {loc.begin, par->type->loc.end};
  } else {
    par->loc = loc;
  }

  return par;
}

const AttributeGroup* via::Parser::parseAttribGroup()
{
  SAVE_FIRST()
  expect(BRACKET_OPEN, "parsing attribute group");

  auto* atg = mAlloc.emplace<AttributeGroup>();

  while (true) {
    atg->ats.push_back({
      .sp = parseStaticPath(),
      .args = {},
    });

    if (match(BRACKET_CLOSE)) {
      break;
    } else {
      expect(COMMA, "parsing attribute group");
    }
  }

  auto* last = expect(BRACKET_CLOSE, "terminating attribute group");
  atg->loc = {loc.begin, last->location(mSource).end};
  return atg;
}

const ExprLit* via::Parser::parseExprLit()
{
  auto* lit = mAlloc.emplace<ExprLit>();
  lit->tok = advance();
  lit->loc = lit->tok->location(mSource);
  return lit;
}

const ExprSymbol* via::Parser::parseExprSymbol()
{
  auto* symbol = mAlloc.emplace<ExprSymbol>();
  symbol->symbol = advance();
  symbol->loc = symbol->symbol->location(mSource);
  return symbol;
}

const Expr* via::Parser::parseExprGroupOrTuple()
{
  auto loc = advance()->location(mSource);
  auto* first = parseExpr();

  if (match(COMMA)) {
    std::vector<const Expr*> vals;
    vals.push_back(first);

    while (match(COMMA)) {
      advance();
      vals.push_back(parseExpr());
    }

    expect(PAREN_CLOSE, "parsing tuple expression");

    auto* tup = mAlloc.emplace<ExprTuple>();
    tup->vals = std::move(vals);
    tup->loc = {loc.begin, peek(-1)->location(mSource).end};

    return tup;
  }

  expect(PAREN_CLOSE, "parsing grouping expression");

  auto* group = mAlloc.emplace<ExprGroup>();
  group->expr = first;
  group->loc = {loc.begin, peek(-1)->location(mSource).end};
  return group;
}

const ExprUnary* via::Parser::parseExprUnary(const ast::Expr* expr)
{
  auto* un = mAlloc.emplace<ExprUnary>();
  un->op = advance();
  un->expr = parseExprAffix();
  un->loc = {un->op->location(mSource).begin, un->expr->loc.end};
  return un;
}

const ExprDynAccess* via::Parser::parseExprDynAccess(const ast::Expr* expr)
{
  advance();  // consume '.'

  auto* da = mAlloc.emplace<ExprDynAccess>();
  da->root = expr;
  da->index = expect(IDENTIFIER, "parsing dynamic access specifier");
  da->loc = {da->root->loc.begin, da->index->location(mSource).end};
  return da;
}

const ExprStaticAccess* via::Parser::parseExprStAccess(const ast::Expr* expr)
{
  advance();  // consume '::'

  auto* sa = mAlloc.emplace<ExprStaticAccess>();
  sa->root = expr;
  sa->index = expect(IDENTIFIER, "parsing static access specifier");
  sa->loc = {sa->root->loc.begin, sa->index->location(mSource).end};
  return sa;
}

const ExprCall* via::Parser::parseExprCall(const ast::Expr* expr)
{
  advance();  // consume '('

  std::vector<const Expr*> args;

  if (!match(PAREN_CLOSE)) {
    do
      args.push_back(parseExpr());
    while (match(COMMA) && advance());

    expect(PAREN_CLOSE, "parsing function call");
  } else
    advance();  // consume ')'

  auto* call = mAlloc.emplace<ExprCall>();
  call->lval = expr;
  call->args = std::move(args);
  call->loc = {expr->loc.begin, peek(-1)->location(mSource).end};
  return call;
}

const ExprSubscript* via::Parser::parseExprSubscript(const ast::Expr* expr)
{
  advance();  // consume '['

  const Expr* idx = parseExpr();

  expect(BRACKET_CLOSE, "parsing subscript expression");

  auto* subs = mAlloc.emplace<ExprSubscript>();
  subs->lval = expr;
  subs->idx = idx;
  subs->loc = {expr->loc.begin, peek(-1)->location(mSource).end};
  return subs;
}

const ExprCast* via::Parser::parseExprCast(const ast::Expr* expr)
{
  advance();

  auto* cast = mAlloc.emplace<ExprCast>();
  cast->expr = expr;
  cast->type = parseType();
  cast->loc = {expr->loc.begin, cast->type->loc.end};
  return cast;
}

const ExprTernary* via::Parser::parseExprTernary(const ast::Expr* expr)
{
  advance();

  auto* tern = mAlloc.emplace<ExprTernary>();
  tern->lhs = expr;
  tern->cnd = parseExpr();

  expect(KW_ELSE, "parsing ternary expression");

  tern->rhs = parseExpr();
  tern->loc = {expr->loc.begin, tern->rhs->loc.end};
  return tern;
}

const ExprArray* via::Parser::parseExprArray()
{
  auto loc = peek()->location(mSource);
  auto* arr = mAlloc.emplace<ExprArray>();

  if (!match(BRACKET_CLOSE)) {
    while (true) {
      arr->init.push_back(parseExpr());

      if (match(BRACKET_CLOSE)) {
        optional(COMMA);  // trailing comma
        break;
      } else {
        expect(COMMA, "parsing array initializer");
      }
    }
  }

  auto* last = expect(BRACKET_CLOSE, "terminating array initializer");
  arr->loc = {loc.begin, last->location(mSource).end};
  return arr;
}

const ExprLambda* via::Parser::parseExprLambda()
{
  auto loc = peek()->location(mSource);
  auto* fn = mAlloc.emplace<ExprLambda>();

  expect(PAREN_OPEN, "parsing lambda parameter list");

  if (!match(PAREN_CLOSE)) {
    while (true) {
      fn->pms.push_back(parseParameter());

      if (match(PAREN_CLOSE)) {
        break;
      } else {
        expect(COMMA, "parsing lambda parameter list");
      }
    }

    expect(PAREN_CLOSE, "terminating lambda parameter list");
  }

  fn->scope = parseStmtScope();
  fn->loc = {loc.begin, fn->scope->loc.end};
  return fn;
}

const Expr* via::Parser::parseExprPrimary()
{
  SAVE_FIRST_DONT_ADVANCE_THO()

  switch (first->kind) {
    // Literal expression
    case LIT_INT:
    case LIT_BINT:
    case LIT_XINT:
    case LIT_NIL:
    case LIT_FLOAT:
    case LIT_TRUE:
    case LIT_FALSE:
    case LIT_STRING:
      return parseExprLit();
    case IDENTIFIER:
      return parseExprSymbol();
    case PAREN_OPEN:
      return parseExprGroupOrTuple();
    case BRACKET_OPEN:
      return parseExprArray();
    case KW_FN:
      return parseExprLambda();
    default:
      throw ParserError(
        loc,
        std::format(
          "Unexpected token '{}' ({}) while parsing primary expression",
          first->toString(), magic_enum::enum_name(first->kind)),
        Footnote(Footnote::Kind::HINT,
                 "Expected INT | BINARY_INT | HEX_INT | 'nil' | FLOAT | 'true' "
                 "| 'false' | "
                 "STRING | IDENTIFIER | '(' | ')' | 'fn'"));
  }
}

const Expr* via::Parser::parseExprAffix()
{
  const Expr* expr;

  switch (peek()->kind) {
    case KW_NOT:
    case OP_MINUS:
    case OP_TILDE:
    case OP_AMP:
      expr = parseExprUnary(expr);
      break;
    default:
      expr = parseExprPrimary();
      break;
  }

  while (true) {
    switch (peek()->kind) {
      case KW_AS:
        expr = parseExprCast(expr);
        break;
      case KW_IF:
        expr = parseExprTernary(expr);
        break;
      case PAREN_OPEN:
        expr = parseExprCall(expr);
        break;
      case BRACKET_OPEN:
        expr = parseExprSubscript(expr);
        break;
      case PERIOD:
        expr = parseExprDynAccess(expr);
        break;
      case COLON_COLON:
        expr = parseExprStAccess(expr);
        break;
      default:
        return expr;
    }
  }
}

const Expr* via::Parser::parseExpr(int minPrec)
{
  const Expr* lhs = parseExprAffix();

  int prec;
  while ((prec = binPrec(peek()->kind), prec >= minPrec)) {
    auto bin = mAlloc.emplace<ExprBinary>();
    bin->op = advance();
    bin->lhs = lhs;
    bin->rhs = parseExpr(prec + 1);
    bin->loc = {lhs->loc.begin, bin->rhs->loc.end};
    lhs = bin;
  }

  return lhs;
}

const TypeBuiltin* via::Parser::parseTypeBuiltin()
{
  SAVE_FIRST()

  auto* bt = mAlloc.emplace<TypeBuiltin>();
  bt->tok = first;
  bt->loc = loc;
  return bt;
}

const TypeArray* via::Parser::parseTypeArray()
{
  SAVE_FIRST();

  auto* at = mAlloc.emplace<TypeArray>();
  at->type = parseType();

  auto* end = expect(BRACKET_CLOSE, "terminating array type");

  at->loc = {loc.begin, end->location(mSource).end};
  return at;
}

const TypeDict* via::Parser::parseTypeDict()
{
  SAVE_FIRST();

  auto* dt = mAlloc.emplace<TypeDict>();
  dt->key = parseType();

  expect(COLON, "parsing dictionary type");

  dt->val = parseType();

  auto* end = expect(BRACE_CLOSE, "terminating dictionary type");

  dt->loc = {first->location(mSource).begin, end->location(mSource).end};
  return dt;
}

const TypeFunc* via::Parser::parseTypeFunc()
{
  SAVE_FIRST()
  expect(PAREN_OPEN, "parsing function type parameter list");

  auto* fn = mAlloc.emplace<TypeFunc>();

  while (!match(PAREN_CLOSE)) {
    fn->params.push_back(parseParameter());
    expect(COMMA, "terminating function type parameter");
  }

  expect(ARROW, "parsing function type return type");

  fn->ret = parseType();
  fn->loc = {loc.begin, fn->ret->loc.end};
  return fn;
}

const Type* via::Parser::parseType()
{
  auto* tok = peek();
  switch (tok->kind) {
    case LIT_NIL:
    case KW_BOOL:
    case KW_INT:
    case KW_FLOAT:
    case KW_STRING:
      return parseTypeBuiltin();
    case BRACKET_OPEN:
      return parseTypeArray();
    case BRACE_OPEN:
      return parseTypeDict();
    case KW_FN:
      return parseTypeFunc();
    default:
      throw ParserError(
        tok->location(mSource),
        std::format("Unexpected token '{}' ({}) while parsing type",
                    tok->toString(), magic_enum::enum_name(tok->kind)),
        Footnote(Footnote::Kind::HINT,
                 "Expected 'nil' | 'bool' | 'int' | 'float' | "
                 "'string' | '[' | '{' | 'fn'"));
  }
}

const StmtScope* via::Parser::parseStmtScope()
{
  SAVE_FIRST()

  auto scope = mAlloc.emplace<StmtScope>();

  if (first->kind == COLON) {
    scope->stmts.push_back(parseStmt());
    scope->loc = {loc.begin, scope->stmts.back()->loc.end};
  } else if (first->kind == BRACE_OPEN) {
    while (!match(BRACE_CLOSE)) {
      scope->stmts.push_back(parseStmt());
    }

    auto* last = advance();
    scope->loc = {
      first->location(mSource).begin,
      last->location(mSource).end,
    };
  } else
    throw ParserError(loc,
                      std::format("Unexpected token '{}' while parsing scope",
                                  first->toString()),
                      Footnote(Footnote::Kind::HINT, "Expected ':' | '{'"));

  return scope;
}

const StmtVarDecl* via::Parser::parseStmtVarDecl(bool semicolon)
{
  SAVE_FIRST()

  auto vars = mAlloc.emplace<StmtVarDecl>();
  vars->decl = first;
  vars->lval = parseLValue();

  if (optional(COLON)) {
    vars->type = parseType();
  } else {
    vars->type = nullptr;
  }

  expect(OP_EQ, "parsing variable declaration");

  vars->rval = parseExpr();
  vars->loc = {loc.begin, vars->rval->loc.end};

  if (semicolon) {
    optional(SEMICOLON);
  }

  return vars;
}

const StmtFor* via::Parser::parseStmtFor()
{
  SAVE_FIRST()

  auto fors = mAlloc.emplace<StmtFor>();
  fors->init = parseStmtVarDecl(false);

  if (fors->init->decl->kind == KW_CONST) {
    throw ParserError(fors->init->decl->location(mSource),
                      "'const' variable not allowed in ranged for loop");
  }

  expect(COMMA, "parsing ranged for loop");

  fors->target = parseExpr();

  if (match(COMMA)) {
    advance();
    fors->step = parseExpr();
  }

  fors->br = parseStmtScope();
  fors->loc = {loc.begin, fors->br->loc.end};
  return fors;
}

const StmtForEach* via::Parser::parseStmtForEach()
{
  SAVE_FIRST()

  auto fors = mAlloc.emplace<StmtForEach>();
  fors->lval = parseLValue();

  expect(KW_IN, "parsing for each statement");

  fors->iter = parseExpr();
  fors->br = parseStmtScope();
  fors->loc = {loc.begin, fors->br->loc.end};
  return fors;
}

const StmtIf* via::Parser::parseStmtIf()
{
  using Branch = StmtIf::Branch;

  SAVE_FIRST()

  Branch br;
  br.cnd = parseExpr();
  br.br = parseStmtScope();

  auto* ifs = mAlloc.emplace<StmtIf>();
  ifs->brs.push_back(br);

  while (match(KW_ELSE)) {
    advance();

    Branch br;

    if (match(KW_IF)) {
      advance();
      br.cnd = parseExpr();
    } else {
      br.cnd = nullptr;
    }

    br.br = parseStmtScope();
    ifs->brs.push_back(br);
  }

  ifs->loc = {loc.begin, br.br->loc.end};
  return ifs;
}

const StmtWhile* via::Parser::parseStmtWhile()
{
  SAVE_FIRST()

  auto* whs = mAlloc.emplace<StmtWhile>();
  whs->cnd = parseExpr();
  whs->br = parseStmtScope();
  whs->loc = {loc.begin, whs->br->loc.end};
  return whs;
}

const StmtAssign* via::Parser::parseStmtAssign(const Expr* expr)
{
  auto as = mAlloc.emplace<StmtAssign>();
  as->lval = expr;
  as->op = advance();
  as->rval = parseExpr();
  as->loc = {as->lval->loc.begin, as->rval->loc.end};
  optional(SEMICOLON);
  return as;
}

const StmtReturn* via::Parser::parseStmtReturn()
{
  SAVE_FIRST()

  auto* ret = mAlloc.emplace<StmtReturn>();

  if (isExprInitial(peek()->kind)) {
    ret->expr = parseExpr();
    ret->loc = {loc.begin, ret->expr->loc.end};
  } else {
    ret->expr = nullptr;
    ret->loc = loc;
  }

  optional(SEMICOLON);
  return ret;
}

const StmtEnum* via::Parser::parseStmtEnum()
{
  SAVE_FIRST()

  auto ens = mAlloc.emplace<StmtEnum>();
  ens->symbol = advance();

  if (optional(KW_OF)) {
    ens->type = parseType();
  }

  expect(BRACE_OPEN, "parsing enumerator list");

  while (!match(BRACE_CLOSE)) {
    auto* symbol = advance();
    expect(OP_EQ, "parsing enumerator pair");

    ens->pairs.push_back({
      .symbol = symbol,
      .expr = parseExpr(),
    });

    expect(COMMA, "parsing enumerator pair");
  }

  ens->loc = {loc.begin, advance()->location(mSource).end};
  return ens;
}

const StmtModule* via::Parser::parseStmtModule()
{
  SAVE_FIRST()

  auto* mod = mAlloc.emplace<StmtModule>();
  mod->symbol = advance();

  expect(BRACE_OPEN, "parsing module body");

  if (!match(BRACE_CLOSE)) {
    while (true) {
      auto* tok = peek();
      switch (tok->kind) {
        case KW_CONST:
        case KW_VAR:
          mod->scope.push_back(parseStmtVarDecl(true));
          break;
        case KW_FN:
          mod->scope.push_back(parseStmtFuncDecl());
          break;
        case KW_STRUCT:
          mod->scope.push_back(parseStmtStructDecl());
          break;
        case KW_TYPE:
          mod->scope.push_back(parseStmtTypeDecl());
          break;
        case KW_MODULE:
          mod->scope.push_back(parseStmtModule());
          break;
        case KW_USING:
          mod->scope.push_back(parseStmtUsingDecl());
          break;
        case KW_ENUM:
          mod->scope.push_back(parseStmtEnum());
          break;
        default:
          throw ParserError(
            tok->location(mSource),
            std::format("Unexpected token '{}' ({}) while parsing module",
                        tok->toString(), magic_enum::enum_name(tok->kind)));
      }
    }
  }

  auto* last = expect(BRACE_CLOSE, "terminating module body");
  mod->loc = {loc.begin, last->location(mSource).end};
  return mod;
}

const StmtImport* via::Parser::parseStmtImport()
{
  using TailKind = StmtImport::TailKind;

  SAVE_FIRST()

  usize end;
  auto imp = mAlloc.emplace<StmtImport>();
  imp->kind = TailKind::Import;

  while (true) {
    auto* tok = advance();

    if (tok->kind == IDENTIFIER) {
      imp->path.push_back(tok);

      if (match(COLON_COLON)) {
        advance();
      } else {
        end = tok->location(mSource).end;
        break;
      }
    } else if (tok->kind == BRACE_OPEN) {
      imp->kind = TailKind::ImportCompound;

      while (!match(BRACE_CLOSE)) {
        auto* member = expect(IDENTIFIER, "parsing compound import member");
        imp->tail.push_back(member);
        expect(COMMA, "parsing compound import");
      }

      auto* last = expect(BRACE_CLOSE, "terminating compound import");
      end = last->location(mSource).end;
      break;
    } else if (tok->kind == OP_STAR) {
      imp->kind = TailKind::ImportAll;
      end = tok->location(mSource).end;
      break;
    } else {
      throw ParserError(
        tok->location(mSource),
        std::format("Unexpected token '{}' ({}) while parsing import path",
                    tok->toString(), magic_enum::enum_name(tok->kind)));
    }
  }

  imp->loc = {loc.begin, end};
  optional(SEMICOLON);
  return imp;
}

const StmtFunctionDecl* via::Parser::parseStmtFuncDecl()
{
  SAVE_FIRST()

  auto* fn = mAlloc.emplace<StmtFunctionDecl>();
  fn->name = expect(IDENTIFIER, "parsing function name");

  expect(PAREN_OPEN, "parsing function parameter list");

  while (!match(PAREN_CLOSE)) {
    fn->parms.push_back(parseParameter());

    if (match(PAREN_CLOSE)) {
      optional(COMMA);
      break;
    } else {
      expect(COMMA, "terminating function parameter");
    }
  }

  expect(PAREN_CLOSE, "terminating function parameter list");

  if (optional(ARROW)) {
    fn->ret = parseType();
  } else {
    fn->ret = nullptr;
  }

  fn->scope = parseStmtScope();
  fn->loc = {loc.begin, fn->scope->loc.end};
  return fn;
}

const StmtStructDecl* via::Parser::parseStmtStructDecl()
{
  SAVE_FIRST()

  auto* strc = mAlloc.emplace<StmtStructDecl>();
  strc->name = expect(IDENTIFIER, "parsing struct name");

  expect(BRACE_OPEN, "parsing struct body");

  while (!match(BRACE_CLOSE)) {
    auto* tok = peek();
    switch (tok->kind) {
      case KW_CONST:
      case KW_VAR:
        strc->scope.push_back(parseStmtVarDecl(false));
        expect(COMMA, "terminating struct member");
        break;
      case KW_FN:
        strc->scope.push_back(parseStmtFuncDecl());
        break;
      case KW_TYPE:
        strc->scope.push_back(parseStmtTypeDecl());
        expect(COMMA, "terminating struct member");
        break;
      case KW_USING:
        strc->scope.push_back(parseStmtUsingDecl());
        break;
      case KW_ENUM:
        strc->scope.push_back(parseStmtEnum());
        break;
      default:
        throw ParserError(
          tok->location(mSource),
          std::format("Unexpected token '{}' ({}) while parsing struct body",
                      tok->toString(), magic_enum::enum_name(tok->kind)));
    }
  }

  auto* last = expect(BRACE_CLOSE, "terminating struct body");
  strc->loc = {loc.begin, last->location(mSource).end};
  return strc;
}

const StmtTypeDecl* via::Parser::parseStmtTypeDecl()
{
  SAVE_FIRST()

  auto* ty = mAlloc.emplace<StmtTypeDecl>();
  ty->symbol = advance();

  expect(OP_EQ, "parsing type declaration");

  ty->type = parseType();
  ty->loc = {loc.begin, ty->type->loc.end};

  optional(SEMICOLON);
  return ty;
}

const StmtUsing* via::Parser::parseStmtUsingDecl()
{
  SAVE_FIRST();

  auto* usn = mAlloc.emplace<StmtUsing>();
  usn->sp = parseStaticPath();
  usn->scope = parseStmtScope();
  usn->loc = {loc.begin, usn->scope->loc.end};
  return usn;
}

const Stmt* via::Parser::parseStmt()
{
  switch (peek()->kind) {
    case KW_IF:
      return parseStmtIf();
    case KW_WHILE:
      return parseStmtWhile();
    case KW_VAR:
    case KW_CONST:
      return parseStmtVarDecl(true);
    case KW_DO:
      advance();
      return parseStmtScope();
    case KW_FOR:
      // generic for loop
      if (match(KW_VAR, 1))
        return parseStmtFor();

      // for each loop
      return parseStmtForEach();
    case KW_RETURN:
      return parseStmtReturn();
    case KW_ENUM:
      return parseStmtEnum();
    case KW_MODULE:
      return parseStmtModule();
    case KW_IMPORT:
      return parseStmtImport();
    case KW_FN:
      return parseStmtFuncDecl();
    case KW_STRUCT:
      return parseStmtStructDecl();
    case KW_TYPE:
      return parseStmtTypeDecl();
    case KW_USING:
      return parseStmtUsingDecl();
    case SEMICOLON: {
      auto empty = mAlloc.emplace<StmtEmpty>();
      empty->loc = advance()->location(mSource);
      return empty;
    }
    default:
      break;
  }

  const Token* first = peek();
  if (!isExprInitial(first->kind)) {
  unexpected_token:
    throw ParserError(
      first->location(mSource),
      std::format("Unexpected token '{}' ({}) while parsing statement",
                  first->toString(), magic_enum::enum_name(first->kind)));
  }

  const Expr* expr = parseExpr();

  switch (peek()->kind) {
    case OP_EQ:
    case OP_PLUS_EQ:
    case OP_MINUS_EQ:
    case OP_STAR_EQ:
    case OP_SLASH_EQ:
    case OP_STAR_STAR_EQ:
    case OP_PERCENT_EQ:
    case OP_PIPE_EQ:
    case OP_AMP_EQ:
      return parseStmtAssign(expr);
    default: {
      auto es = mAlloc.emplace<StmtExpr>();
      es->expr = expr;
      es->loc = es->expr->loc;

      if TRY_COERCE (const ExprCall, _, expr) {
        goto valid_expr_stmt;
      } else {
        goto unexpected_token;
      }

    valid_expr_stmt:
      optional(SEMICOLON);
      return es;
    }
  }
}

via::SyntaxTree via::Parser::parse()
{
  SyntaxTree nodes;

  while (!match(EOF_)) {
    try {
      nodes.push_back(parseStmt());
    } catch (const ParserError& e) {
      mDiag.report(e.diag);
      break;
    }
  }

  return nodes;
}
