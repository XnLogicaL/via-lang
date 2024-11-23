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
    gc->freelist = {};
    gc->reflist = {};

    return gc;
}

template<typename Ptr>
void viaGC_add(viaGCState *S, Ptr *p)
{
    S->freelist.push_back(p);
    S->size += sizeof(Ptr);
}

void viaGC_collect(viaGCState *S)
{
    for (void *p : S->freelist)
        std::free(p);

    S->size = 0;
    S->collections++;
}

void viaGC_addref(viaGCState *S, std::shared_ptr<viaValue> val)
{
    if (S->terminating)
        return;

    S->reflist.push_back(val);
    S->size += sizeof(viaValue);
}

void viaGC_cleanup(viaGCState *S)
{
    S->terminating = true;
    S->reflist.clear();
    viaGC_collect(S);

    delete S;
}

} // namespace via