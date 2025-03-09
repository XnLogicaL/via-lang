// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_MACRO_H
#define _VIA_MACRO_H

#include "common.h"
#include "token.h"

VIA_NAMESPACE_BEGIN

struct Macro {
    SIZE begin;
    SIZE end;
    SIZE line;

    std::string              name;   // Name of the macro
    std::vector<std::string> params; // Macro parameter names
    std::vector<Token>       body;   // Macro body as a list of tokens
};

VIA_NAMESPACE_END

#endif
