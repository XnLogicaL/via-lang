#ifndef VIA_REGISTER_H
#define VIA_REGISTER_H

#include "common.h"
#include "bytecode.h"
#include "types.h"

namespace via
{

namespace VM
{

class RegisterAllocator
{
    via_Value* gpr;
    via_Value* ar;
    via_Value* rr;
    via_TableKey* ir;
    via_Table* selfr;
    std::byte* auxr;

    uint8_t get_size(const Register::RType& type);
    void* alloc(const Register::RType& type, const uint8_t& count);
    void prealloc();

public:

    RegisterAllocator() {
        this->prealloc();
    }

    ~RegisterAllocator() {
        std::free(gpr);
        std::free(ar);
        std::free(rr);
        std::free(ir);
        std::free(auxr);
        std::free(selfr);
    }

    template <typename T>
    inline T* get(const Register& r)
    {
        const auto off = r.offset;

        switch (r.type)
        {
        case Register::RType::R:
            return reinterpret_cast<T*>(gpr + off);
        case Register::RType::AR:
            return reinterpret_cast<T*>(ar + off);
        case Register::RType::RR:
            return reinterpret_cast<T*>(rr + off);
        case Register::RType::IR:
            return reinterpret_cast<T*>(ir + off);
        case Register::RType::AUX:
            return reinterpret_cast<T*>(auxr + off);
        case Register::RType::SELFR:
            return reinterpret_cast<T*>(selfr + off);
        default:
            break;
        }

        return nullptr;
    }

    void flush(const Register::RType& r);
    void print(const Register::RType& rt);
};

} // namespace VM

} // namespace via

#endif // VIA_REGISTER_H
