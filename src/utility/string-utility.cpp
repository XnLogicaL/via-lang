// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "string-utility.h"

namespace via {

char* duplicate_string(const std::string& str) {
  char* chars = new char[str.size() + 1];
  std::strcpy(chars, str.c_str());
  return chars;
}

char* duplicate_string(const char* str) {
  char* chars = new char[std::strlen(str) + 1];
  std::strcpy(chars, str);
  return chars;
}

uint32_t hash_string_custom(const char* str) {
  static const constexpr uint32_t BASE = 31;
  static const constexpr uint32_t MOD = 0xFFFFFFFF;

  uint32_t hash = 0;
  while (char chr = *str++) {
    hash = (hash * BASE + static_cast<uint32_t>(chr)) % MOD;
  }

  return hash;
}

} // namespace via
