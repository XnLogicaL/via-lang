/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "stack.h"

namespace via
{

TStack::TStack()
    : sp(0)
{
    void *mem = std::malloc(VIA_STACK_SIZE);
    this->sbp = reinterpret_cast<TValue *>(mem);

    VIA_ASSERT_SILENT(mem, "TStack(): std::malloc failed");
}

TStack::~TStack()
{
    std::free(this->sbp);
}

// Push a value onto the stack
void tspush(TStack *VIA_RESTRICT S, StkVal val) noexcept
{
    S->sbp[S->sp++] = val;
}

// Pop a value from the stack
StkVal tspop(TStack *VIA_RESTRICT S) noexcept
{
    StkVal val = S->sbp[S->sp--]; // Copy the stack top value in order to avoid possible memory bugs
    return val;
}

// Get the top value from the stack
StkVal tstop(TStack *VIA_RESTRICT S) noexcept
{
    return S->sbp[S->sp];
}

// Reset the stack pointer to the base
void tsflush(TStack *VIA_RESTRICT S) noexcept
{
    S->sp = 0;
}

} // namespace via