#pragma once

#include <map>

class Heap {
private:
    std::map<void*, std::size_t> allocations;

public:
    void* alloc(std::size_t size) {
        void* ptr = std::malloc(size);

        if (ptr != nullptr)
            allocations[ptr] = size;

        return ptr;
    }

    void free(void* ptr) {
        auto it = allocations.find(ptr);

        if (it != allocations.end()) {
            std::size_t size = it->second;
            std::free(ptr);

            allocations.erase(it);
        }
    }

    ~Heap() {
        for (const auto& alloc : allocations)
            std::free(alloc.first);

        allocations.clear();
    }
};