/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "types.h"
#include "gc.h"
#include "bytecode.h"

#include "magic_enum.hpp"

#ifndef __VIA_STACK_SIZE
    #define __VIA_STACK_SIZE 128
#endif

#ifndef __VIA_STACK_FRAME_SIZE
    #define __VIA_STACK_FRAME_SIZE 1024
#endif

namespace via::VM
{

class StackFrame {
    std::unordered_map<std::string_view, via_Value> locals;
    GarbageCollector &m_gc;

public:

    const Instruction* return_address;

    StackFrame(Instruction* ret, GarbageCollector& m_gc)
        : m_gc(m_gc)
        , return_address(ret)
    {
        locals.reserve(__VIA_STACK_FRAME_SIZE);
    }

    ~StackFrame() {
        for (const auto& it : locals)
        {
            m_gc.add(reinterpret_cast<const void*>(&it.second));
        }
    }

    inline void set_local(const char* key, const via_Value& value)
    {
        locals[std::string_view(key)] = value;
    }

    inline via_Value get_local(const char* key)
    {
        auto it = locals.find(std::string_view(key));

        if (it != locals.end())
        {
            auto val = it->second;
            return val;
        }

        return via_Value();
    }
};

template <typename T>
class Stack {
private:
    std::unique_ptr<T> stack[__VIA_STACK_SIZE];
    size_t stack_ptr = 0;

public:

    inline void push(const T& t)
    {
        if (stack_ptr == __VIA_STACK_SIZE)
            throw std::overflow_error("Stack overflow");

        stack[stack_ptr++] = std::make_unique<T>(t);
    }

    inline void pop()
    {
        if (is_empty())
            throw std::underflow_error("Stack underflow");

        stack[--stack_ptr].reset();
    }

    inline T& top()
    {
        if (is_empty())
            throw std::underflow_error("Stack underflow");

        return *stack[stack_ptr - 1];
    }

    inline bool is_empty() const
    {
        return stack_ptr == 0;
    }

    inline void flush()
    {
        for (size_t i = 0; i < stack_ptr; ++i)
            stack[i].reset();

        stack_ptr = 0;
    }
};

} // namespace via::VM
