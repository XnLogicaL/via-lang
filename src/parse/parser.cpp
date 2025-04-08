//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================
#include "parser.h"
#include "compiler-types.h"

#define VIA_CHECKRESULT(result)                                                                    \
  if (!result.has_value()) {                                                                       \
    return tl::unexpected(result.error());                                                         \
  }

#define VIA_CHECKRESULT_MAINFN(result)                                                             \
  if (!result.has_value()) {                                                                       \
    auto err = result.error();                                                                     \
    err_bus.log({false, err.what, unit_ctx, ERROR_, unit_ctx.tokens->at(err.where)});              \
    return true;                                                                                   \
  }

namespace via {

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#endif

using enum CErrorLevel;
using enum TokenType;

template<typename T>
using result = Parser::result<T>;

result<Token> Parser::current() {
  return peek(0);
}

result<Token> Parser::peek(int32_t ahead) {
  if (position + ahead >= unit_ctx.tokens->size()) {
    return tl::unexpected<ParseError>({
      position,
      std::format("Unexpected end of file (attempted read of: Token #{})", position + ahead),
    });
  }

  return unit_ctx.tokens->at(position + ahead);
}

result<Token> Parser::consume(uint32_t ahead) {
  uint64_t new_pos = position + static_cast<uint64_t>(ahead);
  if (new_pos >= unit_ctx.tokens->size()) {
    return tl::unexpected<ParseError>({
      position,
      std::format("Unexpected end of file (attempted consumption of: Token #{})", new_pos),
    });
  }

  position = new_pos;
  return peek(-static_cast<int32_t>(ahead));
}

result<Token> Parser::expect_consume(TokenType type, const std::string& what) {
  result<Token> curr = current();
  if (!curr.has_value()) {
    return curr;
  }

  if (curr->type == type) {
    return consume();
  }

  return tl::unexpected<ParseError>({position, what});
}

result<StmtModifiers> Parser::parse_modifiers() {
  StmtModifiers StmtModifiers;

  while (true) {
    result<Token> curr = current();
    VIA_CHECKRESULT(curr);

    if (curr->type == KW_CONST) {
      if (StmtModifiers.is_const) {
        err_bus.log({false, "Modifier 'const' encountered multiple times", unit_ctx, WARNING, *curr}
        );
      }

      StmtModifiers.is_const = true;
      consume();
    }
    else {
      break;
    }
  }

  return StmtModifiers;
}

result<StmtAttribute> Parser::parse_attribute() {
  result<Token> expect_at = expect_consume(AT, "Expected '@' to begin StmtAttribute");
  result<Token> expect_id =
    expect_consume(IDENTIFIER, "Expected identifier for StmtAttribute name");
  result<Token> curr = current();

  VIA_CHECKRESULT(expect_at);
  VIA_CHECKRESULT(expect_id);
  VIA_CHECKRESULT(curr);

  StmtAttribute attr;
  attr.identifier = *expect_id;

  if (curr->type == PAREN_OPEN) {
    consume();

    while (true) {
      curr = current();
      VIA_CHECKRESULT(curr);

      if (curr->type == PAREN_CLOSE) {
        break;
      }

      result<Token> tok = consume();
      attr.arguments.push_back(*tok);
    }

    consume();
  }

  return attr;
}

result<TypeNodeBase*> Parser::parse_generic() {
  using generics_t = GenericTypeNode::generics_t;

  result<Token> identifier = consume();
  result<StmtModifiers> modifiers = parse_modifiers();

  VIA_CHECKRESULT(identifier);
  VIA_CHECKRESULT(modifiers);

  generics_t generics;

  result<Token> expect_lt = expect_consume(OP_LT, "Expected '<' to open type generic");
  result<Token> curr = current();

  VIA_CHECKRESULT(expect_lt);
  VIA_CHECKRESULT(curr);

  while (curr->type != OP_GT) {
    result<TypeNodeBase*> result_type = parse_type();
    result<Token> result_token = current();

    VIA_CHECKRESULT(result_type);
    VIA_CHECKRESULT(result_token);

    generics.emplace_back(result_type.value());

    if (curr->type != OP_GT) {
      result<Token> expect_comma = expect_consume(COMMA, "Expected ',' to seperate type generics");

      VIA_CHECKRESULT(expect_comma);
    }
  }

  result<Token> expect_gt = expect_consume(OP_GT, "Expected '>' to close type generic");
  VIA_CHECKRESULT(expect_gt);

  return unit_ctx.ast->allocator.emplace<GenericTypeNode>(*identifier, generics, *modifiers);
}

result<TypeNodeBase*> Parser::parse_type_primary() {
  static std::unordered_map<std::string, IValueType> primitive_map = {
    {"nil", IValueType::nil},
    {"int", IValueType::integer},
    {"float", IValueType::floating_point},
    {"bool", IValueType::boolean},
    {"string", IValueType::string},
  };

  result<Token> tok = current();

  VIA_CHECKRESULT(tok);

  switch (tok->type) {
  case KW_AUTO:
    if (tok->lexeme == "auto") {
      return unit_ctx.ast->allocator.emplace<AutoTypeNode>(
        tok->position, tok->position + tok->lexeme.length()
      );
    }
    [[fallthrough]];
  case IDENTIFIER:
  case LIT_NIL: {
    auto it = primitive_map.find(tok->lexeme);
    if (it != primitive_map.end()) {
      return unit_ctx.ast->allocator.emplace<PrimTypeNode>(consume().value(), it->second);
    }

    return parse_generic();
  }
  case PAREN_OPEN: {
    using parameters = FunctionTypeNode::parameter_vector;
    parameters params;

    result<Token> curr = consume();
    VIA_CHECKRESULT(curr);

    while (true) {
      result<Token> tok = current();
      VIA_CHECKRESULT(tok);

      if (tok->type == PAREN_CLOSE) {
        consume();
        return unit_ctx.ast->allocator.emplace<PrimTypeNode>(*tok, IValueType::nil);
      }

      result<TypeNodeBase*> type_result = parse_type();
      VIA_CHECKRESULT(type_result);

      params.emplace_back(
        Token(TokenType::IDENTIFIER, "", 0, 0, 0), StmtModifiers{}, type_result.value()
      );

      if (tok->type != PAREN_CLOSE) {
        result<Token> expect_comma =
          expect_consume(COMMA, "Expected ',' to seperate function parameter types");

        VIA_CHECKRESULT(expect_comma);
      }
    }

    result<Token> expect_pc =
      expect_consume(PAREN_CLOSE, "Expected ')' to close function type parameters");
    result<Token> expect_rt =
      expect_consume(RETURNS, "Expected '->' to specify function return type");
    result<TypeNodeBase*> return_type = parse_type();

    VIA_CHECKRESULT(expect_pc);
    VIA_CHECKRESULT(expect_rt);
    VIA_CHECKRESULT(return_type);

    return unit_ctx.ast->allocator.emplace<FunctionTypeNode>(params, return_type.value());
  }
  default: {
    break;
  }
  }

  return tl::unexpected<ParseError>({
    position,
    std::format("Unexpected Token '{}' while parsing type primary", tok->lexeme),
  });
}

result<TypeNodeBase*> Parser::parse_type() {
  return parse_type_primary();
}

result<ExprNodeBase*> Parser::parse_primary() {
  // Get the current Token.
  result<Token> tok = current();
  VIA_CHECKRESULT(tok);

  switch (tok->type) {
  case LIT_INT:
  case LIT_HEX: {
    consume();
    int value = std::stoi(tok->lexeme);
    return unit_ctx.ast->allocator.emplace<LitExprNode>(*tok, value);
  }
  case LIT_FLOAT: {
    consume();
    float value = std::stof(tok->lexeme);
    return unit_ctx.ast->allocator.emplace<LitExprNode>(*tok, value);
  }
  case LIT_BINARY: {
    consume();
    // Skip the "0b" prefix before converting.
    return unit_ctx.ast->allocator.emplace<LitExprNode>(
      *tok, static_cast<int>(std::bitset<64>(tok->lexeme.substr(2)).to_ullong())
    );
  }
  case LIT_NIL:
    consume();
    return unit_ctx.ast->allocator.emplace<LitExprNode>(*tok, std::monostate());
  case LIT_BOOL:
    consume();
    return unit_ctx.ast->allocator.emplace<LitExprNode>(*tok, tok->lexeme == "true");
  case IDENTIFIER:
    consume();
    return unit_ctx.ast->allocator.emplace<SymExprNode>(*tok);
  case LIT_STRING:
    consume();
    return unit_ctx.ast->allocator.emplace<LitExprNode>(*tok, tok->lexeme);
  case OP_SUB: { // Unary minus operator
    consume();
    // Directly parse the operand; no need to keep a copy of Token here.
    result<ExprNodeBase*> expr = parse_primary();
    VIA_CHECKRESULT(expr);

    return unit_ctx.ast->allocator.emplace<UnaryExprNode>(*expr);
  }
  case BRACKET_OPEN: {
    result<Token> br_open = consume();
    ArrayExprNode::values_t values;

    while (true) {
      result<Token> curr = current();
      VIA_CHECKRESULT(curr);

      if (curr->type == BRACKET_CLOSE) {
        break;
      }

      result<ExprNodeBase*> expr = parse_expr();
      VIA_CHECKRESULT(expr);

      values.emplace_back(*expr);

      curr = expect_consume(COMMA, "Expected ',' to seperate list elements");
      VIA_CHECKRESULT(curr);
    }

    result<Token> br_close = consume();
    VIA_CHECKRESULT(br_close);

    return unit_ctx.ast->allocator.emplace<ArrayExprNode>(*br_open, *br_close, values);
  }
  default:
    break;
  }

  // If none of the valid Token types were matched, return an error.
  return tl::unexpected<ParseError>(
    {position, std::format("Unexpected Token '{}' while parsing primary expression", tok->lexeme)}
  );
}

result<ExprNodeBase*> Parser::parse_postfix(ExprNodeBase* lhs) {
  // Process all postfix expressions (member access, array indexing, function calls, type casts)
  while (true) {
    result<Token> curr = current();
    if (!curr.has_value()) {
      return lhs;
    }

    switch (curr->type) {
    case DOT: { // Member access: obj.property
      consume();
      result<Token> index_token =
        expect_consume(IDENTIFIER, "Expected identifier while parsing index");
      VIA_CHECKRESULT(index_token);

      lhs = unit_ctx.ast->allocator.emplace<IndexExprNode>(
        lhs, unit_ctx.ast->allocator.emplace<SymExprNode>(*index_token)
      );

      continue;
    }
    case BRACKET_OPEN: { // Array indexing: obj[expr]
      consume();
      result<ExprNodeBase*> index = parse_expr();
      VIA_CHECKRESULT(index);

      result<Token> expect_br =
        expect_consume(BRACKET_CLOSE, "Expected ']' to close index expression");
      VIA_CHECKRESULT(expect_br);

      lhs = unit_ctx.ast->allocator.emplace<IndexExprNode>(lhs, *index);
      continue;
    }
    case PAREN_OPEN: { // IFunction calls: func(arg1, ...)
      consume();
      std::vector<ExprNodeBase*> arguments;

      while (true) {
        result<Token> curr = current();
        VIA_CHECKRESULT(curr);

        if (curr->type == PAREN_CLOSE) {
          break;
        }

        result<ExprNodeBase*> expr = parse_expr();
        VIA_CHECKRESULT(expr);

        arguments.emplace_back(*expr);

        // After an argument, check for a comma.
        curr = current();
        VIA_CHECKRESULT(curr);

        if (curr->type == COMMA) {
          consume();
        }
        else {
          break;
        }
      }

      // Expect the closing parenthesis.
      result<Token> expect_par =
        expect_consume(PAREN_CLOSE, "Expected ')' to close function call arguments");
      VIA_CHECKRESULT(expect_par);

      lhs = unit_ctx.ast->allocator.emplace<CallExprNode>(lhs, arguments);
      continue;
    }
    case OP_INCREMENT: {
      consume();

      lhs = unit_ctx.ast->allocator.emplace<StepExprNode>(lhs, true, true);
      continue;
    }
    case OP_DECREMENT: {
      consume();

      lhs = unit_ctx.ast->allocator.emplace<StepExprNode>(lhs, false, true);
      continue;
    }
    case KW_AS: { // Type casting: expr as Type
      consume();

      result<TypeNodeBase*> type_result = parse_type();
      VIA_CHECKRESULT(type_result);

      lhs = unit_ctx.ast->allocator.emplace<CastExprNode>(lhs, *type_result);
      continue;
    }
    default:
      // No more postfix tokens; return the accumulated expression.
      return lhs;
    }
  }
}

result<ExprNodeBase*> Parser::parse_binary(int precedence) {
  // Parse the primary expression first.
  result<ExprNodeBase*> prim = parse_primary();
  VIA_CHECKRESULT(prim);

  // Parse any postfix operations that apply to the primary expression.
  result<ExprNodeBase*> lhs = parse_postfix(*prim);
  VIA_CHECKRESULT(lhs);

  // Parse binary operators according to precedence.
  while (position < unit_ctx.tokens->size()) {
    result<Token> op = current();
    VIA_CHECKRESULT(op);

    // Stop if the Token is not an operator.
    if (!op->is_operator()) {
      break;
    }

    int op_prec = op->bin_prec();
    if (op_prec < precedence) {
      break;
    }

    consume();

    result<ExprNodeBase*> rhs = parse_binary(op_prec + 1);
    VIA_CHECKRESULT(rhs);

    lhs = unit_ctx.ast->allocator.emplace<BinExprNode>(*op, *lhs, *rhs);
  }

  return lhs;
}

result<ExprNodeBase*> Parser::parse_expr() {
  return parse_binary(0);
}

result<StmtNodeBase*> Parser::parse_declaration() {
  result<Token> first = consume();
  VIA_CHECKRESULT(first);

  TokenType first_type = first->type;

  bool is_local = false;
  bool is_global = false;
  bool is_const = false;

  // Otherwise, handle local/global/const
  is_local = first_type == KW_LOCAL;
  is_global = first_type == KW_GLOBAL;
  is_const = first_type == KW_CONST;

  result<Token> curr = current();
  VIA_CHECKRESULT(curr);

  // Detect `fn` at the start (fn name())
  if (first_type == KW_FUNC) {
    goto parse_function;
  }

  if (is_local && curr->type == KW_CONST) {
    is_const = true;
    consume();
  }

  curr = current();
  VIA_CHECKRESULT(curr);

  if (curr->type == KW_FUNC) {
    consume();

  parse_function:
    result<Token> identifier = expect_consume(IDENTIFIER, "Expected function name after 'fn'");
    result<Token> expect_par =
      expect_consume(PAREN_OPEN, "Expected '(' to open function parameters");

    VIA_CHECKRESULT(identifier);
    VIA_CHECKRESULT(expect_par);

    std::vector<ParamNode> parameters;

    while (true) {
      curr = current();
      if (curr.has_value() && curr->type == PAREN_CLOSE) {
        break;
      }

      result<StmtModifiers> StmtModifiers = parse_modifiers();
      result<Token> param_name =
        expect_consume(IDENTIFIER, "Expected identifier for function parameter name");
      result<Token> expect_col = expect_consume(COLON, "Expected ':' between parameter and type");
      result<TypeNodeBase*> param_type = parse_type();

      VIA_CHECKRESULT(StmtModifiers);
      VIA_CHECKRESULT(param_name);
      VIA_CHECKRESULT(expect_col);
      VIA_CHECKRESULT(param_type);

      parameters.emplace_back(*param_name, *StmtModifiers, *param_type);

      curr = current();
      VIA_CHECKRESULT(curr);

      if (curr->type != PAREN_CLOSE) {
        result<Token> expect_comma =
          expect_consume(COMMA, "Expected ',' between function parameters");
        VIA_CHECKRESULT(expect_comma);
      }
      else {
        break;
      }
    }

    StmtModifiers StmtModifiers{is_const};

    result<Token> expect_rpar =
      expect_consume(PAREN_CLOSE, "Expected ')' to close function parameters");
    result<Token> expect_rets =
      expect_consume(RETURNS, "Expected '->' to denote function return type");
    result<TypeNodeBase*> returns = parse_type();
    result<StmtNodeBase*> body_scope = parse_scope();

    VIA_CHECKRESULT(expect_rpar);
    VIA_CHECKRESULT(expect_rets);
    VIA_CHECKRESULT(returns);
    VIA_CHECKRESULT(body_scope);

    return unit_ctx.ast->allocator.emplace<FuncDeclStmtNode>(
      is_global, StmtModifiers, *identifier, *body_scope, *returns, parameters
    );
  }

  // If not a function, continue parsing as a variable declaration
  TypeNodeBase* type;
  result<Token> identifier =
    expect_consume(IDENTIFIER, "Expected identifier for variable declaration");
  curr = current();

  VIA_CHECKRESULT(identifier);
  VIA_CHECKRESULT(curr);

  if (curr->type == COLON) {
    consume();

    result<TypeNodeBase*> temp = parse_type();
    VIA_CHECKRESULT(temp);
    type = *temp;
  }
  else {
    type = unit_ctx.ast->allocator.emplace<AutoTypeNode>(
      curr->position, curr->position + curr->lexeme.length()
    );
  }

  result<Token> expect_eq = expect_consume(EQ, "Expected '=' for variable declaration");
  result<ExprNodeBase*> value = parse_expr();

  VIA_CHECKRESULT(expect_eq);
  VIA_CHECKRESULT(value);

  type->expression = *value; // Attach expression reference to type

  return unit_ctx.ast->allocator.emplace<DeclStmtNode>(
    is_global, StmtModifiers{is_const}, *identifier, *value, type
  );
}

result<StmtNodeBase*> Parser::parse_scope() {
  std::vector<StmtNodeBase*> scope_statements;

  result<Token> curr = current();
  VIA_CHECKRESULT(curr);

  if (curr->type == BRACE_OPEN) {
    result<Token> expect_br = expect_consume(BRACE_OPEN, "Expected '{' to open scope");
    VIA_CHECKRESULT(expect_br);

    while (true) {
      result<Token> curr = current();
      VIA_CHECKRESULT(curr);

      if (curr->type == BRACE_CLOSE) {
        break;
      }

      result<StmtNodeBase*> stmt = parse_stmt();
      VIA_CHECKRESULT(stmt);

      scope_statements.emplace_back(*stmt);
    }

    expect_br = expect_consume(BRACE_CLOSE, "Expected '}' to close scope");
    VIA_CHECKRESULT(expect_br);
  }
  else if (curr->type == COLON) {
    result<Token> expect_col = expect_consume(COLON, "Expected ':' to open single-statment scope");
    VIA_CHECKRESULT(expect_col);

    result<StmtNodeBase*> stmt = parse_stmt();
    VIA_CHECKRESULT(stmt);

    scope_statements.emplace_back(*stmt);
  }
  else {
    return tl::unexpected<ParseError>(
      {position, "Expected '{' or ':' to open scope or single-statement scope"}
    );
  }

  return unit_ctx.ast->allocator.emplace<ScopeStmtNode>(scope_statements);
}

result<StmtNodeBase*> Parser::parse_if() {
  using elseif_node = IfStmtNode::elseif_node;

  consume();

  result<ExprNodeBase*> condition = parse_expr();
  result<StmtNodeBase*> scope = parse_scope();

  VIA_CHECKRESULT(condition);
  VIA_CHECKRESULT(scope);

  std::optional<StmtNodeBase*> else_scope;
  std::vector<elseif_node> elseif_nodes;

  while (true) {
    result<Token> curr = current();
    VIA_CHECKRESULT(curr);

    if (curr->type != KW_ELIF) {
      break;
    }

    result<ExprNodeBase*> elseif_condition = parse_expr();
    result<StmtNodeBase*> elseif_scope = parse_scope();

    VIA_CHECKRESULT(elseif_condition);
    VIA_CHECKRESULT(elseif_scope);

    elseif_node elseif(*elseif_condition, *elseif_scope);
    elseif_nodes.emplace_back(elseif);
  }

  result<Token> curr = current();
  VIA_CHECKRESULT(curr);

  if (curr->type == KW_ELSE) {
    consume();

    result<StmtNodeBase*> else_scope_inner = parse_scope();
    VIA_CHECKRESULT(else_scope_inner);

    else_scope = *else_scope_inner;
  }
  else {
    else_scope = nullptr;
  }

  return unit_ctx.ast->allocator.emplace<IfStmtNode>(*condition, *scope, *else_scope, elseif_nodes);
}

result<StmtNodeBase*> Parser::parse_return() {
  consume();

  result<ExprNodeBase*> expr = parse_expr();

  VIA_CHECKRESULT(expr);

  return unit_ctx.ast->allocator.emplace<ReturnStmtNode>(*expr);
}

result<StmtNodeBase*> Parser::parse_while() {
  consume();

  result<ExprNodeBase*> condition = parse_expr();
  result<StmtNodeBase*> body = parse_scope();

  VIA_CHECKRESULT(condition);
  VIA_CHECKRESULT(body);

  return unit_ctx.ast->allocator.emplace<WhileStmtNode>(*condition, *body);
}

result<StmtNodeBase*> Parser::parse_stmt() {
  result<Token> initial_token = current();
  VIA_CHECKRESULT(initial_token);

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
  case KW_BREAK: {
    result<Token> con = consume();
    VIA_CHECKRESULT(con);
    return unit_ctx.ast->allocator.emplace<BreakStmtNode>(*con);
  }
  case KW_CONTINUE: {
    result<Token> con = consume();
    VIA_CHECKRESULT(con);
    return unit_ctx.ast->allocator.emplace<ContinueStmtNode>(*con);
  }
  default:
    result<ExprNodeBase*> lvalue = parse_expr();
    result<Token> curr = current();

    VIA_CHECKRESULT(lvalue);
    VIA_CHECKRESULT(curr);

    if (curr->type == EOF_) {
      return unit_ctx.ast->allocator.emplace<ExprStmtNode>(*lvalue);
    }

    result<Token> pk = peek();
    VIA_CHECKRESULT(pk);

    if (curr->lexeme == "=" || (curr->is_operator() && pk->lexeme == "=")) {
      result<Token> possible_augment = consume();
      VIA_CHECKRESULT(possible_augment);

      if (possible_augment->lexeme != "=") {
        consume();
      }

      result<ExprNodeBase*> rvalue = parse_expr();

      return unit_ctx.ast->allocator.emplace<AssignStmtNode>(*lvalue, *possible_augment, *rvalue);
    }

    return unit_ctx.ast->allocator.emplace<ExprStmtNode>(*lvalue);
  }

  VIA_UNREACHABLE();
  return nullptr;
}

bool Parser::parse() {
  while (true) {
    result<Token> curr = current();
    VIA_CHECKRESULT_MAINFN(curr);

    if (curr->type == EOF_) {
      break;
    }

    while (true) {
      curr = current();
      VIA_CHECKRESULT_MAINFN(curr);

      if (curr->type != AT) {
        break;
      }

      result<StmtAttribute> attrib = parse_attribute();
      VIA_CHECKRESULT_MAINFN(attrib);

      attrib_buffer.push_back(*attrib);
    }

    result<StmtNodeBase*> stmt = parse_stmt();
    VIA_CHECKRESULT_MAINFN(stmt);

    stmt.value()->attributes = attrib_buffer;
    attrib_buffer.clear();

    unit_ctx.ast->statements.emplace_back(*stmt);
  }

  return false;
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

} // namespace via
