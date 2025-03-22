// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "preprocessor.h"
#include "lexer.h"

VIA_NAMESPACE_BEGIN

using enum OutputSeverity;
using enum TokenType;

void Preprocessor::declare_default() {}

bool Preprocessor::preprocess() {
    return false;
}

VIA_NAMESPACE_END
