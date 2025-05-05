// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "globals.h"

#include <parse/ast.h>
#include <codegen/types.h>

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
#define TYPE(type, ...)           allocator.emplace<type>(__VA_ARGS__)
#define DEF_TYPE(name, type, ...) TypeNodeBase* name = TYPE(type, __VA_ARGS__)
#define DEF_ARGS(name)            std::vector<ParamStmtNode> name;
#define DEF_FUNC(name, args, ret)                                                                  \
  CompilerGlobal name{__id_dummy__, #name, TYPE(FunctionTypeNode, args, ret)};
#define ADD_ARG(args, name, type)                                                                  \
  args.push_back(ParamStmtNode{                                                                    \
    Token(TokenType::IDENTIFIER, #name, 0, 0, 0), StmtModifiers{}, std::move(type)});
#define ADD_FUNC(name) globals.emplace_back(std::move(name));

  using enum Value::Tag;

  static const Token __id_dummy__ = Token(TokenType::IDENTIFIER, "<internal-identifier>", 0, 0, 0);
  DEF_TYPE(nil_type, PrimTypeNode, __id_dummy__, Nil);
  DEF_TYPE(str_type, PrimTypeNode, __id_dummy__, String);

  // print
  DEF_ARGS(print_args);
  ADD_ARG(print_args, arg0, str_type);
  DEF_FUNC(__print, print_args, nil_type);
  ADD_FUNC(__print);

  // error
  DEF_ARGS(error_args);
  ADD_ARG(print_args, arg0, str_type);
  DEF_FUNC(__error, error_args, nil_type);
  ADD_FUNC(__error);

#undef TYPE
#undef DEF_TYPE
#undef DEF_ARGS
#undef DEF_FUNC
#undef ADD_ARG
#undef ADD_FUNC
} // namespace via

} // namespace via
