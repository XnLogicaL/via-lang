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
    return nullptr;
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
          curr->lexeme.length(),
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

  expect_consume(OP_LT, "Expected '<' to open type generic");

  std::vector<AstNode*> buf;

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
  case LIT_STRING:
    consume();
    return lctx.astalloc.emplace<NodeLitExpr>(*tok, tok->lexeme);
  case OP_INC:
  case OP_DEC:
  case OP_LEN:
  case OP_SUB: {
    Token* op = consume();
    ParseResult<ExprNode*> expr = parse_primary();

    CHECK_RESULT(op);
    CHECK_RESULT(expr);

    return lctx.astalloc.emplace<NodeUnExpr>(*op, *expr);
  }
  case BRACKET_OPEN: {
    Token* br_open = consume();
    NodeArrExpr::values_t values;

    while (true) {
      Token* curr = current();
      CHECK_RESULT(curr);

      if (curr->type == BRACKET_CLOSE) {
        break;
      }

      ParseResult<ExprNode*> expr = parse_expr();
      CHECK_RESULT(expr);

      values.emplace_back(*expr);

      curr = expect_consume(COMMA, "Expected ',' to seperate list elements");
      CHECK_RESULT(curr);
    }

    Token* br_close = consume();
    CHECK_RESULT(br_close);

    return lctx.astalloc.emplace<NodeArrExpr>(br_open->position, br_close->position, values);
  }
  case KW_TYPE:
  case KW_TYPEOF:
  case KW_NAMEOF:
  case KW_PRINT:
  case KW_ERROR:
  case KW_TRY: {
    Token* intrinsic = consume();
    ParseResult<ExprNode*> expr = parse_expr();

    CHECK_RESULT(intrinsic);
    CHECK_RESULT(expr);

    return lctx.astalloc.emplace<NodeIntrExpr>(*intrinsic, std::vector<ExprNode*>{*expr});
  }
  case PAREN_OPEN: {
    consume();

    ParseResult<ExprNode*> expr = parse_expr();
    Token* expect_par = expect_consume(PAREN_CLOSE, "Expected ')' to close grouping expression");

    CHECK_RESULT(expr);
    CHECK_RESULT(expect_par);

    return lctx.astalloc.emplace<NodeGroupExpr>(*expr);
  }
  default:
    break;
  }

  // If none of the valid Token types were matched, return an error.
  return tl::unexpected<ParseError>(
    {position, std::format("Unexpected Token '{}' while parsing primary expression", tok->lexeme)}
  );
}

ParseResult<ExprNode*> Parser::parse_postfix(ExprNode* lhs) {
  // Process all postfix expressions (member access, array indexing, function calls, type casts)
  while (true) {
    Token* curr = current();
    if (!curr.has_value()) {
      return lhs;
    }

    switch (curr->type) {
    case DOT: { // Member access: obj.property
      consume();
      Token* index_token = expect_consume(IDENTIFIER, "Expected identifier while parsing index");
      CHECK_RESULT(index_token);

      lhs =
        lctx.astalloc.emplace<NodeIndexExpr>(lhs, lctx.astalloc.emplace<NodeSymExpr>(*index_token));

      continue;
    }
    case BRACKET_OPEN: { // Array indexing: obj[expr]
      consume();

      ParseResult<ExprNode*> index = parse_expr();
      Token* expect_br = expect_consume(BRACKET_CLOSE, "Expected ']' to close index expression");

      CHECK_RESULT(index);
      CHECK_RESULT(expect_br);

      lhs = lctx.astalloc.emplace<NodeIndexExpr>(lhs, *index);
      continue;
    }
    case PAREN_OPEN: { // Function calls: func(arg1, ...)
      consume();
      std::vector<ExprNode*> arguments;

      while (true) {
        Token* curr = current();
        CHECK_RESULT(curr);

        if (curr->type == PAREN_CLOSE) {
          break;
        }

        ParseResult<ExprNode*> expr = parse_expr();
        CHECK_RESULT(expr);

        arguments.emplace_back(*expr);

        // After an argument, check for a comma.
        curr = current();
        CHECK_RESULT(curr);

        if (curr->type == COMMA) {
          consume();
        }
        else {
          break;
        }
      }

      // Expect the closing parenthesis.
      Token* expect_par =
        expect_consume(PAREN_CLOSE, "Expected ')' to close function call arguments");
      CHECK_RESULT(expect_par);

      lhs = lctx.astalloc.emplace<NodeCallExpr>(lhs, arguments);
      continue;
    }
    case OP_INC: {
      consume();

      lhs = lctx.astalloc.emplace<StepExprNode>(lhs, true);
      continue;
    }
    case OP_DEC: {
      consume();

      lhs = lctx.astalloc.emplace<StepExprNode>(lhs, false);
      continue;
    }
    case KW_AS: { // Type casting: expr as Type
      consume();

      ParseResult<TypeNode*> type_result = parse_type();
      CHECK_RESULT(type_result);

      lhs = lctx.astalloc.emplace<NodeCastExpr>(lhs, *type_result);
      continue;
    }
    default:
      // No more postfix tokens; return the accumulated expression.
      return lhs;
    }
  }
}

ParseResult<ExprNode*> Parser::parse_binary(int precedence) {
  // Parse the primary expression first.
  ParseResult<ExprNode*> prim = parse_primary();
  CHECK_RESULT(prim);

  // Parse any postfix operations that apply to the primary expression.
  ParseResult<ExprNode*> lhs = parse_postfix(*prim);
  CHECK_RESULT(lhs);

  // Parse binary operators according to precedence.
  while (position < lctx.tokens.size()) {
    Token* op = current();
    CHECK_RESULT(op);

    // Stop if the Token is not an operator.
    if (!op->is_operator()) {
      break;
    }

    int op_prec = op->bin_prec();
    if (op_prec < precedence) {
      break;
    }

    consume();

    ParseResult<ExprNode*> rhs = parse_binary(op_prec + 1);
    CHECK_RESULT(rhs);

    lhs = lctx.astalloc.emplace<NodeBinExpr>(*op, *lhs, *rhs);
  }

  return lhs;
}

ParseResult<ExprNode*> Parser::parse_expr() {
  return parse_binary(0);
}

ParseResult<StmtNode*> Parser::parse_declaration() {
  Token* first = consume();
  CHECK_RESULT(first);

  size_t begin = first->position;
  TokenType first_type = first->type;

  bool is_local = false;
  bool is_global = false;
  bool is_const = false;

  // Otherwise, handle local/global/const
  is_local = first_type == KW_LOCAL;
  is_global = first_type == KW_GLOBAL;
  is_const = first_type == KW_CONST;

  Token* curr = current();
  CHECK_RESULT(curr);

  // Detect `fn` at the start (fn name())
  if (first_type == KW_FUNC) {
    goto parse_function;
  }

  if (is_local && curr->type == KW_CONST) {
    is_const = true;
    consume();
  }

  curr = current();
  CHECK_RESULT(curr);

  if (curr->type == KW_FUNC) {
    consume();

  parse_function:
    Token* identifier = expect_consume(IDENTIFIER, "Expected function name after 'func'");
    Token* expect_par = expect_consume(PAREN_OPEN, "Expected '(' to open function parameters");

    CHECK_RESULT(identifier);
    CHECK_RESULT(expect_par);

    std::vector<ParamStmtNode> parameters;

    while (true) {
      curr = current();
      if (curr.has_value() && curr->type == PAREN_CLOSE) {
        break;
      }

      ParseResult<StmtModifiers> StmtModifiers = parse_modifiers();
      Token* param_name =
        expect_consume(IDENTIFIER, "Expected identifier for function parameter name");
      Token* expect_col = expect_consume(COLON, "Expected ':' between parameter and type");
      ParseResult<TypeNode*> param_type = parse_type();

      CHECK_RESULT(StmtModifiers);
      CHECK_RESULT(param_name);
      CHECK_RESULT(expect_col);
      CHECK_RESULT(param_type);

      parameters.emplace_back(*param_name, *StmtModifiers, *param_type);

      curr = current();
      CHECK_RESULT(curr);

      if (curr->type != PAREN_CLOSE) {
        Token* expect_comma = expect_consume(COMMA, "Expected ',' between function parameters");
        CHECK_RESULT(expect_comma);
      }
      else {
        break;
      }
    }

    StmtModifiers StmtModifiers{is_const};

    Token* expect_rpar = expect_consume(PAREN_CLOSE, "Expected ')' to close function parameters");
    Token* expect_rets = expect_consume(RETURNS, "Expected '->' to denote function return type");
    ParseResult<TypeNode*> returns = parse_type();
    ParseResult<StmtNode*> body_scope = parse_scope();

    CHECK_RESULT(expect_rpar);
    CHECK_RESULT(expect_rets);
    CHECK_RESULT(returns);
    CHECK_RESULT(body_scope);

    return lctx.astalloc.emplace<NodeFuncDeclStmt>(
      begin,
      (*body_scope)->end,
      is_global,
      StmtModifiers,
      *identifier,
      *body_scope,
      *returns,
      parameters
    );
  }

  // If not a function, continue parsing as a variable declaration
  TypeNode* type;
  Token* identifier = expect_consume(IDENTIFIER, "Expected identifier for variable declaration");
  curr = current();

  CHECK_RESULT(identifier);
  CHECK_RESULT(curr);

  if (curr->type == COLON) {
    consume();

    ParseResult<TypeNode*> temp = parse_type();
    CHECK_RESULT(temp);
    type = *temp;
  }
  else {
    type =
      lctx.astalloc.emplace<AutoTypeNode>(curr->position, curr->position + curr->lexeme.length());
  }

  Token* expect_eq = expect_consume(EQ, "Expected '=' for variable declaration");
  ParseResult<ExprNode*> value = parse_expr();

  CHECK_RESULT(expect_eq);
  CHECK_RESULT(value);

  type->expression = *value; // Attach expression reference to type

  return lctx.astalloc.emplace<NodeDeclStmt>(
    begin, (*value)->end, is_global, StmtModifiers{is_const}, *identifier, *value, type
  );
}

ParseResult<StmtNode*> Parser::parse_scope() {
  std::vector<StmtNode*> scope_statements;

  Token* curr = current();
  CHECK_RESULT(curr);

  size_t begin = curr->position;
  size_t end;

  if (curr->type == BRACE_OPEN) {
    Token* expect_br = expect_consume(BRACE_OPEN, "Expected '{' to open scope");
    CHECK_RESULT(expect_br);

    while (true) {
      Token* curr = current();
      CHECK_RESULT(curr);

      if (curr->type == BRACE_CLOSE) {
        break;
      }

      ParseResult<StmtNode*> stmt = parse_stmt();
      CHECK_RESULT(stmt);

      scope_statements.emplace_back(*stmt);
    }

    expect_br = expect_consume(BRACE_CLOSE, "Expected '}' to close scope");
    CHECK_RESULT(expect_br);
    end = expect_br->position;
  }
  else if (curr->type == COLON) {
    Token* expect_col = expect_consume(COLON, "Expected ':' to open single-statment scope");
    CHECK_RESULT(expect_col);

    ParseResult<StmtNode*> stmt = parse_stmt();
    CHECK_RESULT(stmt);

    scope_statements.emplace_back(*stmt);
    end = (*stmt)->end;
  }
  else {
    return tl::unexpected<ParseError>(
      {position, "Expected '{' or ':' to open scope or single-statement scope"}
    );
  }

  return lctx.astalloc.emplace<NodeScopeStmt>(begin, end, scope_statements);
}

ParseResult<StmtNode*> Parser::parse_if() {
  Token* kw = consume();
  ParseResult<ExprNode*> condition = parse_expr();
  ParseResult<StmtNode*> scope = parse_scope();

  CHECK_RESULT(kw);
  CHECK_RESULT(condition);
  CHECK_RESULT(scope);

  size_t begin = kw->position;
  size_t end;
  std::optional<StmtNode*> else_scope;
  std::vector<ElseIfNode*> elseif_nodes;

  while (true) {
    Token* curr = current();
    CHECK_RESULT(curr);

    if (curr->type != KW_ELIF) {
      break;
    }

    consume();

    ParseResult<ExprNode*> elseif_condition = parse_expr();
    ParseResult<StmtNode*> elseif_scope = parse_scope();

    CHECK_RESULT(elseif_condition);
    CHECK_RESULT(elseif_scope);

    ElseIfNode* elseif = lctx.astalloc.emplace<ElseIfNode>(
      curr->position, (*elseif_scope)->end, *elseif_condition, *elseif_scope
    );
    elseif_nodes.emplace_back(elseif);
  }

  Token* curr = current();
  CHECK_RESULT(curr);

  if (curr->type == KW_ELSE) {
    consume();

    ParseResult<StmtNode*> else_scope_inner = parse_scope();
    CHECK_RESULT(else_scope_inner);

    else_scope = *else_scope_inner;
    end = (*else_scope_inner)->end;
  }
  else {
    else_scope = nullptr;

    if (!elseif_nodes.empty()) {
      ElseIfNode*& last = elseif_nodes.back();
      end = last->end;
    }
    else {
      end = (*scope)->end;
    }
  }

  // Return the IfStmtNode
  return lctx.astalloc.emplace<IfStmtNode>(
    begin, end, *condition, *scope, *else_scope, elseif_nodes
  );
}

ParseResult<StmtNode*> Parser::parse_return() {
  Token* kw = consume();
  ParseResult<ExprNode*> expr = parse_expr();
  CHECK_RESULT(kw);
  CHECK_RESULT(expr);

  return lctx.astalloc.emplace<ReturnStmtNode>(kw->position, (*expr)->end, *expr);
}

ParseResult<StmtNode*> Parser::parse_while() {
  Token* kw = consume();
  ParseResult<ExprNode*> condition = parse_expr();
  ParseResult<StmtNode*> body = parse_scope();

  CHECK_RESULT(kw);
  CHECK_RESULT(condition);
  CHECK_RESULT(body);

  return lctx.astalloc.emplace<WhileStmtNode>(kw->position, (*body)->end, *condition, *body);
}

ParseResult<StmtNode*> Parser::parse_stmt() {
  Token* initial_token = current();
  CHECK_RESULT(initial_token);

  switch (initial_token->type) {
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
    Token* con = consume();
    ParseResult<StmtNode*> stmt = parse_stmt();

    CHECK_RESULT(con);
    CHECK_RESULT(stmt);

    return lctx.astalloc.emplace<DeferStmtNode>(con->position, (*stmt)->end, *stmt);
  }
  case KW_BREAK: {
    Token* con = consume();
    CHECK_RESULT(con);

    return lctx.astalloc.emplace<BreakStmtNode>(
      con->position, con->position + con->lexeme.length()
    );
  }
  case KW_CONTINUE: {
    Token* con = consume();
    CHECK_RESULT(con);

    return lctx.astalloc.emplace<ContinueStmtNode>(
      con->position, con->position + con->lexeme.length()
    );
  }
  default:
    ParseResult<ExprNode*> lvalue = parse_expr();
    Token* curr = current();

    CHECK_RESULT(lvalue);
    CHECK_RESULT(curr);

    if (curr->type == EOF_) {
      return lctx.astalloc.emplace<ExprStmtNode>(*lvalue);
    }

    Token* pk = peek();
    CHECK_RESULT(pk);

    if (curr->lexeme == "=" || (curr->is_operator() && pk->lexeme == "=")) {
      Token* possible_augment = consume();
      CHECK_RESULT(possible_augment);

      if (possible_augment->lexeme != "=") {
        consume();
      }

      ParseResult<ExprNode*> rvalue = parse_expr();
      return lctx.astalloc.emplace<NodeAsgnStmt>(*lvalue, *possible_augment, *rvalue);
    }

    return lctx.astalloc.emplace<ExprStmtNode>(*lvalue);
  }

  VIA_UNREACHABLE();
  return nullptr;
}

bool Parser::parse() {
  while (true) {
    Token* curr = current();
    CHECK_RESULT_MAINFN(curr);

    if (curr->type == EOF_) {
      break;
    }

    while (true) {
      curr = current();
      CHECK_RESULT_MAINFN(curr);

      if (curr->type != AT) {
        break;
      }

      ParseResult<StmtAttribute> attrib = parse_attribute();
      CHECK_RESULT_MAINFN(attrib);

      attrib_buffer.push_back(*attrib);
    }

    ParseResult<StmtNode*> stmt = parse_stmt();
    CHECK_RESULT_MAINFN(stmt);

    stmt.value()->attributes = attrib_buffer;
    attrib_buffer.clear();

    lctx.ast.emplace_back(*stmt);
  }

  return false;
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

} // namespace via
