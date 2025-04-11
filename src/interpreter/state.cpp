//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================
#include "state.h"
#include "bytecode.h"
#include "api-aux.h"
#include "api-impl.h"

namespace via {

using namespace impl;

// Initializes and returns a new state object
IState::IState(GlobalState* glb, stack_registers_t& stk_registers, TransUnitContext& unit_ctx)
  : id(glb->threads++),
    glb(glb),
    err(new ErrorState()),
    stack_registers(stk_registers),
    unit_ctx(unit_ctx) {
  // Load initial bytecode into the Instruction buffer.
  this->load(*unit_ctx.bytecode);

  __register_allocate(this);
  __stack_allocate(this);
  __label_allocate(this, unit_ctx.internal.label_count);
  __label_load(this);
}

IState::~IState() {
  delete err;
  delete[] sibp;

  __register_deallocate(this);
  __stack_deallocate(this);
  __label_deallocate(this);
}

void IState::load(const BytecodeHolder& BytecodeHolder) {
  delete[] sibp;

  auto& pipeline = BytecodeHolder.get();

  if (pipeline.empty()) {
    ibp = sibp = pc = nullptr;
    return;
  }

  ibp = new Instruction[pipeline.size()]; // Allocate ibp (instruction base/begin pointer)
  sibp = ibp;
  pc = ibp; // Initialize pc (instruction pointer)

  size_t position = 0;
  for (const Bytecode& pair : pipeline) {
    ibp[position++] = pair.instruct;
  }
}

} // namespace via
