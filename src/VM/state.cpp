// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "state.h"
#include "api.h"
#include "bytecode.h"
#include "gc.h"
#include "vmapi_aux.h"

namespace via {

using namespace impl;

// Initializes and returns a new State object
State::State(GState *G, ProgramData *program)
    : id(G->threads++)
    , G(G)
    , gc(new GarbageCollector())
    , sbp(new TValue[VIA_VM_STACK_SIZE / sizeof(TValue)])
    , registers(new TValue[VIA_REGISTER_COUNT])
    , err(new ErrorState())
    , program(program)
{
    load(*program->bytecode);

    TFunction *main = __create_main_function(this);
    __native_call(this, main, 0);
}

void State::load(BytecodeHolder &bytecode)
{
    if (this->ihp) { // Clean up previous instruction pipeline
        delete[] this->ihp;
        this->ihp = nullptr;
    }

    const std::vector<Bytecode> &pipeline = bytecode.get();

    if (pipeline.empty()) {
        this->ihp = this->ibp = this->ip = nullptr;
        return;
    }

    this->ihp = new Instruction[pipeline.size()]; // Allocate ihp (Instruction head pointer)
    this->ibp = this->ihp + pipeline.size();      // Initialize ibp (Instruction base pointer)
    this->ip  = this->ihp;                        // Initialize ip (Instruction pointer)

    U64 position = 0;
    for (const Bytecode &pair : pipeline) {
        const Instruction &instruction = pair.instruction;
        this->ihp[position++]          = instruction;
    }
}

State::~State()
{
    if (this->sstate) {
        // Invalidate shared resources to avoid double frees
        sstate->gc = nullptr;

        if (sstate->ihp == ihp) {
            sstate->ihp = nullptr;
        }

        if (sstate->sbp == sbp) {
            sstate->sbp = nullptr;
        }

        delete this->sstate;
    }


    if (this->gc) {
        delete this->gc;
    }

    if (this->registers) {
        delete[] this->registers;
    }

    if (this->ihp) {
        delete[] this->ihp;
    }

    if (this->sbp) {
        delete[] this->sbp;
    }
}

std::string to_string(State *state)
{
#define TO_VOID_STAR(ptr) reinterpret_cast<void *>(ptr)

    std::ostringstream oss;

    oss << std::format("==== state@{} ====\n", TO_VOID_STAR(state));
    oss << std::format("|id    | {}\n", state->id);
    oss << std::format("|G     | <GState@{}>\n", TO_VOID_STAR(state->G));
    oss << std::format("|ip    | {}\n", TO_VOID_STAR(state->ip));
    oss << std::format("|ihp   | {}\n", TO_VOID_STAR(state->ihp));
    oss << std::format("|ibp   | {}\n", TO_VOID_STAR(state->ibp));
    oss << std::format("|reg   | {}\n", TO_VOID_STAR(state->registers));
    oss << std::format("|gc    | {}\n", TO_VOID_STAR(state->gc));
    oss << std::format("|sbp   | {}\n", TO_VOID_STAR(state->sbp));
    oss << std::format("|sp    | {}\n", state->sp);
    oss << std::format("|ssp   | {}\n", state->ssp);
    oss << std::format("|frame | {}\n", TO_VOID_STAR(state->frame));
    oss << std::format("|argc  | {}\n", state->argc);
    oss << std::format("|abort | {}\n", state->abort);
    oss << std::format("|err   | <ErrorState@{}>\n", TO_VOID_STAR(state->err));
    oss << std::format("|tstate| {}\n", magic_enum::enum_name(state->tstate));
    oss << std::format("|sstate| <State@{}>\n", TO_VOID_STAR(state->sstate));

    oss << "==== state ====\n";

    return oss.str();
#undef TO_VOID_STAR
}

} // namespace via
