#include "common.h"
#include "register.h"
#include "magic_enum.hpp"
#include <cstdlib>
#include <cstring>

using namespace via::VM;
using RType = Register::RType;

uint8_t RegisterAllocator::get_size(const RType& type)
{
    switch (type)
    {
    case RType::R:      return sizeof(via_Value);
    case RType::AR:     return sizeof(via_Value);
    case RType::RR:     return sizeof(via_Value);
    case RType::IR:     return sizeof(via_TableKey);
    case RType::AUX:    return 1;
    case RType::SELFR:  return sizeof(via_Table);
    default:            return 0;
    }
}

void* RegisterAllocator::alloc(const Register::RType& type, const uint8_t& count)
{
    return std::malloc(get_size(type) * count);
}

void RegisterAllocator::prealloc()
{
    this->gpr = reinterpret_cast<via_Value*>(alloc(RType::R, 128));
    this->ar = reinterpret_cast<via_Value*>(alloc(RType::AR, 16));
    this->rr = reinterpret_cast<via_Value*>(alloc(RType::RR, 16));
    this->ir = reinterpret_cast<via_TableKey*>(alloc(RType::IR, 1));
    this->auxr = reinterpret_cast<std::byte*>(alloc(RType::AUX, 32));
    this->selfr = reinterpret_cast<via_Table*>(alloc(RType::SELFR, 1));
}

void RegisterAllocator::flush(const RType& r)
{
    switch (r)
    {
    case RType::R:
        std::free(this->gpr);
        this->gpr = reinterpret_cast<via_Value*>(alloc(RType::R, 128));
        break;

    case RType::AR:
        std::free(this->ar);
        this->ar = reinterpret_cast<via_Value*>(alloc(RType::AR, 16));
        break;

    case RType::RR:
        std::free(this->rr);
        this->rr = reinterpret_cast<via_Value*>(alloc(RType::RR, 16));
        break;

    case RType::IR:
        std::free(this->ir);
        this->ir = reinterpret_cast<via_TableKey*>(alloc(RType::IR, 1));
        break;

    case RType::AUX:
        std::free(this->auxr);
        this->auxr = reinterpret_cast<std::byte*>(alloc(RType::AUX, 32));
        break;

    case RType::SELFR:
        std::free(this->selfr);
        this->selfr = reinterpret_cast<via_Table*>(alloc(RType::SELFR, 1));
        break;

    default:
        break;
    }
}

void RegisterAllocator::print(const Register::RType& rt)
{
    switch (rt)
    {
    case RType::R: {
        auto rv = std::vector(gpr, gpr + 128);
        int i = 0;

        for (const auto &r : rv)
        {
            std::cout << std::format("|{}| T={}\n", i++,
                magic_enum::enum_name(r.type));
        }

        break;
    }
    
    default:
        break;
    }
}