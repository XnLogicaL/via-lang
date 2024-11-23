/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "types.h"
#include "shared.h"
#include "gc.h"

// Stack depth limit
#ifndef VIA_STACK_SIZE
#define VIA_STACK_SIZE 128
#endif

// Stackframe variable limit
#ifndef VIA_STACK_FRAME_SIZE
#define VIA_STACK_FRAME_SIZE 1024
#endif

namespace via
{

class StackFrame
{
public:
    const viaInstruction *retaddr;

    StackFrame(viaInstruction *, viaGCState *);
    ~StackFrame();

    void set_local(viaState *, viaLocalIdentifier, viaValue);
    viaValue *get_local(viaState *, viaLocalIdentifier);

private:
    viaHashMap<viaLocalIdentifier, viaValue> locals;
    viaGCState *gcs;
};

template<typename T>
class Stack
{
public:
    T &top();
    void push(const T &);
    void pop();
    bool is_empty() const;
    void flush();

private:
    std::unique_ptr<T> stack[VIA_STACK_SIZE];
    size_t stack_ptr = 0;
};

} // namespace via
