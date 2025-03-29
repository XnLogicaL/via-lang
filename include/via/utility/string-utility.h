// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef vl_has_header_strutils_h
#define vl_has_header_strutils_h

#include "common.h"

namespace via {

char* duplicate_string(const std::string&);
char* duplicate_string(const char*);
uint32_t hash_string_custom(const char*);

} // namespace via

#endif
