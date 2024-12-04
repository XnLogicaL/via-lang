/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"

// Stack depth limit
#ifndef VIA_STACK_SIZE
#define VIA_STACK_SIZE 128
#endif

#ifndef VIA_SFRAME_SIZE
#define VIA_SFRAME_SIZE 2048
#endif

namespace via
{

template<typename T>
struct viaStackState
{
    T *sp;
    T *sbp;
    size_t size;

    // For iteration support
    inline auto end()
    {
        return sbp;
    };

    inline auto begin()
    {
        return sp;
    };
};

template<typename T>
inline void viaS_push(viaStackState<T> *S, T val)
{
    S->sp++;
    *S->sp = val;
}

template<typename T>
inline void viaS_pop(viaStackState<T> *S)
{
    S->sp--;
}

template<typename T>
inline T viaS_top(viaStackState<T> *S)
{
    return *S->sp;
}

template<typename T>
inline void viaS_flush(viaStackState<T> *S)
{
    S->sp = S->sbp;
}

template<typename T>
viaStackState<T> *viaS_newstate();

template<typename T>
void viaS_deletestate(viaStackState<T> *);

} // namespace via
