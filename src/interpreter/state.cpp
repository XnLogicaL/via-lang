// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "state.h"
#include "bytecode.h"
#include "api-aux.h"
#include "api-impl.h"

namespace via {

using namespace impl;

// Initializes and returns a new state object
state::state(global_state* glb, trans_unit_context& unit_ctx)
  : id(glb->threads++),
    glb(glb),
    err(new error_state()),
    unit_ctx(unit_ctx) {
  load(*unit_ctx.bytecode);

  __register_allocate(this);
  __stack_allocate(this);
  __label_allocate(this, unit_ctx.internal.label_count);
  __label_load(this);
}

state::~state() {
  delete err;

  delete[] sibp;

  __register_deallocate(this);
  __stack_deallocate(this);
  __label_deallocate(this);
}

void state::load(const bytecode_holder& bytecode_holder) {
  delete[] sibp;

  auto& pipeline = bytecode_holder.get();

  if (pipeline.empty()) {
    ibp = sibp = siep = iep = pc = nullptr;
    return;
  }

  ibp = new instruction[pipeline.size()]; // Allocate ibp (instruction base/begin pointer)
  sibp = ibp;
  iep = ibp + pipeline.size(); // Initialize iep (instruction end pointer)
  siep = iep;
  pc = ibp; // Initialize pc (instruction pointer)

  size_t position = 0;
  for (const bytecode& pair : pipeline) {
    ibp[position++] = pair.instruction;
  }
}

std::string to_string(state* state) {
#define VOID_STAR(ptr) reinterpret_cast<void*>(ptr)

  std::ostringstream oss;

  oss << std::format("==== state@{} ====\n", VOID_STAR(state));
  oss << std::format("|id    | {}\n", state->id);
  oss << std::format("|glb     | <global_state@{}>\n", VOID_STAR(state->glb));
  oss << std::format("|pc    | {}\n", VOID_STAR(state->pc));
  oss << std::format("|ibp   | {}\n", VOID_STAR(state->ibp));
  oss << std::format("|iep   | {}\n", VOID_STAR(state->iep));
  oss << std::format("|reg   | {}\n", VOID_STAR(state->registers));
  oss << std::format("|sbp   | {}\n", VOID_STAR(state->sbp));
  oss << std::format("|sp    | {}\n", state->sp);
  oss << std::format("|frame | {}\n", VOID_STAR(state->frame));
  oss << std::format("|abort | {}\n", state->abort);
  oss << std::format("|err   | <ErrorState@{}>\n", VOID_STAR(state->err));
  oss << std::format("|tstate| {}\n", magic_enum::enum_name(state->tstate));

  oss << "==== state ====\n";

  return oss.str();
#undef VOID_STAR
}

} // namespace via
