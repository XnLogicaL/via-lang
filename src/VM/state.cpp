/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "state.h"
#include "api.h"
#include "stack.h"
#include "register.h"
#include "gc.h"

namespace via
{

GState *stnewgstate()
{
    GState *G = new GState;

    G->threads = 0;
    G->stable = new StrTable();
    G->gtable = new GlbTable();
    G->ktable = new kTable();
    G->symtable = new SymTable();

    return G;
}

// Initializes and returns a new RTState object
RTState *stnewstate(GState *G, const std::vector<Instruction> &pipeline)
{
    RTState *V = new RTState;

    V->id = G->threads++;
    V->G = G;

    // Allocate ihp (Instruction head pointer)
    V->ihp = new Instruction[pipeline.size()];
    // Initialize ibp (Instruction base pointer)
    V->ibp = V->ihp + pipeline.size();
    // Initialize ip (Instruction pointer)
    V->ip = V->ihp;

    // Copy instructions into the instruction pipeline
    std::copy(pipeline.begin(), pipeline.end(), V->ip);

    V->frame = nullptr;
    V->stack = tsnewstate();
    // I know, the odd one out...
    V->labels = new LblMap();
    V->ralloc = rnewstate(V);
    V->gc = gcnewstate();

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

    // This is set to idle by default because the via thread manipulators (execute, pausethread, killthread)
    // are the only ones allowed to mutate this
    V->tstate = ThreadState::PAUSED;
    V->sstate = nullptr;

    V->heapptr = nullptr;

    // Mimic a "main" function
    // This is necessary for setting up a global scope, and isn't meant to be a conventional function
    TFunction *main = newfunc(V, VIA_MAIN_ID, nullptr, pipeline, false, false);
    nativecall(V, main, 0);

    // Initialize labels
    Instruction *ip = V->ip;
    for (Instruction instr : pipeline)
    {
        if (instr.op == OpCode::LABEL)
        {
            Operand ident = instr.operand1;
            (*V->labels)[LabelId(ident.val_identifier)] = ip;
        }
        ++ip;
    }

    return V;
}

void stcleanupgstate(GState *G)
{
    delete G->stable;
    delete G->gtable;
    delete G;
}

void stcleanupstate(RTState *V)
{
    stcleanupgstate(V->G);
    gccleanup(V->gc);
    tscleanupstate(V->stack);
    rcleanupstate(V->ralloc);

    // Clean up saved state, if there is one
    if (V->sstate)
        stcleanupstate(V->sstate);

    // Clean up heap values
    TValue *current_value = V->heapptr;
    while (current_value)
    {
        cleanupval(V, current_value);
        current_value = current_value->next;
    }

    // This automatically invalidates both ip and ibp
    // No need to clean them up seperately
    delete[] V->ihp;
    delete V->labels;
    delete V;
}

} // namespace via
