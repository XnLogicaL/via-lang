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

std::string escape_string(const std::string& str) {
  std::ostringstream oss;
  for (unsigned char c : str) {
    switch (c) {
      // clang-format off
    case '\a': oss << "\\a"; break;
    case '\b': oss << "\\b"; break;
    case '\f': oss << "\\f"; break;
    case '\n': oss << "\\n"; break;
    case '\r': oss << "\\r"; break;
    case '\t': oss << "\\t"; break;
    case '\v': oss << "\\v"; break;
    case '\\': oss << "\\\\"; break;
    case '\"': oss << "\\\""; break;
      // clang-format on
    default:
      // If the character is printable, output it directly.
      // Otherwise, output it as a hex escape.
      if (std::isprint(c))
        oss << c;
      else
        oss << "\\x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
      break;
    }
  }

  return oss.str();
}

} // namespace via
