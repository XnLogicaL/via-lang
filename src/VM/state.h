#ifndef VIA_STATE_H
#define VIA_STATE_H

#include "common.h"

namespace via::VM
{

struct VMState
{
    bool is_running;
    int exit_code;
    std::string exit_message;
    std::unordered_map<std::string, int> fflags;

    VMState() 
        : is_running(false)
        , exit_code(0)
        , exit_message("")
    {
        fflags = {
            {"FFLAG_ABRT", false},  // Abort execution in the next dispatch
            {"FFLAG_SKIP", false},  // Skip instruction in the next dispatch (debounces)
        };
    }
};

} // namespace via::VM

#endif // VIA_STATE_H
