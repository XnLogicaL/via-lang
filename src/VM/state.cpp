/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "state.h"
#include "stack.h"
#include "register.h"
#include "gc.h"

namespace via
{

viaGlobalState *viaA_newgstate()
{
    viaGlobalState *G = new viaGlobalState;

    G->stable = new viaSTable();

    return G;
}

// Initializes and returns a new viaState object
viaState *viaA_newstate(const std::vector<viaInstruction> &pipeline)
{
    viaState *V = new viaState;

    V->id = __thread_id__++;
    V->G = viaA_newgstate();

    // Allocate ihp (Instruction head pointer)
    V->ihp = new viaInstruction[pipeline.size()];
    // Initialize ibp (Instruction base pointer)
    V->ibp = V->ihp + pipeline.size();
    // Initialize ip (Instruction pointer)
    V->ip = V->ihp;

    // Copy instructions into the instruction pipeline
    std::copy(pipeline.begin(), pipeline.end(), V->ip);

    V->stack = viaS_newstate<viaFunction *>();
    V->arguments = viaS_newstate<viaValue>();
    V->returns = viaS_newstate<viaValue>();
    // I know, the odd one out...
    V->labels = new viaLabels();
    V->ralloc = viaR_newstate(V);
    V->gc = viaGC_newstate();

    // Exit code
    V->exitc = 0;
    // Exit message
    V->exitm = "";

    // Abort on the next VM clock cycle
    V->abrt = false;
    // Skip the instruction on the next VM clock cycle
    V->skip = false;
    // Yield for a set amount (V->yieldfor) on the next VM cycle
    V->yield = false;
    // Restore the state on the next VM cycle, error if V->sstate is not valid
    V->restorestate = false;

    V->yieldfor = 0.0f;
    V->argc = 0;

    // This is set to idle by default because the via thread manipulators (via_execute, via_pausethread, via_killthread)
    // are the only ones allowed to mutate this
    V->tstate = viaThreadState::PAUSED;
    V->sstate = nullptr;

    // Mimic a "main" function
    // This is necessary for setting up a global scope, and isn't meant to be a conventional function
    viaFunction *mainf = new viaFunction{0, false, false, VIA_MAIN_ID, {}, {}, {}};
    viaS_push(V->stack, mainf);

    return V;
}

void viaA_cleanupgstate(viaGlobalState *G)
{
    delete G->stable;
    delete G;
}

void viaA_cleanupstate(viaState *V)
{
    viaA_cleanupgstate(V->G);
    viaGC_cleanup(V->gc);
    viaS_cleanupstate(V->stack);
    viaS_cleanupstate(V->arguments);
    viaS_cleanupstate(V->returns);
    viaR_cleanupstate(V->ralloc);

    // This automatically invalidates both ip and ibp
    // No need to clean them up seperately
    delete[] V->ihp;
    delete V->labels;
    delete V;
}

viaThreadId_t viaA_threadcount(viaState *)
{
    // Just return the latest thread id as it represents the number of threads
    return __thread_id__;
}
} // namespace via