/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "gc.h"
#include "types.h"

namespace via
{

GCState *gcnewstate()
{
    GCState *gc = new GCState;

    gc->terminating = false;
    gc->collections = 0;
    gc->size = 0;
    gc->dellist = {};
    gc->reflist = {};

    return gc;
}

void gcadd(RTState *V, TValue *p)
{
    V->gc->dellist.push_back(p);
    V->gc->size += sizeof(TValue);
}

void gccollect(RTState *V)
{
    for (TValue *p : V->gc->dellist)
        delete p;

    V->gc->size = 0;
    V->gc->collections++;
}

void gcaddref(RTState *V, std::shared_ptr<TValue> val)
{
    if (V->gc->terminating)
        return;

    V->gc->reflist.push_back(val);
    V->gc->size += sizeof(TValue);
}

void gccleanup(GCState *S)
{
    S->terminating = true;
    S->reflist.clear();

    delete S;
}

} // namespace via