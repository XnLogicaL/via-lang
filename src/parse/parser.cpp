// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "parser.h"
#include "compiler-types.h"

#define vl_checkresult(result)                                                                     \
  if (!result.has_value()) {                                                                       \
    return tl::unexpected(result.error());                                                         \
  }

#define vl_checkresult_mainfn(result)                                                              \
  if (!result.has_value()) {                                                                       \
    auto err = result.error();                                                                     \
    err_bus.log({false, err.what, unit_ctx, ERROR_, unit_ctx.tokens->at(err.where)});              \
    return true;                                                                                   \
  }

namespace via {

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#elifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-result"
#elifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4834)
#endif

using enum comp_err_lvl;
using enum token_type;

template<typename T>
using result = parser::result<T>;

result<token> parser::current() {
  return peek(0);
}

result<token> parser::peek(int32_t ahead) {
  if (position + ahead >= unit_ctx.tokens->size()) {
    return tl::unexpected<parse_error>({
      position,
      std::format("Unexpected end of file (attempted read of: token #{})", position + ahead),
    });
  }

  return unit_ctx.tokens->at(position + ahead);
}

result<token> parser::consume(uint32_t ahead) {
  uint64_t new_pos = position + static_cast<uint64_t>(ahead);
  if (new_pos >= unit_ctx.tokens->size()) {
    return tl::unexpected<parse_error>({
      position,
      std::format("Unexpected end of file (attempted consumption of: token #{})", new_pos),
    });
  }

  position = new_pos;
  return peek(-static_cast<int32_t>(ahead));
}

result<token> parser::expect_consume(token_type type, const std::string& what) {
  result<token> curr = current();
  if (!curr.has_value()) {
    return curr;
  }

  if (curr->type == type) {
    return consume();
  }

  return tl::unexpected<parse_error>({position, what});
}

result<modifiers> parser::parse_modifiers() {
  modifiers modifiers;

  while (true) {
    result<token> curr = current();
    vl_checkresult(curr);

    if (curr->type == KW_CONST) {
      if (modifiers.is_const) {
        err_bus.log({false, "Modifier 'const' encountered multiple times", unit_ctx, WARNING, *curr}
        );
      }

      modifiers.is_const = true;
      consume();
    }
    else {
      break;
    }
  }

  return modifiers;
}

result<attribute> parser::parse_attribute() {
  result<token> expect_at = expect_consume(AT, "Expected '@' to begin attribute");
  result<token> expect_id = expect_consume(IDENTIFIER, "Expected identifier for attribute name");
  result<token> curr = current();

  vl_checkresult(expect_at);
  vl_checkresult(expect_id);
  vl_checkresult(curr);

  attribute attr;
  attr.identifier = *expect_id;

  if (curr->type == PAREN_OPEN) {
    consume();

    while (true) {
      curr = current();
      vl_checkresult(curr);

      if (curr->type == PAREN_CLOSE) {
        break;
      }

      result<token> tok = consume();
      attr.arguments.push_back(*tok);
    }

    consume();
  }

  return attr;
}

result<p_type_node_t> parser::parse_generic() {
  using generics_t = generic_type_node::generics_t;

  result<token> identifier = consume();
  result<modifiers> modifiers = parse_modifiers();

  vl_checkresult(identifier);
  vl_checkresult(modifiers);

  generics_t generics;

  result<token> expect_lt = expect_consume(OP_LT, "Expected '<' to open type generic");
  result<token> curr = current();

  vl_checkresult(expect_lt);
  vl_checkresult(curr);

  while (curr->type != OP_GT) {
    result<p_type_node_t> result_type = parse_type();
    result<token> result_token = current();

    vl_checkresult(result_type);
    vl_checkresult(result_token);

    generics.emplace_back(std::move(result_type.value()));

    if (curr->type != OP_GT) {
      result<token> expect_comma = expect_consume(COMMA, "Expected ',' to seperate type generics");

      vl_checkresult(expect_comma);
    }
  }

  result<token> expect_gt = expect_consume(OP_GT, "Expected '>' to close type generic");

  vl_checkresult(expect_gt);

  return std::make_unique<generic_type_node>(
    identifier.value(), std::move(generics), modifiers.value()
  );
}

result<p_type_node_t> parser::parse_type_primary() {
  static std::unordered_map<std::string, value_type> primitive_map = {
    {"nil", value_type::nil},
    {"int", value_type::integer},
    {"float", value_type::floating_point},
    {"bool", value_type::boolean},
    {"string", value_type::string},
  };

  result<token> tok = current();

  vl_checkresult(tok);

  switch (tok->type) {
  case IDENTIFIER:
  case LIT_NIL: {
    auto it = primitive_map.find(tok->lexeme);
    if (it != primitive_map.end()) {
      return std::make_unique<primitive_type_node>(consume().value(), it->second);
    }

    return parse_generic();
  }
  case PAREN_OPEN: {
    using parameters = function_type_node::parameter_vector;
    parameters params;

    result<token> curr = consume();
    vl_checkresult(curr);

    while (true) {
      result<token> tok = current();
      vl_checkresult(tok);

      if (tok->type == PAREN_CLOSE) {
        consume();
        return std::make_unique<primitive_type_node>(*tok, value_type::nil);
      }

      result<p_type_node_t> type_result = parse_type();
      vl_checkresult(type_result);

      params.emplace_back(
        token(token_type::IDENTIFIER, "FUCK", 0, 0, 0), modifiers{}, type_result.value()->clone()
      );

      if (tok->type != PAREN_CLOSE) {
        result<token> expect_comma =
          expect_consume(COMMA, "Expected ',' to seperate function parameter types");

        vl_checkresult(expect_comma);
      }
    }

    result<token> expect_pc =
      expect_consume(PAREN_CLOSE, "Expected ')' to close function type parameters");
    result<token> expect_rt =
      expect_consume(RETURNS, "Expected '->' to specify function return type");
    result<p_type_node_t> return_type = parse_type();

    vl_checkresult(expect_pc);
    vl_checkresult(expect_rt);
    vl_checkresult(return_type);

    return std::make_unique<function_type_node>(std::move(params), std::move(return_type.value()));
  }
  default: {
    break;
  }
  }

  return tl::unexpected<parse_error>({
    position,
    std::format("Unexpected token '{}' while parsing type primary", tok->lexeme),
  });
}

result<p_type_node_t> parser::parse_type() {
  return parse_type_primary();
}

result<p_expr_node_t> parser::parse_primary() {
  // Get the current token.
  result<token> tok = current();
  vl_checkresult(tok);

  switch (tok->type) {
  case LIT_INT:
  case LIT_HEX: {
    consume();
    int value = std::stoi(tok->lexeme);
    return std::make_unique<lit_expr_node>(*tok, value);
  }
  case LIT_FLOAT: {
    consume();
    float value = std::stof(tok->lexeme);
    return std::make_unique<lit_expr_node>(*tok, value);
  }
  case LIT_BINARY: {
    consume();
    // Skip the "0b" prefix before converting.
    return std::make_unique<lit_expr_node>(
      *tok, static_cast<int>(std::bitset<64>(tok->lexeme.substr(2)).to_ullong())
    );
  }
  case LIT_NIL:
    consume();
    return std::make_unique<lit_expr_node>(*tok, std::monostate());
  case LIT_BOOL:
    consume();
    return std::make_unique<lit_expr_node>(*tok, tok->lexeme == "true");
  case IDENTIFIER:
    consume();
    return std::make_unique<sym_expr_node>(*tok);
  case LIT_STRING:
    consume();
    return std::make_unique<lit_expr_node>(*tok, tok->lexeme);
  case OP_SUB: { // Unary minus operator
    consume();
    // Directly parse the operand; no need to keep a copy of token here.
    result<p_expr_node_t> expr = parse_primary();
    vl_checkresult(expr);

    return std::make_unique<unary_expr_node>(std::move(*expr));
  }
  case BRACKET_OPEN: {
    result<token> br_open = consume();

    while (true) {
      result<token> curr = current();
      vl_checkresult(curr);

      if (curr->type == BRACKET_CLOSE) {
        break;
      }
    }
  }
  default:
    break;
  }

  // If none of the valid token types were matched, return an error.
  return tl::unexpected<parse_error>(
    {position, std::format("Unexpected token '{}' while parsing primary expression", tok->lexeme)}
  );
}

result<p_expr_node_t> parser::parse_postfix(p_expr_node_t lhs) {
  // Process all postfix expressions (member access, array indexing, function calls, type casts)
  while (true) {
    result<token> curr = current();
    if (!curr.has_value()) {
      return lhs;
    }

    switch (curr->type) {
    case DOT: { // Member access: obj.property
      consume();
      result<token> index_token =
        expect_consume(IDENTIFIER, "Expected identifier while parsing index");
      vl_checkresult(index_token);

      lhs = std::make_unique<index_expr_node>(
        std::move(lhs), std::make_unique<sym_expr_node>(*index_token)
      );

      continue;
    }
    case BRACKET_OPEN: { // Array indexing: obj[expr]
      consume();
      result<p_expr_node_t> index = parse_expr();
      vl_checkresult(index);

      result<token> expect_br =
        expect_consume(BRACKET_CLOSE, "Expected ']' to close index expression");
      vl_checkresult(expect_br);

      lhs = std::make_unique<index_expr_node>(std::move(lhs), std::move(*index));
      continue;
    }
    case PAREN_OPEN: { // Function calls: func(arg1, ...)
      consume();
      std::vector<p_expr_node_t> arguments;

      while (true) {
        result<token> curr = current();
        vl_checkresult(curr);

        if (curr->type == PAREN_CLOSE) {
          break;
        }

        result<p_expr_node_t> expr = parse_expr();
        vl_checkresult(expr);

        arguments.emplace_back(std::move(*expr));

        // After an argument, check for a comma.
        curr = current();
        vl_checkresult(curr);

        if (curr->type == COMMA) {
          consume();
        }
        else {
          break;
        }
      }

      // Expect the closing parenthesis.
      result<token> expect_par =
        expect_consume(PAREN_CLOSE, "Expected ')' to close function call arguments");
      vl_checkresult(expect_par);

      lhs = std::make_unique<call_expr_node>(std::move(lhs), std::move(arguments));
      continue;
    }
    case KW_AS: { // Type casting: expr as Type
      consume();

      result<p_type_node_t> type_result = parse_type();
      vl_checkresult(type_result);

      lhs = std::make_unique<cast_expr_node>(std::move(lhs), std::move(*type_result));
      continue;
    }
    default:
      // No more postfix tokens; return the accumulated expression.
      return lhs;
    }
  }
}

result<p_expr_node_t> parser::parse_binary(int precedence) {
  // Parse the primary expression first.
  result<p_expr_node_t> prim = parse_primary();
  vl_checkresult(prim);

  // Parse any postfix operations that apply to the primary expression.
  result<p_expr_node_t> lhs = parse_postfix(std::move(*prim));
  vl_checkresult(lhs);

  // Parse binary operators according to precedence.
  while (position < unit_ctx.tokens->size()) {
    result<token> op = current();
    vl_checkresult(op);

    // Stop if the token is not an operator.
    if (!op->is_operator()) {
      break;
    }

    int op_prec = op->bin_prec();
    if (op_prec < precedence) {
      break;
    }

    consume();

    result<p_expr_node_t> rhs = parse_binary(op_prec + 1);
    vl_checkresult(rhs);

    lhs = std::make_unique<bin_expr_node>(*op, std::move(*lhs), std::move(*rhs));
  }

  return lhs;
}

result<p_expr_node_t> parser::parse_expr() {
  return parse_binary(0);
}

result<p_stmt_node_t> parser::parse_declaration() {
  result<token> first = consume();
  vl_checkresult(first);

  token_type first_type = first->type;

  bool is_local = false;
  bool is_global = false;
  bool is_const = false;

  // Otherwise, handle local/global/const
  is_local = first_type == KW_LOCAL;
  is_global = first_type == KW_GLOBAL;
  is_const = first_type == KW_CONST;

  result<token> curr = current();
  vl_checkresult(curr);

  // Detect `fn` at the start (fn name())
  if (first_type == KW_FUNC) {
    goto parse_function;
  }

  if (is_local && curr->type == KW_CONST) {
    is_const = true;
    consume();
  }

  curr = current();
  vl_checkresult(curr);

  if (curr->type == KW_FUNC) {
    consume();

  parse_function:
    result<token> identifier = expect_consume(IDENTIFIER, "Expected function name after 'fn'");
    result<token> expect_par =
      expect_consume(PAREN_OPEN, "Expected '(' to open function parameters");

    vl_checkresult(identifier);
    vl_checkresult(expect_par);

    std::vector<param_node> parameters;

    while (true) {
      curr = current();
      if (curr.has_value() && curr->type == PAREN_CLOSE) {
        break;
      }

      result<modifiers> modifiers = parse_modifiers();
      result<token> param_name =
        expect_consume(IDENTIFIER, "Expected identifier for function parameter name");
      result<token> expect_col = expect_consume(COLON, "Expected ':' between parameter and type");
      result<p_type_node_t> param_type = parse_type();

      vl_checkresult(modifiers);
      vl_checkresult(param_name);
      vl_checkresult(expect_col);
      vl_checkresult(param_type);

      parameters.emplace_back(*param_name, *modifiers, std::move(*param_type));

      curr = current();
      vl_checkresult(curr);

      if (curr->type != PAREN_CLOSE) {
        result<token> expect_comma =
          expect_consume(COMMA, "Expected ',' between function parameters");
        vl_checkresult(expect_comma);
      }
      else {
        break;
      }
    }

    modifiers modifiers{is_const};

    result<token> expect_rpar =
      expect_consume(PAREN_CLOSE, "Expected ')' to close function parameters");
    result<token> expect_rets =
      expect_consume(RETURNS, "Expected '->' to denote function return type");
    result<p_type_node_t> returns = parse_type();
    result<p_stmt_node_t> body_scope = parse_scope();

    vl_checkresult(expect_rpar);
    vl_checkresult(expect_rets);
    vl_checkresult(returns);
    vl_checkresult(body_scope);

    return std::make_unique<func_stmt_node>(
      is_global,
      modifiers,
      *identifier,
      std::move(*body_scope),
      std::move(*returns),
      std::move(parameters)
    );
  }

  // If not a function, continue parsing as a variable declaration
  p_type_node_t type;
  result<token> identifier =
    expect_consume(IDENTIFIER, "Expected identifier for variable declaration");
  curr = current();

  vl_checkresult(identifier);
  vl_checkresult(curr);

  if (curr->type == COLON) {
    consume();

    result<p_type_node_t> temp = parse_type();
    vl_checkresult(temp);

    type = std::move(*temp);
  }
  else {
    type = std::make_unique<auto_type_node>(curr->position, curr->position + curr->lexeme.length());
  }

  result<token> expect_eq = expect_consume(EQUAL, "Expected '=' for variable declaration");
  result<p_expr_node_t> value = parse_expr();

  vl_checkresult(expect_eq);
  vl_checkresult(value);

  type->expression = value->get(); // Attach expression reference to type

  return std::make_unique<decl_stmt_node>(
    is_global, modifiers{is_const}, *identifier, std::move(*value), std::move(type)
  );
}

result<p_stmt_node_t> parser::parse_scope() {
  std::vector<p_stmt_node_t> scope_statements;

  result<token> curr = current();
  vl_checkresult(curr);

  if (curr->type == BRACE_OPEN) {
    result<token> expect_br = expect_consume(BRACE_OPEN, "Expected '{' to open scope");
    vl_checkresult(expect_br);

    while (true) {
      result<token> curr = current();
      vl_checkresult(curr);

      if (curr->type == BRACE_CLOSE) {
        break;
      }

      result<p_stmt_node_t> stmt = parse_stmt();
      vl_checkresult(stmt);

      scope_statements.emplace_back(std::move(*stmt));
    }

    expect_br = expect_consume(BRACE_CLOSE, "Expected '}' to close scope");
    vl_checkresult(expect_br);
  }
  else if (curr->type == COLON) {
    result<token> expect_col = expect_consume(COLON, "Expected ':' to open single-statment scope");
    vl_checkresult(expect_col);

    result<p_stmt_node_t> stmt = parse_stmt();
    vl_checkresult(stmt);

    scope_statements.emplace_back(std::move(*stmt));
  }
  else {
    return tl::unexpected<parse_error>(
      {position, "Expected '{' or ':' to open scope or single-statement scope"}
    );
  }

  return std::make_unique<scope_stmt_node>(std::move(scope_statements));
}

result<p_stmt_node_t> parser::parse_if() {
  using elseif_node = if_stmt_node::elseif_node;

  consume();

  result<p_expr_node_t> condition = parse_expr();
  result<p_stmt_node_t> scope = parse_scope();

  vl_checkresult(condition);
  vl_checkresult(scope);

  std::optional<p_stmt_node_t> else_scope;
  std::vector<elseif_node> elseif_nodes;

  while (true) {
    result<token> curr = current();
    vl_checkresult(curr);

    if (curr->type != KW_ELIF) {
      break;
    }

    result<p_expr_node_t> elseif_condition = parse_expr();
    result<p_stmt_node_t> elseif_scope = parse_scope();

    vl_checkresult(elseif_condition);
    vl_checkresult(elseif_scope);

    elseif_node elseif(std::move(*elseif_condition), std::move(*elseif_scope));
    elseif_nodes.emplace_back(std::move(elseif));
  }

  result<token> curr = current();
  vl_checkresult(curr);

  if (curr->type == KW_ELSE) {
    consume();

    result<p_stmt_node_t> else_scope_inner = parse_scope();
    vl_checkresult(else_scope_inner);

    else_scope = std::move(*else_scope_inner);
  }
  else {
    else_scope = nullptr;
  }

  return std::make_unique<if_stmt_node>(
    std::move(*condition), std::move(*scope), std::move(*else_scope), std::move(elseif_nodes)
  );
}

result<p_stmt_node_t> parser::parse_return() {
  consume();

  result<p_expr_node_t> expr = parse_expr();

  vl_checkresult(expr);

  return std::make_unique<return_stmt_node>(std::move(*expr));
}

result<p_stmt_node_t> parser::parse_while() {
  consume();

  result<p_expr_node_t> condition = parse_expr();
  result<p_stmt_node_t> body = parse_scope();

  vl_checkresult(condition);
  vl_checkresult(body);

  return std::make_unique<while_stmt_node>(std::move(*condition), std::move(*body));
}

result<p_stmt_node_t> parser::parse_stmt() {
  result<token> initial_token = current();
  vl_checkresult(initial_token);

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
    result<token> con = consume();
    vl_checkresult(con);
    return std::make_unique<break_stmt_node>(*con);
  }
  case KW_CONTINUE: {
    result<token> con = consume();
    vl_checkresult(con);
    return std::make_unique<continue_stmt_node>(*con);
  }
  default:
    result<p_expr_node_t> lvalue = parse_expr();
    result<token> curr = current();

    vl_checkresult(lvalue);
    vl_checkresult(curr);

    if (curr->type == EOF_) {
      return std::make_unique<expr_stmt_node>(std::move(*lvalue));
    }

    result<token> pk = peek();
    vl_checkresult(pk);

    if (curr->lexeme == "=" || (curr->is_operator() && pk->lexeme == "=")) {
      result<token> possible_augment = consume();
      vl_checkresult(possible_augment);

      if (possible_augment->lexeme != "=") {
        consume();
      }

      result<p_expr_node_t> rvalue = parse_expr();

      return std::make_unique<assign_stmt_node>(
        std::move(*lvalue), *possible_augment, std::move(*rvalue)
      );
    }

    return std::make_unique<expr_stmt_node>(std::move(*lvalue));
  }

  vl_unreachable;
  return nullptr;
}

bool parser::parse() {
  while (true) {
    result<token> curr = current();
    vl_checkresult_mainfn(curr);

    if (curr->type == EOF_) {
      break;
    }

    while (true) {
      curr = current();
      vl_checkresult_mainfn(curr);

      if (curr->type != AT) {
        break;
      }

      result<attribute> attrib = parse_attribute();
      vl_checkresult_mainfn(attrib);

      attrib_buffer.push_back(*attrib);
    }

    result<p_stmt_node_t> stmt = parse_stmt();
    vl_checkresult_mainfn(stmt);

    stmt.value()->attributes = attrib_buffer;
    attrib_buffer.clear();

    unit_ctx.ast->statements.emplace_back(std::move(*stmt));
  }

  return false;
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#elifdef __clang__
#pragma clang diagnostic pop
#elifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace via
