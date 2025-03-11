// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_DEF_H
#define _VIA_DEF_H

#include "common.h"
#include "token.h"

VIA_NAMESPACE_BEGIN

struct Definition {
    size_t begin;
    size_t end;
    size_t line;

    std::string        identifier;
    std::vector<Token> replacement;
};

VIA_NAMESPACE_END

#endif
