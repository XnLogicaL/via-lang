/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "state.h"
#include "api.h"
#include "bytecode.h"
#include "register.h"
#include "gc.h"

namespace via
{

// Initializes and returns a new State object
State::State(GState *G, ProgramData &program)
    : id(G->threads++)
    , G(G)
    , ihp(nullptr)
    , ralloc(new RAState())
    , gc(new GCState())
    , sbp(new TValue[VIA_VM_STACK_SIZE])
    , sp(0)
    , ssp(0)
    , frame(nullptr)
    , argc(0)
    , exitc(0)
    , exitm("")
    , abrt(false)
    , skip(false)
    , yield(false)
    , restorestate(false)
    , yieldfor(0.0f)
    , tstate(ThreadState::PAUSED)
    , sstate(nullptr)
{
    load(*program.bytecode);

    // Mimic a "main" function
    // This is necessary for setting up a global scope, and isn't meant to be a conventional function
    TFunction *main = new TFunction(this, VIA_MAIN_ID, this->ip, this->frame, {}, false, false);
    nativecall(this, main, 0);
}

void State::load(BytecodeHolder &bytecode)
{
    if (this->ihp)
    { // Clean up previous instruction pipeline
        delete[] this->ihp;
    }

    std::vector<Instruction> pipeline = bytecode.get();

    this->ihp = new Instruction[pipeline.size()]; // Allocate ihp (Instruction head pointer)
    this->ibp = this->ihp + pipeline.size();      // Initialize ibp (Instruction base pointer)
    this->ip = this->ihp;                         // Initialize ip (Instruction pointer)
}

GState::GState()
    : stable(new StrTable())
    , gtable(new GlbTable())
    , ktable(new kTable())
    , symtable(new SymTable())
    , threads(0)
{
}

GState::~GState()
{
    delete stable;
    delete gtable;
    delete ktable;
    delete symtable;
}

State::~State()
{
    delete this->G;
    delete this->gc;
    delete this->ralloc;

    // Clean up saved state, if there is one
    if (this->sstate)
        delete this->sstate;

    // This automatically invalidates both ip and ibp
    // No need to clean them up seperately
    delete[] this->ihp;
    delete[] this->sbp;
}

} // namespace via
