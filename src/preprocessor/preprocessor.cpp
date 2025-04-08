//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================
#include "Preprocessor.h"
#include "Lexer.h"

namespace via {

using enum TokenType;

void Preprocessor::declare_default() {}

bool Preprocessor::preprocess() {
  return false;
}

} // namespace via
