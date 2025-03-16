// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "memutils.h"

VIA_NAMESPACE_BEGIN

void dump_raw_memory(const void* ptr, size_t size) {
    const uint8_t* data = static_cast<const uint8_t*>(ptr);
    for (size_t i = 0; i < size; i++) {
        if (i % 16 == 0) {
            if (i != 0)
                std::cout << "  ";
            std::cout << std::endl << std::setw(8) << std::setfill('0') << std::hex << i << ": ";
        }
        std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)data[i] << " ";
    }
    std::cout << std::endl;
}

void dump_memory(const void* ptr, size_t size, const std::string& label) {
    std::cout << "Memory dump for: " << label << std::endl;
    dump_raw_memory(ptr, size);
}

VIA_NAMESPACE_END
