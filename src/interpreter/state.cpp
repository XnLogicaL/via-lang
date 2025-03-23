// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "state.h"
#include "bytecode.h"
#include "gc.h"
#include "api-aux.h"
#include "api-impl.h"

#define DELETE_IF(symbol)                                                                          \
    if (symbol) {                                                                                  \
        delete symbol;                                                                             \
        symbol = nullptr;                                                                          \
    }

#define DELETE_ARR_IF(symbol)                                                                      \
    if (symbol) {                                                                                  \
        delete[] symbol;                                                                           \
        symbol = nullptr;                                                                          \
    }

VIA_NAMESPACE_BEGIN

using namespace impl;

// Initializes and returns a new State object
State::State(GState* G, ProgramData& program)
    : id(G->threads++),
      G(G),
      gc(new GarbageCollector()),
      sbp(new TValue[VIA_VM_STACK_SIZE / sizeof(TValue)]),
      registers(new TValue[VIA_REGISTER_COUNT]),
      err(new ErrorState()),
      program(program) {
    load(*program.bytecode);

    main = new TFunction(false, false, ip, nullptr);

    main->bytecode_len = static_cast<uint32_t>(iep - ibp);
    main->bytecode     = new Instruction[main->bytecode_len];

    std::memcpy(main->bytecode, ibp, main->bytecode_len);

    __label_load(this, program.label_count);
    __native_call(this, main, 0);
}

State::~State() {
    if (sstate) {
        // Invalidate shared resources to avoid double frees
        sstate->gc = nullptr;

        if (sstate->ibp == ibp) {
            sstate->ibp = nullptr;
        }

        if (sstate->sbp == sbp) {
            sstate->sbp = nullptr;
        }

        delete sstate;
    }

    DELETE_IF(gc);
    DELETE_IF(err);
    DELETE_IF(main);

    DELETE_ARR_IF(registers);
    DELETE_ARR_IF(ibp);
    DELETE_ARR_IF(sbp);

    __label_deallocate(this);
}

void State::load(BytecodeHolder& bytecode) {
    DELETE_ARR_IF(ibp);

    auto& pipeline = bytecode.get();

    if (pipeline.empty()) {
        ibp = iep = ip = nullptr;
        return;
    }

    ibp = new Instruction[pipeline.size()]; // Allocate ibp (Instruction base/begin pointer)
    iep = ibp + pipeline.size();            // Initialize iep (Instruction end pointer)
    ip  = ibp;                              // Initialize ip (Instruction pointer)

    uint64_t position = 0;
    for (const Bytecode& pair : pipeline) {
        const Instruction& instruction = pair.instruction;
        ibp[position++]                = instruction;
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

VIA_NAMESPACE_END
