// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "state.h"
#include "bytecode.h"
#include "api-aux.h"
#include "api-impl.h"

VIA_NAMESPACE_BEGIN

using namespace impl;

// Initializes and returns a new State object
State::State(GState* G, ProgramData& program)
    : id(G->threads++),
      G(G),
      registers(new TValue[VIA_REGISTER_COUNT]),
      err(new ErrorState()),
      program(program) {
    load(*program.bytecode);

    __stack_allocate(this);
    __label_allocate(this, program.label_count);
    __label_load(this);
}

State::~State() {
    delete err;

    delete[] registers;
    delete[] sibp;

    __stack_deallocate(this);
    __label_deallocate(this);
}

void State::load(BytecodeHolder& bytecode) {
    delete[] sibp;

    auto& pipeline = bytecode.get();

    if (pipeline.empty()) {
        ibp = sibp = siep = iep = ip = nullptr;
        return;
    }

    ibp  = new Instruction[pipeline.size()]; // Allocate ibp (Instruction base/begin pointer)
    sibp = ibp;
    iep  = ibp + pipeline.size(); // Initialize iep (Instruction end pointer)
    siep = iep;
    ip   = ibp; // Initialize ip (Instruction pointer)

    size_t position = 0;
    for (const Bytecode& pair : pipeline) {
        ibp[position++] = pair.instruction;
    }
}

std::string to_string(State* state) {
#define TO_VOID_STAR(ptr) reinterpret_cast<void*>(ptr)

    std::ostringstream oss;

    oss << std::format("==== state@{} ====\n", TO_VOID_STAR(state));
    oss << std::format("|id    | {}\n", state->id);
    oss << std::format("|G     | <GState@{}>\n", TO_VOID_STAR(state->G));
    oss << std::format("|ip    | {}\n", TO_VOID_STAR(state->ip));
    oss << std::format("|ibp   | {}\n", TO_VOID_STAR(state->ibp));
    oss << std::format("|iep   | {}\n", TO_VOID_STAR(state->iep));
    oss << std::format("|reg   | {}\n", TO_VOID_STAR(state->registers));
    oss << std::format("|sbp   | {}\n", TO_VOID_STAR(state->sbp));
    oss << std::format("|sp    | {}\n", state->sp);
    oss << std::format("|frame | {}\n", TO_VOID_STAR(state->frame));
    oss << std::format("|abort | {}\n", state->abort);
    oss << std::format("|err   | <ErrorState@{}>\n", TO_VOID_STAR(state->err));
    oss << std::format("|tstate| {}\n", magic_enum::enum_name(state->tstate));

    oss << "==== state ====\n";

    return oss.str();
#undef TO_VOID_STAR
}

VIA_NAMESPACE_END
