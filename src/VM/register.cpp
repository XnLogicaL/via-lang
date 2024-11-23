/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "register.h"

namespace via
{

RegisterAllocator::RegisterAllocator()
{
    this->prealloc();
}

RegisterAllocator::~RegisterAllocator()
{
    std::free(gpr);
    std::free(ar);
    std::free(rr);
}

viaRegisterSize RegisterAllocator::get_size(viaRegisterType type)
{
    switch (type)
    {
    case viaRegisterType::R:
        return sizeof(viaValue);
    case viaRegisterType::AR:
        return sizeof(viaValue);
    case viaRegisterType::RR:
        return sizeof(viaValue);
    default:
        return 0;
    }
}

void *RegisterAllocator::alloc(viaRegisterType type, viaRegisterCount count)
{
    return std::malloc(get_size(type) * count);
}

void RegisterAllocator::prealloc()
{
    this->gpr = reinterpret_cast<viaValue *>(alloc(viaRegisterType::R, 128));
    this->ar = reinterpret_cast<viaValue *>(alloc(viaRegisterType::AR, 16));
    this->rr = reinterpret_cast<viaValue *>(alloc(viaRegisterType::RR, 16));
}

void RegisterAllocator::flush(viaRegisterType r)
{
    switch (r)
    {
    case viaRegisterType::R:
        // std::free(this->gpr);
        this->gpr = reinterpret_cast<viaValue *>(alloc(viaRegisterType::R, 128));
        break;

    case viaRegisterType::AR:
        // std::free(this->ar);
        this->ar = reinterpret_cast<viaValue *>(alloc(viaRegisterType::AR, 16));
        break;

    case viaRegisterType::RR:
        // std::free(this->rr);
        this->rr = reinterpret_cast<viaValue *>(alloc(viaRegisterType::RR, 16));
        break;

    default:
        break;
    }
}

void RegisterAllocator::print(viaRegisterType rt)
{
    std::cout << "---------- viaRegister Map ----------\n";

    switch (rt)
    {
    case viaRegisterType::R:
    {
        auto rv = std::vector(gpr, gpr + 128);
        int i = 0;

        for (const auto &r : rv)
            std::cout << std::format("|{}| T={}\n", i++, magic_enum::enum_name(r.type));

        break;
    }

    default:
        break;
    }

    std::cout << "----------------------------------\n";
}

} // namespace via
