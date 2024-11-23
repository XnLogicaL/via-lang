/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "shared.h"
#include "types.h"
#include <cstdint>

namespace via
{

using viaRegisterSize = uint16_t;
using viaRegisterCount = uint8_t;

class RegisterAllocator
{
public:
    RegisterAllocator();
    ~RegisterAllocator();

    // This is inlined due to performance critical and frequent usage
    template<typename T>
    inline T *get(const viaRegister &R)
    {
        const viaRegisterOffset off = R.offset;
        static T *registers[] = {
            reinterpret_cast<T *>(gpr),
            reinterpret_cast<T *>(ar),
            reinterpret_cast<T *>(rr),
        };

        // Ensure R.type maps to a valid index in registers[]
        if (R.type >= viaRegisterType::R && R.type <= viaRegisterType::RR)
            return registers[static_cast<uint8_t>(R.type) - static_cast<uint8_t>(viaRegisterType::R)] + off;

        return nullptr;
    }

    void flush(viaRegisterType);
    void print(viaRegisterType);

private:
    viaValue *gpr;
    viaValue *ar;
    viaValue *rr;

    viaRegisterSize get_size(viaRegisterType);
    void *alloc(viaRegisterType, viaRegisterCount);
    void prealloc();
};

} // namespace via