//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================
#include "globals.h"
#include "ast.h"
#include "compiler-types.h"

namespace via {

using index_query_result = GlobalHolder::index_query_result;
using global_query_result = GlobalHolder::global_query_result;
using global_vector = GlobalHolder::global_vector;

size_t GlobalHolder::size() {
  return globals.size();
}

void GlobalHolder::declare_global(CompilerGlobal global) {
  globals.emplace_back(std::move(global));
}

bool GlobalHolder::was_declared(const CompilerGlobal& global) {
  return was_declared(global.symbol);
}

bool GlobalHolder::was_declared(const std::string& symbol) {
  for (const CompilerGlobal& global : globals) {
    if (global.symbol == symbol) {
      return true;
    }
  }

  return false;
}

index_query_result GlobalHolder::get_index(const CompilerGlobal& global) {
  return get_index(global.symbol);
}

index_query_result GlobalHolder::get_index(const std::string& symbol) {
  uint64_t index = 0;
  for (const CompilerGlobal& global : globals) {
    if (global.symbol == symbol) {
      return index;
    }

    ++index;
  }

  return std::nullopt;
}

global_query_result GlobalHolder::get_global(const std::string& symbol) {
  for (const CompilerGlobal& global : globals) {
    if (global.symbol == symbol) {
      return CompilerGlobal{global.tok, global.symbol, global.type};
    }
  }

  return std::nullopt;
}

global_query_result GlobalHolder::get_global(size_t index) {
  CompilerGlobal& global = globals.at(index);
  return CompilerGlobal{global.tok, global.symbol, global.type};
}

const global_vector& GlobalHolder::get() {
  return globals;
}

void GlobalHolder::declare_builtins() {
  Token tok = Token(TokenType::IDENTIFIER, "", 0, 0, 0);

  TypeNodeBase* nil_type = allocator.emplace<PrimTypeNode>(tok, IValueType::nil);
  TypeNodeBase* str_type = allocator.emplace<PrimTypeNode>(tok, IValueType::string);

  auto make_argument = [](const std::string& id, TypeNodeBase* type) -> ParamNode {
    return {Token(TokenType::IDENTIFIER, id, 0, 0, 0), StmtModifiers{}, std::move(type)};
  };

  std::vector<ParamNode> print_args;
  print_args.push_back(make_argument("arg0", str_type));

  CompilerGlobal print = {
    tok,
    "print",
    allocator.emplace<FunctionTypeNode>(print_args, nil_type),
  };

  std::vector<ParamNode> println_args;
  println_args.push_back(make_argument("arg0", str_type));

  CompilerGlobal println = {
    tok,
    "println",
    allocator.emplace<FunctionTypeNode>(println_args, nil_type),
  };

  globals.emplace_back(std::move(print));
  globals.emplace_back(std::move(println));
}

} // namespace via
