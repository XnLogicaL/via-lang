// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include <common/strutils.h>

namespace via {

char* ustrdup(const std::string& str) {
  char* chars = new char[str.size() + 1];
  std::strcpy(chars, str.c_str());
  return chars;
}

char* ustrdup(const char* str) {
  char* chars = new char[std::strlen(str) + 1];
  std::strcpy(chars, str);
  return chars;
}

uint32_t ustrhash(const char* str) {
  static constexpr uint32_t BASE = 31u; // Prime number
  static constexpr uint32_t MOD = 0xFFFFFFFFu;

  uint32_t hash = 0;
  while (char chr = *str++) {
    hash = (hash * BASE + static_cast<uint32_t>(chr)) % MOD;
  }

  return hash;
}

std::string ustresc(const std::string& str) {
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
