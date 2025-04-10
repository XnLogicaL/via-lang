//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================

#ifndef VIA_HAS_HEADER_STRUTILS_H
#define VIA_HAS_HEADER_STRUTILS_H

#include <string>
#include <cstdint>

namespace via {

// Allocates a copy of the std::string object as a char*.
char* duplicate_string(const std::string& str);

// Allocates a copy of the given string as a char*.
char* duplicate_string(const char* str);

// Applies a general purpose hashing algorithm to the given string.
uint32_t hash_string_custom(const char* str);

// Returns an escaped version of the given std::string.
std::string escape_string(const std::string& str);

} // namespace via

#endif
