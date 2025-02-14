/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "state.h"
#include "api.h"
#include "bytecode.h"
#include "register.h"
#include "gc.h"

namespace via {

// Initializes and returns a new State object
State::State(GState *G, ProgramData &program)
    : id(G->threads++)
    , G(G)
    , ralloc(new RAState())
    , gc(new GCState())
    , sbp(new TValue[VIA_VM_STACK_SIZE])
    , err(new ErrorState())
{
    load(*program.bytecode);

    // Mimic a "main" function
    // This is necessary for setting up a global scope, and isn't meant to be a conventional function
    TFunction *main = new TFunction(this, VIA_MAIN_ID, this->ip, this->frame, {}, false, false);
    nativecall(this, main, 0);
}

void State::load(BytecodeHolder &bytecode)
{
    if (this->ihp) { // Clean up previous instruction pipeline
        delete[] this->ihp;
    }

    std::vector<Instruction> pipeline = bytecode.get();

    this->ihp = new Instruction[pipeline.size()]; // Allocate ihp (Instruction head pointer)
    this->ibp = this->ihp + pipeline.size();      // Initialize ibp (Instruction base pointer)
    this->ip = this->ihp;                         // Initialize ip (Instruction pointer)

    std::memcpy(ihp, pipeline.data(), pipeline.size());
}

GState::GState()
    : threads(0)
{
}

State::~State()
{
    // Clean up saved state, if there is one
    if (sstate) {
        // Invalidate shared resources to avoid double frees
        sstate->gc = nullptr;
        sstate->ralloc = nullptr;

        if (sstate->ihp == ihp)
            sstate->ihp = nullptr;

        if (sstate->sbp == sbp)
            sstate->sbp = nullptr;

        delete this->sstate;
    }


    if (gc)
        delete this->gc;

    if (ralloc)
        delete this->ralloc;

    if (ihp)
        delete[] this->ihp;

    if (sbp)
        delete[] this->sbp;
}

} // namespace via
