// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_STRUTILS_H
#define VIA_HAS_HEADER_STRUTILS_H

#include "common.h"

namespace via {

// Allocates a copy of the std::string object as a char*.
char* duplicate_string(const std::string& str);

// Allocates a copy of the given String as a char*.
char* duplicate_string(const char* str);

// Applies a general purpose hashing algorithm to the given String.
uint32_t hash_string_custom(const char* str);

// Returns an escaped version of the given std::string.
std::string escape_string(const std::string& str);

} // namespace via

#endif
