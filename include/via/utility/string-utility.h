// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_STRUTILS_H
#define _VIA_STRUTILS_H

#include "common.h"

VIA_NAMESPACE_BEGIN

char* duplicate_string(const std::string&);
char* duplicate_string(const char*);
uint32_t hash_string_custom(const char*);

VIA_NAMESPACE_END

#endif
