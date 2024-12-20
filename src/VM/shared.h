/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "instruction.h"
#include <string_view>

namespace via
{

// Default hashmap used in via, can be changed with an appropriate alternative
template<typename K, typename T>
using HashMap = std::unordered_map<K, T>;

// Default hash type
using Hash = uint32_t;

// Default label key
using LabelId = std::string_view;

// Default variable identifiers
using VarId = Hash;

// VM related
using CallArgc = uint16_t;
using ExitCode = int8_t;
#ifdef VIA_LONGJMP
using JmpOffset = long int;
#else
using JmpOffset = int;
#endif
using ThreadId = uint32_t;

} // namespace via