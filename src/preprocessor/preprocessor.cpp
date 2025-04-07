//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================
#include "preprocessor.h"
#include "lexer.h"

namespace via {

using enum token_type;

void preprocessor::declare_default() {}

bool preprocessor::preprocess() {
  return false;
}

} // namespace via
