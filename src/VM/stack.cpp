/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "stack.h"
#include "shared.h"

namespace via
{

//* Stack
template<typename T>
void Stack<T>::push(const T &t)
{
    if (stack_ptr == VIA_STACK_SIZE)
        throw std::overflow_error("Stack overflow");

    stack[stack_ptr++] = std::make_unique<T>(t);
}

template<typename T>
void Stack<T>::pop()
{
    if (is_empty())
        throw std::underflow_error("Stack underflow");

    stack[--stack_ptr].reset();
}

template<typename T>
constexpr T &Stack<T>::top()
{
    if (is_empty())
        throw std::underflow_error("Stack underflow");

    return *stack[stack_ptr - 1];
}

template<typename T>
constexpr bool Stack<T>::is_empty() const
{
    return stack_ptr == 0;
}

template<typename T>
void Stack<T>::flush()
{
    for (size_t i = 0; i < stack_ptr; ++i)
        stack[i].reset();

    stack_ptr = 0;
}

//* Stackframe
void StackFrame::set_local(viaState *, viaLocalIdentifier id, viaValue value)
{
    locals[id] = value;
}

viaValue *StackFrame::get_local(viaState *V, viaLocalIdentifier id)
{
    auto it = locals.find(id);

    if (it != locals.end())
        return &it->second;

    return viaT_newvalue(V);
}

StackFrame::~StackFrame()
{
    for (const auto &it : locals)
        viaGC_add(gcs, &it.second);
}

StackFrame::StackFrame(viaInstruction *ret, viaGCState *gcs)
    : retaddr(ret)
    , locals({})
    , gcs(gcs)
{
    locals.reserve(VIA_STACK_FRAME_SIZE);
}

} // namespace via