// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "preprocessor.h"
#include "lexer.h"

namespace via {

using enum TokenType;

void Preprocessor::declare_default() {}

bool Preprocessor::preprocess() {
  return false;
}

} // namespace via
