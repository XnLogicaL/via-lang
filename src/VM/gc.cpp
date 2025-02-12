/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "gc.h"
#include "types.h"

namespace via
{

GCState::GCState()
    : terminating(false)
    , collections(0)
    , size(0)
{
}

GCState::~GCState()
{
    terminating = true;

    for (const GCCleanupFunction &fn : callback_list)
        fn();
}

void gccollect(State *V)
{
    // TODO: Implement collection mechanism

    V->gc->size = 0;
    V->gc->collections++;
}

void gcaddcallback(State *V, GCCleanupFunction fn)
{
    V->gc->callback_list.push_back(fn);
}

} // namespace via