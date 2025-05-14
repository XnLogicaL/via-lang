// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "parser.h"

namespace via {

using enum CErrorLevel;
using enum TokenType;

AstNode* Parser::alloc_node(LexLocation loc, AstKind kind, AstNode::Un u) {
  return lctx.astalloc.emplace<AstNode>(AstNode{
    .loc = {.line = loc.line, .offset = loc.offset, .begin = loc.begin, .end = current()->loc.end},
    .kind = kind,
    .u = u,
  });
}

Token* Parser::current() {
  return peek(0);
}

Token* Parser::peek(int32_t ahead) {
  if (position + ahead >= lctx.tokens.size()) {
    throw ParserError{true, {}, "Unexpected end of file"};
  }

  return &lctx.tokens.at(position + ahead);
}

Token* Parser::consume(uint32_t ahead) {
  uint64_t new_pos = position + static_cast<uint64_t>(ahead);
  if (new_pos >= lctx.tokens.size()) {
    throw ParserError{true, {}, "Unexpected end of file"};
  }

  position = new_pos;
  return peek(-static_cast<int32_t>(ahead));
}

Token* Parser::expect_consume(TokenType type, const std::string& what) {
  Token* curr = current();
  if (curr->type != type) {
    throw ParserError{false, curr->loc, what};
  }

  return consume();
}

int* Parser::parse_modifiers() {
  int* modfs = lctx.semalloc.emplace<int>(0);

  while (true) {
    Token* curr = current();
    if (curr->type == KW_CONST) {
      if (*modfs & F_SEMA_CONST)
        err_bus.log({
          curr->loc,
          false,
          "Modifier 'const' found multiple times",
          WARNING,
          lctx,
        });

      *modfs |= F_SEMA_CONST;
      consume();
    }
    else
      break;
  }

  return modfs;
}

Attribute* Parser::parse_attribute() {
  expect_consume(AT, "Expected '@' to denote attribute");
  Token* ident = expect_consume(IDENTIFIER, "Expected identifier for attribute name");

  std::vector<const char*> buf;

  if (current()->type == PAREN_OPEN) {
    consume();

    while (true) {
      if (current()->type == PAREN_CLOSE) {
        break;
      }

      Token* tok = consume();
      char* str = sema::alloc_string(lctx.stralloc, tok->lexeme);
      buf.push_back(str);
    }

    consume();
  }

  Attribute* attr = lctx.astalloc.emplace<Attribute>();
  attr->ident = sema::alloc_string(lctx.stralloc, ident->lexeme);
  attr->args = sema::alloc_array(lctx.semalloc, buf);
  attr->argc = buf.size();

  return attr;
}

AstNode* Parser::parse_generic() {
  Token* ident = consume();
  std::vector<AstNode*> buf;

  expect_consume(OP_LT, "Expected '<' to open type generic");

  while (current()->type != OP_GT) {
    buf.push_back(parse_type());

    if (current()->type != OP_GT) {
      expect_consume(COMMA, "Expected ',' to seperate type generics");
    }
  }

  expect_consume(OP_GT, "Expected '>' to close type generic");

  NodeGenType gen_node;
  gen_node.id = sema::alloc_string(lctx.stralloc, ident->lexeme);
  gen_node.gens = sema::alloc_array(lctx.astalloc, buf);
  gen_node.genc = buf.size();

  return alloc_node(ident->loc, AstKind::TYPE_Gen, {.t_gen = gen_node});
}

AstNode* Parser::parse_type_primary() {
  static std::unordered_map<std::string, Value::Tag> primitive_map = {
    {"nil", Value::Tag::Nil},
    {"int", Value::Tag::Int},
    {"float", Value::Tag::Float},
    {"bool", Value::Tag::Bool},
    {"string", Value::Tag::String},
  };

  Token* tok = current();
  switch (tok->type) {
  case KW_AUTO:
    consume();
    return alloc_node(tok->loc, AstKind::TYPE_Auto, {});
  case IDENTIFIER:
  case LIT_NIL: {
    auto it = primitive_map.find(tok->lexeme);
    if (it != primitive_map.end()) {
      NodePrimType prim;
      prim.id = sema::alloc_string(lctx.stralloc, tok->lexeme);
      prim.type = it->second;

      return alloc_node(tok->loc, AstKind::TYPE_Prim, {.t_prim = prim});
    }

    return parse_generic();
  }
  case PAREN_OPEN: {
    consume();
    std::vector<Parameter> buf;

    while (true) {
      Token* tok = current();
      if (tok->type == PAREN_CLOSE) {
        consume();

        NodePrimType prim;
        prim.type = Value::Tag::Nil;
        prim.id = sema::alloc_string(lctx.stralloc, tok->lexeme);

        return alloc_node(tok->loc, AstKind::TYPE_Prim, {.t_prim = prim});
      }

      Parameter par;
      par.id = sema::alloc_string(lctx.stralloc, tok->lexeme);
      par.type = parse_type();

      buf.push_back(par);

      if (tok->type != PAREN_CLOSE) {
        expect_consume(COMMA, "Expected ',' to seperate function parameter types");
      }
    }

    expect_consume(PAREN_CLOSE, "Expected ')' to close function type parameters");
    expect_consume(RETURNS, "Expected '->' to specify function return type");

    NodeFuncType fnty;
    fnty.rets = parse_type();
    fnty.params = sema::alloc_array(lctx.astalloc, buf);
    fnty.paramc = buf.size();

    return alloc_node(tok->loc, AstKind::TYPE_Fun, {.t_fun = fnty});
  }
  case BRACKET_OPEN: {
    consume();

    NodeArrType arrt;
    arrt.type = parse_type();

    expect_consume(BRACKET_CLOSE, "Expected ']' to close array type");
    return alloc_node(tok->loc, AstKind::TYPE_Arr, {.t_arr = arrt});
  }
  default: {
    throw ParserError{
      false,
      tok->loc,
      std::format("Unexpected token '{}' while parsing type", tok->lexeme),
    };
  }
  }

  VIA_UNREACHABLE();
}

AstNode* Parser::parse_type_postfix() {
  using enum TokenType;

  AstNode* type = parse_type_primary();
  Token* tok = current();

  switch (tok->type) {
  case QUESTION:
    consume();

    NodeOptType opt;
    opt.type = type;

    return alloc_node(tok->loc, AstKind::TYPE_Opt, {.t_opt = opt});
  default:
    break;
  }

  return type;
}

AstNode* Parser::parse_type() {
  return parse_type_postfix();
}

AstNode* Parser::parse_primary() {
  Token* tok = current();
  switch (tok->type) {
  case LIT_INT:
  case LIT_HEX: {
    consume();

    NodeLitExpr lit;
    lit.kind = Value::Tag::Int;
    lit.u = {.i = std::stoi(tok->lexeme)};

    return alloc_node(tok->loc, AstKind::EXPR_Lit, {.e_lit = lit});
  }
  case LIT_FLOAT: {
    consume();

    NodeLitExpr lit;
    lit.kind = Value::Tag::Float;
    lit.u = {.f = std::stof(tok->lexeme)};

    return alloc_node(tok->loc, AstKind::EXPR_Lit, {.e_lit = lit});
  }
  case LIT_BINARY: {
    consume();

    NodeLitExpr lit;
    lit.kind = Value::Tag::Int;
    lit.u = {.i = (int)std::bitset<64>(tok->lexeme.substr(2)).to_ullong()};

    return alloc_node(tok->loc, AstKind::EXPR_Lit, {.e_lit = lit});
  }
  case LIT_NIL: {
    consume();

    NodeLitExpr lit;
    lit.kind = Value::Tag::Nil;

    return alloc_node(tok->loc, AstKind::EXPR_Lit, {.e_lit = lit});
  }
  case LIT_BOOL: {
    consume();

    NodeLitExpr lit;
    lit.kind = Value::Tag::Bool;
    lit.u = {.b = tok->lexeme == "true"};

    return alloc_node(tok->loc, AstKind::EXPR_Lit, {.e_lit = lit});
  }
  case IDENTIFIER: {
    Token* id = consume();

    // Check for intrinsic
    if (id->lexeme == "deep_eq") {
      expect_consume(PAREN_OPEN, "Expected '(' to open intrinsic arguments for 'deep_eq'");
      AstNode* left = parse_expr();
      expect_consume(COMMA, "Expected ',' to seperate intrinsic 'deep_eq' arguments");
      AstNode* right = parse_expr();
      expect_consume(PAREN_CLOSE, "Expected ')' to close intrinsic arguments for 'deep_eq'");

      std::vector<AstNode*> buf;
      buf.push_back(left);
      buf.push_back(right);

      NodeIntrExpr intr;
      intr.id = "deep_eq";
      intr.exprs = sema::alloc_array(lctx.astalloc, buf);
      intr.exprc = buf.size();

      return alloc_node(id->loc, AstKind::EXPR_Intr, {.e_intr = intr});
    }

    NodeSymExpr sym;
    sym.symbol = sema::alloc_string(lctx.stralloc, id->lexeme);

    return alloc_node(tok->loc, AstKind::EXPR_Sym, {.e_sym = sym});
  }
  case LIT_STRING: {
    consume();

    NodeLitExpr lit;
    lit.kind = Value::Tag::String;
    lit.u = {.s = sema::alloc_string(lctx.stralloc, tok->lexeme)};

    return alloc_node(tok->loc, AstKind::EXPR_Lit, {.e_lit = lit});
  }
  case OP_INC:
  case OP_DEC:
  case OP_LEN:
  case OP_SUB: {
    consume();
    AstNode* expr = parse_primary();

    NodeUnExpr un;
    un.expr = expr;
    un.op = tok->type;

    return alloc_node(tok->loc, AstKind::EXPR_Un, {.e_un = un});
  }
  case BRACKET_OPEN: {
    consume();
    std::vector<AstNode*> buf;

    while (true) {
      if (current()->type == BRACKET_CLOSE) {
        break;
      }

      buf.push_back(parse_expr());
      expect_consume(COMMA, "Expected ',' to seperate list elements");
    }

    NodeArrExpr arr;
    arr.vals = sema::alloc_array(lctx.astalloc, buf);
    arr.valc = buf.size();

    return alloc_node(tok->loc, AstKind::EXPR_Arr, {.e_arr = arr});
  }
  case KW_TYPE:
  case KW_TYPEOF:
  case KW_NAMEOF:
  case KW_PRINT:
  case KW_ERROR:
  case KW_TRY: {
    consume();

    NodeIntrExpr intr;
    intr.id = sema::alloc_string(lctx.stralloc, tok->lexeme);
    intr.exprs = sema::alloc_array<AstNode*>(lctx.astalloc, 1);
    intr.exprc = 1;
    intr.exprs[0] = parse_expr();

    return alloc_node(tok->loc, AstKind::EXPR_Intr, {.e_intr = intr});
  }
  case PAREN_OPEN: {
    consume();

    AstNode* expr = parse_expr();
    expect_consume(PAREN_CLOSE, "Expected ')' to close grouping expression");

    NodeGroupExpr grp;
    grp.expr = expr;

    return alloc_node(tok->loc, AstKind::EXPR_Group, {.e_grp = grp});
  }
  default:
    throw ParserError{
      false,
      tok->loc,
      std::format("Unexpected token '{}' while parsing expression", tok->lexeme),
    };
  }

  VIA_UNREACHABLE();
}

AstNode* Parser::parse_postfix(AstNode* lhs) {
  // Process all postfix expressions (member access, array indexing, function calls, type casts)
  while (true) {
    Token* curr = current();
    switch (curr->type) {
      // non-static member access: obj.property
    case DOT: {
      consume();
      Token* id = expect_consume(IDENTIFIER, "Expected identifier while parsing index expression");

      NodeSymExpr sym;
      sym.symbol = sema::alloc_string(lctx.stralloc, id->lexeme);

      NodeIndexExpr idx;
      idx.obj = lhs;
      idx.idx = alloc_node(id->loc, AstKind::EXPR_Sym, {.e_sym = sym});

      lhs = alloc_node(lhs->loc, AstKind::EXPR_Idx, {.e_idx = idx});
      continue;
    }
      // Array indexing: obj[expr]
    case BRACKET_OPEN: {
      consume();

      AstNode* expr = parse_expr();
      expect_consume(BRACKET_CLOSE, "Expected ']' to close index expression");

      NodeIndexExpr idx;
      idx.obj = lhs;
      idx.idx = expr;

      lhs = alloc_node(lhs->loc, AstKind::EXPR_Idx, {.e_idx = idx});
      continue;
    }
    // Function calls: func(arg1, ...)
    case PAREN_OPEN: {
      consume();
      std::vector<AstNode*> buf;

      while (true) {
        if (current()->type == PAREN_CLOSE) {
          break;
        }

        buf.push_back(parse_expr());
        expect_consume(COMMA, "Expected ',' after function call argument");
      }

      expect_consume(PAREN_CLOSE, "Expected ')' to close function call arguments");

      NodeCallExpr call;
      call.callee = lhs;
      call.args = sema::alloc_array(lctx.astalloc, buf);
      call.argc = buf.size();

      lhs = alloc_node(lhs->loc, AstKind::EXPR_Call, {.e_call = call});
      continue;
    }
    case OP_INC:
    case OP_DEC: {
      consume();

      NodeStepExpr step;
      step.op = curr->type;
      step.expr = parse_expr();

      lhs = alloc_node(curr->loc, AstKind::EXPR_Step, {.e_step = step});
      continue;
    }
    case KW_AS: { // Type casting: expr as T
      consume();

      NodeCastExpr cast;
      cast.expr = lhs;
      cast.ty = parse_type();

      lhs = alloc_node(curr->loc, AstKind::EXPR_Cast, {.e_cast = cast});
      continue;
    }
    default:
      // No more postfix tokens; return the accumulated expression.
      return lhs;
    }
  }
}

AstNode* Parser::parse_binary(int precedence) {
  AstNode* prim = parse_primary();
  AstNode* lhs = parse_postfix(prim);

  // Parse binary operators according to precedence.
  while (true) {
    Token* op = current();
    if (!op->is_operator()) {
      break;
    }

    int op_prec = op->bin_prec();
    if (op_prec < precedence) {
      break;
    }

    consume();

    NodeBinExpr bin;
    bin.op = op->type;
    bin.lhs = lhs;
    bin.rhs = parse_expr();

    lhs = alloc_node(lhs->loc, AstKind::EXPR_Bin, {.e_bin = bin});
  }

  return lhs;
}

AstNode* Parser::parse_expr() {
  return parse_binary(0);
}

AstNode* Parser::parse_declaration() {
  Token* first = consume();
  TokenType first_type = first->type;

  bool is_local = false;
  bool is_global = false;
  bool is_const = false;

  // Otherwise, handle local/global/const
  is_local = first_type == KW_LOCAL;
  is_global = first_type == KW_GLOBAL;
  is_const = first_type == KW_CONST;

  // Detect `fn` at the start (fn name())
  if (first_type == KW_FUNC) {
    goto parse_function;
  }

  if (is_local && current()->type == KW_CONST) {
    is_const = true;
    consume();
  }

  if (current()->type == KW_FUNC) {
    consume();

  parse_function:
    Token* identifier = expect_consume(IDENTIFIER, "Expected function name after 'func'");
    expect_consume(PAREN_OPEN, "Expected '(' to open function parameters");

    std::vector<Parameter> buf;

    while (true) {
      if (current()->type == PAREN_CLOSE) {
        break;
      }

      [[maybe_unused]] int* modfs = parse_modifiers();
      Token* param_name =
        expect_consume(IDENTIFIER, "Expected identifier for function parameter name");
      expect_consume(COLON, "Expected ':' between parameter and type");

      Parameter par;
      par.id = sema::alloc_string(lctx.stralloc, param_name->lexeme);
      par.type = parse_type();
      buf.push_back(par);

      if (current()->type != PAREN_CLOSE) {
        expect_consume(COMMA, "Expected ',' to seperate function parameters");
      }
    }

    [[maybe_unused]] int modfs = 0;
    if (is_const) {
      modfs |= F_SEMA_CONST;
    }

    expect_consume(PAREN_CLOSE, "Expected ')' to close function parameters");
    expect_consume(RETURNS, "Expected '->' to denote function return type");

    NodeFuncDeclStmt func;
    func.id = sema::alloc_string(lctx.stralloc, identifier->lexeme);
    func.glb = is_global;
    func.rets = parse_type();
    func.body = parse_scope();
    func.params = sema::alloc_array(lctx.astalloc, buf);
    func.paramc = buf.size();

    return alloc_node(first->loc, AstKind::STMT_Func, {.s_func = func});
  }

  // If not a function, continue parsing as a variable declaration
  AstNode* type;
  Token* identifier = expect_consume(IDENTIFIER, "Expected identifier for variable declaration");

  if (current()->type == COLON) {
    consume();
    type = parse_type();
  }
  else {
    type = alloc_node(identifier->loc, AstKind::TYPE_Auto, {});
  }

  expect_consume(EQ, "Expected '=' for variable declaration");

  NodeDeclStmt decl;
  decl.id = sema::alloc_string(lctx.stralloc, identifier->lexeme);
  decl.glb = is_global;
  decl.type = type;
  decl.rval = parse_expr();

  return alloc_node(first->loc, AstKind::STMT_Decl, {.s_decl = decl});
}

AstNode* Parser::parse_scope() {
  Token* tok = current();
  std::vector<AstNode*> buf;

  if (tok->type == BRACE_OPEN) {
    expect_consume(BRACE_OPEN, "Expected '{' to open scope");

    while (true) {
      if (current()->type == BRACE_CLOSE) {
        break;
      }
      buf.push_back(parse_stmt());
    }

    expect_consume(BRACE_CLOSE, "Expected '}' to close scope");
  }
  else if (tok->type == COLON) {
    expect_consume(COLON, "Expected ':' to open single-statment scope");
    buf.push_back(parse_stmt());
  }
  else {
    throw ParserError{false, tok->loc, "Expected '{' or ':' to start scope"};
  }

  NodeScopeStmt scope;
  scope.stmts = sema::alloc_array(lctx.astalloc, buf);
  scope.stmtc = buf.size();

  return alloc_node(tok->loc, AstKind::STMT_Scope, {.s_scope = scope});
}

AstNode* Parser::parse_if() {
  Token* tok = consume();
  AstNode* cond = parse_expr();
  AstNode* scope = parse_scope();
  AstNode* els = nullptr;

  std::vector<NodeIfStmt::Elif> buf;

  while (true) {
    if (current()->type != KW_ELIF) {
      break;
    }

    Token* elif_kw = consume();

    NodeIfStmt::Elif elif;
    elif.cond = parse_expr();
    elif.scp = parse_scope();
    elif.loc = elif_kw->loc;

    buf.push_back(elif);
  }

  if (current()->type == KW_ELSE) {
    consume();
    els = parse_scope();
  }

  NodeIfStmt ifs;
  ifs.cond = cond;
  ifs.scp = scope;
  ifs.els = els;
  ifs.elifs = sema::alloc_array(lctx.astalloc, buf);
  ifs.elifc = buf.size();

  return alloc_node(tok->loc, AstKind::STMT_If, {.s_if = ifs});
}

AstNode* Parser::parse_return() {
  Token* tok = consume();

  NodeRetStmt ret;
  ret.expr = parse_expr();

  return alloc_node(tok->loc, AstKind::STMT_Ret, {.s_ret = ret});
}

AstNode* Parser::parse_while() {
  Token* tok = consume();

  NodeWhileStmt whil;
  whil.cond = parse_expr();
  whil.body = parse_scope();

  return alloc_node(tok->loc, AstKind::STMT_Whi, {.s_whi = whil});
}

AstNode* Parser::parse_stmt() {
  Token* tok = current();

  switch (tok->type) {
  case KW_LOCAL:
  case KW_GLOBAL:
  case KW_FUNC:
  case KW_CONST:
    return parse_declaration();
  case KW_DO:
    consume();
    return parse_scope();
  case KW_IF:
    return parse_if();
  case KW_RETURN:
    return parse_return();
  case KW_WHILE:
    return parse_while();
  case KW_DEFER: {
    consume();

    NodeDeferStmt def;
    def.stmt = parse_stmt();

    return alloc_node(tok->loc, AstKind::STMT_Def, {.s_def = def});
  }
  case KW_BREAK:
  case KW_CONTINUE: {
    consume();
    return alloc_node(tok->loc, tok->type == KW_BREAK ? AstKind::STMT_Brk : AstKind::STMT_Cont, {});
  }
  default:
    AstNode* lval = parse_expr();

    if (current()->type != EOF_) {
      if (current()->lexeme == "=" || (current()->is_operator() && peek()->lexeme == "=")) {
        Token* aug = consume();
        if (aug->lexeme != "=") {
          consume();
        }

        NodeAsgnStmt asgn;
        asgn.aug = aug->type;
        asgn.lval = lval;
        asgn.rval = parse_expr();

        return alloc_node(tok->loc, AstKind::STMT_Asgn, {.s_asgn = asgn});
      }
    }

    NodeExprStmt expr;
    expr.expr = lval;

    return alloc_node(tok->loc, AstKind::STMT_Expr, {.s_expr = expr});
  }

  VIA_UNREACHABLE();
  return nullptr;
}

bool Parser::parse() {
  try {
    while (true) {
      if (current()->type == EOF_) {
        break;
      }

      while (true) {
        if (current()->type != AT) {
          break;
        }

        attrib_buffer.push_back(*parse_attribute());
      }

      lctx.ast.push_back(parse_stmt());
      attrib_buffer.clear();
    }
  }
  catch (const ParserError& err) {
    err_bus.log({err.loc, err.flat, err.message, ERROR_, lctx});
    return true;
  }

  return false;
}

} // namespace via
