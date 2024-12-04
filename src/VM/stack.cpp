/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "stack.h"

namespace via
{

template<typename T>
viaStackState<T> *viaS_newstate()
{
    // Allocate memory for the new state
    auto *state = new viaStackState<T>;
    state->size = VIA_STACK_SIZE;

    // Allocate memory for the stack based on the size
    T *stack = new T[state->size];

    // Initialize the stack pointers
    state->sp = stack - 1;
    state->sbp = stack;

    return state;
}

template<typename T>
void viaS_deletestate(viaStackState<T> *S)
{
    delete S->sbp;
    delete S;
}

} // namespace via