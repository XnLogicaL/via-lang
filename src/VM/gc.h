#ifndef VIA_GC_H
#define VIA_GC_H

#include "common.h"

namespace via
{
    
namespace VM
{

struct GCState
{
    size_t collections;
};
    
class GarbageCollector
{
    GCState state;
    std::vector<const void*> free_list;

public:

    inline void collect()
    {
        if (free_list.size() == 0)
        {
            return;
        }

        state.collections++;

        for (const auto& p : free_list)
        {
            std::free(const_cast<void*>(p));
        }
    }

    inline void add(const void* p)
    {
        free_list.push_back(p);
    }

};

} // namespace VM

} // namespace via

#endif // VIA_GC_H
