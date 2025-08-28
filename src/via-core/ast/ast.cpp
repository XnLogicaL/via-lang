// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "ast.h"

namespace via
{

namespace ast
{}  // namespace ast

namespace debug
{

void dump(const SyntaxTree& ast)
{
  fmt::println("debug::dump(SyntaxTree):");

  for (const auto* st : ast) {
    fmt::println("  {}", reinterpret_cast<const void*>(st));
  }
}

}  // namespace debug

}  // namespace via
