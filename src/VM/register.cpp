/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "register.h"
#include "common.h"
#include "magic_enum.hpp"

#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>

using namespace via::VM;

uint8_t RegisterAllocator::get_size(RegisterType type)
{
    switch (type)
    {
    case RegisterType::R:
        return sizeof(viaValue);
    case RegisterType::AR:
        return sizeof(viaValue);
    case RegisterType::RR:
        return sizeof(viaValue);
    case RegisterType::IR:
        return sizeof(viaTableKey);
    case RegisterType::ER:
        return sizeof(viaNumber);
    case RegisterType::SR:
        return sizeof(viaTable);
    default:
        return 0;
    }
}

void *RegisterAllocator::alloc(RegisterType type, uint8_t count)
{
    return std::malloc(get_size(type) * count);
}

void RegisterAllocator::prealloc()
{
    this->gpr = reinterpret_cast<viaValue *>(alloc(RegisterType::R, 128));
    this->ar = reinterpret_cast<viaValue *>(alloc(RegisterType::AR, 16));
    this->rr = reinterpret_cast<viaValue *>(alloc(RegisterType::RR, 16));
    this->ir = reinterpret_cast<viaTableKey *>(alloc(RegisterType::IR, 1));
    this->er = reinterpret_cast<viaNumber *>(alloc(RegisterType::ER, 32));
    this->sr = reinterpret_cast<viaTable *>(alloc(RegisterType::SR, 1));
}

void RegisterAllocator::flush(RegisterType r)
{
    switch (r)
    {
    case RegisterType::R:
        std::free(this->gpr);
        this->gpr = reinterpret_cast<viaValue *>(alloc(RegisterType::R, 128));
        break;

    case RegisterType::AR:
        std::free(this->ar);
        this->ar = reinterpret_cast<viaValue *>(alloc(RegisterType::AR, 16));
        break;

    case RegisterType::RR:
        std::free(this->rr);
        this->rr = reinterpret_cast<viaValue *>(alloc(RegisterType::RR, 16));
        break;

    case RegisterType::IR:
        std::free(this->ir);
        this->ir = reinterpret_cast<viaTableKey *>(alloc(RegisterType::IR, 1));
        break;

    case RegisterType::ER:
        std::free(this->er);
        this->er = reinterpret_cast<viaNumber *>(alloc(RegisterType::ER, 1));
        break;

    case RegisterType::SR:
        std::free(this->sr);
        this->sr = reinterpret_cast<viaTable *>(alloc(RegisterType::SR, 1));
        break;

    default:
        break;
    }
}

void RegisterAllocator::print(RegisterType rt)
{
    std::cout << "---------- viaRegister Map ----------\n";

    switch (rt)
    {
    case RegisterType::R:
    {
        auto rv = std::vector(gpr, gpr + 128);
        int i = 0;

        for (const auto &r : rv)
        {
            std::cout << std::format("|{}| T={}\n", i++, magic_enum::enum_name(r.type));
        }

        break;
    }

    default:
        break;
    }

    std::cout << "----------------------------------\n";
}