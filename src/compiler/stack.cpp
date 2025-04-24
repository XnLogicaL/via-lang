// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "stack.h"
#include "context.h"

namespace via {

std::optional<StackVariable*> CompilerVariableStack::get_local_by_id(size_t pos) {
  if (pos > size()) {
    return std::nullopt;
  }

  return &this->operator[](pos);
}

std::optional<StackVariable*> CompilerVariableStack::get_local_by_symbol(const symbol_t& symbol) {
  for (int i = this->size() - 1; i >= 0; --i) {
    if (this->operator[](i).symbol == symbol) {
      return &this->operator[](i);
    }
  }

  return std::nullopt;
}

std::optional<operand_t> CompilerVariableStack::find_local_id(const symbol_t& symbol) {
  for (int i = this->size() - 1; i >= 0; --i) {
    if (this->operator[](i).symbol == symbol) {
      return i;
    }
  }

  return std::nullopt;
}

void CompilerFunctionStack::push_main_function(TransUnitContext& unit_ctx) {
  ScopeStmtNode* scope = unit_ctx.internal.ast_allocator.emplace<ScopeStmtNode>(
    size_t(0), size_t(0), std::vector<StmtNodeBase*>{}
  );

  PrimTypeNode* ret = unit_ctx.internal.ast_allocator.emplace<PrimTypeNode>(
    Token(TokenType::IDENTIFIER, "Nil", 0, 0, 0), Value::Tag::Nil
  );

  FuncDeclStmtNode* func = unit_ctx.internal.ast_allocator.emplace<FuncDeclStmtNode>(
    size_t(0),
    size_t(0),
    false,
    StmtModifiers{},
    Token(TokenType::IDENTIFIER, "main", 0, 0, 0),
    scope,
    ret,
    std::vector<ParamStmtNode>{}
  );

  this->push_back({
    .stack_pointer = 0,
    .decl = func,
    .locals = {},
  });
}

} // namespace via
