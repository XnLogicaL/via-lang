/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "instruction.h"
#include <string_view>

namespace via
{

// Type alias for C strings
// Useful if it needs to be changed later
using viaRawString = const char *;

// Default hashmap used in via, can be changed with an appropriate alternative
template<typename K, typename T>
using viaHashMap = std::unordered_map<K, T>;

// Instruction type alias
using viaInstruction = Compilation::viaInstruction;

// Default hash type
using viaHash = uint32_t;

// Default label key
using viaLabelKey = std::string_view;

// Default variable identifiers
using viaLocalIdentifier = std::string_view;
using viaGlobalIdentifier = std::string;

// Type aliases for register stuff
using viaRegisterOffset = Compilation::viaRegisterOffset;
using viaRegisterType = Compilation::viaRegisterType;
using viaRegister = Compilation::viaRegister;

// VM related
using viaCallArgC = uint16_t;
using viaExitCode = int8_t;
#ifdef VIA_LONGJMP
using viaJmpOffset = long int;
#else
using viaJmpOffset = int;
#endif

} // namespace via