/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "instruction.h"
#include <string_view>

namespace via
{

// Type alias for C strings
// Useful if it needs to be changed later
using viaRawString_t = const char *;

// Default hashmap used in via, can be changed with an appropriate alternative
template<typename K, typename T>
using viaHashMap_t = std::unordered_map<K, T>;

// Default hash type
using viaHash_t = uint32_t;

// Default label key
using viaLabelKey_t = std::string_view;

// Default variable identifiers
using viaVariableIdentifier_t = viaHash_t;

// VM related
using viaCallArgC_t = uint16_t;
using viaExitCode_t = int8_t;
#ifdef VIA_LONGJMP
using viaJmpOffset_t = long int;
#else
using viaJmpOffset_t = int;
#endif
using viaThreadId_t = uint32_t;

} // namespace via