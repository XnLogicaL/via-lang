// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "globals.h"
#include "ast.h"
#include "compiler-types.h"

namespace via {

using index_query_result = global_holder::index_query_result;
using global_query_result = global_holder::global_query_result;
using global_vector = global_holder::global_vector;

size_t global_holder::size() {
  return globals.size();
}

void global_holder::declare_global(global_obj global) {
  globals.emplace_back(std::move(global));
}

bool global_holder::was_declared(const global_obj& global) {
  return was_declared(global.symbol);
}

bool global_holder::was_declared(const std::string& symbol) {
  for (const global_obj& global : globals) {
    if (global.symbol == symbol) {
      return true;
    }
  }

  return false;
}

index_query_result global_holder::get_index(const global_obj& global) {
  return get_index(global.symbol);
}

index_query_result global_holder::get_index(const std::string& symbol) {
  uint64_t index = 0;
  for (const global_obj& global : globals) {
    if (global.symbol == symbol) {
      return index;
    }

    ++index;
  }

  return std::nullopt;
}

global_query_result global_holder::get_global(const std::string& symbol) {
  for (const global_obj& global : globals) {
    if (global.symbol == symbol) {
      return global_obj{global.token, global.symbol, global.type->clone()};
    }
  }

  return std::nullopt;
}

global_query_result global_holder::get_global(size_t index) {
  global_obj& global = globals.at(index);
  return global_obj{global.token, global.symbol, global.type->clone()};
}

const global_vector& global_holder::get() {
  return globals;
}

void global_holder::declare_builtins() {
  token tok = token(token_type::IDENTIFIER, "", 0, 0, 0);

  p_type_node_t nil_type = std::make_unique<primitive_type_node>(tok, value_type::nil);
  p_type_node_t str_type = std::make_unique<primitive_type_node>(tok, value_type::string);

  auto make_argument = [](const std::string& id, p_type_node_t type) -> param_node {
    return {token(token_type::IDENTIFIER, id, 0, 0, 0), modifiers{}, std::move(type)};
  };

  std::vector<param_node> print_args;
  print_args.push_back(make_argument("arg0", str_type->clone()));

  global_obj print = {
    tok,
    "print",
    std::make_unique<function_type_node>(std::move(print_args), nil_type->clone()),
  };

  std::vector<param_node> println_args;
  println_args.push_back(make_argument("arg0", str_type->clone()));

  global_obj println = {
    tok,
    "println",
    std::make_unique<function_type_node>(std::move(println_args), nil_type->clone()),
  };

  globals.emplace_back(std::move(print));
  globals.emplace_back(std::move(println));
}

} // namespace via
