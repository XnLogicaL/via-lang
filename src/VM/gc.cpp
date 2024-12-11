/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "gc.h"
#include "types.h"

namespace via
{

viaGCState *viaGC_newstate()
{
    viaGCState *gc = new viaGCState;

    gc->terminating = false;
    gc->collections = 0;
    gc->size = 0;
    gc->dellist = {};
    gc->reflist = {};

    return gc;
}

void viaGC_add(viaState *V, viaValue *p)
{
    V->gc->dellist.push_back(p);
    V->gc->size += sizeof(viaValue);
}

void viaGC_collect(viaState *V)
{
    for (viaValue *p : V->gc->dellist)
        viaT_cleanupval(V, p);

    V->gc->size = 0;
    V->gc->collections++;
}

void viaGC_addref(viaState *V, std::shared_ptr<viaValue> val)
{
    if (V->gc->terminating)
        return;

    V->gc->reflist.push_back(val);
    V->gc->size += sizeof(viaValue);
}

void viaGC_cleanup(viaGCState *S)
{
    S->terminating = true;
    S->reflist.clear();

    delete S;
}

} // namespace via