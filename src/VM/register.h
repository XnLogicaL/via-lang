/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "bytecode.h"
#include "common.h"
#include "types.h"

#include "magic_enum.hpp"

namespace via::VM
{

class RegisterAllocator
{
    via_Value *gpr;
    via_Value *ar;
    via_Value *rr;
    via_TableKey *ir;
    via_Table *sr;
    via_Number *er;

    uint8_t get_size(RegisterType type);
    void *alloc(RegisterType type, uint8_t count);
    void prealloc();

public:
    RegisterAllocator()
    {
        this->prealloc();
    }

    ~RegisterAllocator()
    {
        std::free(gpr);
        std::free(ar);
        std::free(rr);
        std::free(ir);
        std::free(er);
        std::free(sr);
    }

    template <typename T>
    inline T *get(const Register &r)
    {
        const auto off        = r.offset;
        static T *registers[] = {
            reinterpret_cast<T *>(gpr), reinterpret_cast<T *>(ar), reinterpret_cast<T *>(rr),
            reinterpret_cast<T *>(ir),  reinterpret_cast<T *>(er), reinterpret_cast<T *>(sr),
        };

        // Ensure r.type maps to a valid index in registers[]
        if (r.type >= RegisterType::R && r.type <= RegisterType::SR)
        {
            return registers[static_cast<int>(r.type) - static_cast<int>(RegisterType::R)] + off;
        }
        return nullptr;
    }

    void flush(RegisterType r);
    void print(RegisterType rt);
};

} // namespace via::VM