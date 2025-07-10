// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_USTRING_H
#define VIA_USTRING_H

#include "common.h"

namespace via {

// Allocates a copy of the std::string object as a char*.
char* dupe_string(const String& str);

// Allocates a copy of the given String as a char*.
char* dupe_string(const char* str);

// Applies a general purpose hashing algorithm to the given String.
u32 hash_string(const char* str);

// Returns an escaped version of the given std::string.
String escape_string(const String& str);

} // namespace via

#endif
