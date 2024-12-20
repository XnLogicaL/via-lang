/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "instruction.h"

namespace via::Compilation
{

using viaTime_t = size_t;
using viaTestVariable_t = void *;

class RegisterPool
{
public:
    ~RegisterPool() = default;
    RegisterPool(size_t pool_size)
    {
        for (Register i = pool_size; i > 0; i--)
            available_registers.push(i);
    }

    Register allocate_register();
    void free_register(Register);
    void spill_register(Register);
    void restore_register(Register);

private:
    std::stack<Register> available_registers;
};

class RegisterManager
{
public:
    struct RegisterLifeRange
    {
        viaTime_t start_time;
        viaTime_t end_time;
    };

    RegisterManager() = default;
    ~RegisterManager() = default;

    void use_register(Register, viaTime_t);
    void unuse_register(Register, viaTime_t);
    bool can_free_register(Register, viaTime_t);

private:
    std::unordered_map<Register, RegisterLifeRange> register_life_ranges;
};

class RegisterAllocator
{
public:
    ~RegisterAllocator() = default;
    RegisterAllocator(RegisterPool &pool, RegisterManager &manager)
        : register_pool(pool)
        , register_manager(manager)
        , current_time(0)
    {
    }

    Register allocate_variable(viaTestVariable_t);
    void free_variable(viaTestVariable_t);
    void increment_time();

private:
    RegisterPool &register_pool;
    RegisterManager &register_manager;
    viaTime_t current_time;
    std::unordered_map<viaTestVariable_t, Register> variable_to_register;
};

} // namespace via::Compilation