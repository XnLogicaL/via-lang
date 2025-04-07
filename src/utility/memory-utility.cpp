//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================
#include "memory-utility.h"
#include "common.h"

namespace via {

std::string get_raw_memory_dump(const void* ptr, size_t size) {
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

void dump_raw_memory(const void* ptr, size_t size) {
  std::cout << get_raw_memory_dump(ptr, size);
}

void dump_memory(const void* ptr, size_t size, const std::string& label) {
  std::cout << "Memory dump for: " << label << std::endl;
  dump_raw_memory(ptr, size);
}

} // namespace via
