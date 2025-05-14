// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "memory.h"

namespace via {

std::string uget_memdump(const void* ptr, size_t size) {
  std::ostringstream oss;

  const uint8_t* data = static_cast<const uint8_t*>(ptr);
  for (size_t i = 0; i < size; i++) {
    if (i % 16 == 0) {
      if (i != 0) {
        oss << "  ";
      }

      oss << std::endl << std::setw(8) << std::setfill('0') << std::hex << i << ": ";
    }

    oss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int32_t>(data[i]) << " ";
  }

  oss << std::endl;
  return oss.str();
}

void umemdumpraw(const void* ptr, size_t size) {
  std::cout << uget_memdump(ptr, size);
}

void umemdump(const void* ptr, size_t size, const std::string& label) {
  std::cout << "Memory dump for: " << label << std::endl;
  umemdumpraw(ptr, size);
}

} // namespace via
