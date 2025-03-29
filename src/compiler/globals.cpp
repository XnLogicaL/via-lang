// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "globals.h"
#include "ast.h"
#include "compiler-types.h"

VIA_NAMESPACE_BEGIN

using index_query_result = GlobalTracker::index_query_result;
using global_query_result = GlobalTracker::global_query_result;
using global_vector = GlobalTracker::global_vector;

size_t GlobalTracker::size() {
  return globals.size();
}

void GlobalTracker::declare_global(Global global) {
  globals.emplace_back(std::move(global));
}

bool GlobalTracker::was_declared(const Global& global) {
  return was_declared(global.symbol);
}

bool GlobalTracker::was_declared(const std::string& symbol) {
  for (const Global& global : globals) {
    if (global.symbol == symbol) {
      return true;
    }
  }

  return false;
}

index_query_result GlobalTracker::get_index(const Global& global) {
  return get_index(global.symbol);
}

index_query_result GlobalTracker::get_index(const std::string& symbol) {
  uint64_t index = 0;
  for (const Global& global : globals) {
    if (global.symbol == symbol) {
      return index;
    }

    ++index;
  }

  return std::nullopt;
}

global_query_result GlobalTracker::get_global(const std::string& symbol) {
  for (const Global& global : globals) {
    if (global.symbol == symbol) {
      return Global{global.token, global.symbol, global.type->clone()};
    }
  }

  return std::nullopt;
}

global_query_result GlobalTracker::get_global(size_t index) {
  Global& global = globals.at(index);
  return Global{global.token, global.symbol, global.type->clone()};
}

const global_vector& GlobalTracker::get() {
  return globals;
}

void GlobalTracker::declare_builtins() {
  static Token tok = Token(TokenType::IDENTIFIER, "", 0, 0, 0);

  pTypeNode ret_void = std::make_unique<PrimitiveTypeNode>(tok, ValueType::nil);

  std::vector<pTypeNode> print_args;
  print_args.emplace_back(std::make_unique<PrimitiveTypeNode>(tok, ValueType::string));

  Global print = {
    tok,
    "print",
    std::make_unique<FunctionTypeNode>(std::move(print_args), ret_void->clone()),
  };

  std::vector<pTypeNode> println_args;
  println_args.emplace_back(std::make_unique<PrimitiveTypeNode>(tok, ValueType::string));

  Global println = {
    tok,
    "println",
    std::make_unique<FunctionTypeNode>(std::move(println_args), ret_void->clone()),
  };

  globals.emplace_back(std::move(print));
  globals.emplace_back(std::move(println));
}

VIA_NAMESPACE_END
