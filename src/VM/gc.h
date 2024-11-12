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
    std::vector<const void*> del_list;

public:

    inline void collect()
    {
        if (free_list.size() == 0)
        {
            return;
        }

        state.collections++;

        for (const auto &p : free_list)
        {
            std::free(const_cast<void*>(p));
        }

        for (const auto &p : del_list)
        {
            delete p;
        }
    }

    inline void add(const void* p)
    {
        free_list.push_back(p);
    }

    inline void add_heap(const void* p)
    {
        del_list.push_back(p);
    }

};

} // namespace VM

} // namespace via

#endif // VIA_GC_H
