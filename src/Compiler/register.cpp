/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "register.h"

namespace via::Compilation
{

// Allocates an empty register for usage
GPRegister RegisterPool::allocate_register()
{
    // Check if there is an available register
    if (available_registers.empty())
        return std::numeric_limits<GPRegister>::max();

    // Save the register
    GPRegister reg = available_registers.top();
    // Pop the register
    available_registers.pop();
    return reg;
}

// Adds the register to the available list
void RegisterPool::free_register(GPRegister reg)
{
    available_registers.push(reg);
}

void RegisterPool::spill_register(GPRegister) {}
void RegisterPool::restore_register(GPRegister) {}

// Marks register <reg> as being "used", keeps track of time
void RegisterManager::use_register(GPRegister reg, viaTime_t time)
{
    register_life_ranges[reg].start_time = time;
}

// Marks register <reg> as unused, stops the timer
void RegisterManager::unuse_register(GPRegister reg, viaTime_t time)
{
    register_life_ranges[reg].end_time = time;
}

// Returns whether if the register <reg> can be freed relative to <current_time>
bool RegisterManager::can_free_register(GPRegister reg, viaTime_t current_time)
{
    return register_life_ranges[reg].end_time <= current_time;
}

// Allocates a variable and assigns it to a register
// Used for advanced compilation analysis
GPRegister RegisterAllocator::allocate_variable(viaTestVariable_t var)
{
    // Try to find a register for the variable
    size_t reg = register_pool.allocate_register();
    if (reg != SIZE_MAX)
    {
        register_manager.use_register(reg, current_time);
        variable_to_register[var] = reg;
    }

    return reg;
}

// Frees the variable <var> and unassigns the register tied to it
// Used for advanced compilation analysis
void RegisterAllocator::free_variable(viaTestVariable_t var)
{
    auto it = variable_to_register.find(var);
    if (it != variable_to_register.end())
    {
        size_t reg = it->second;
        register_manager.unuse_register(reg, current_time);
        register_pool.free_register(reg);
        variable_to_register.erase(it);
    }
}

void RegisterAllocator::increment_time()
{
    current_time++;
}

} // namespace via::Compilation