// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "state.h"
#include "api-impl.h"

namespace via {

using namespace impl;

// Initializes and returns a new state object
State::State(StkRegFile& stk_registers, TransUnitContext& unit_ctx)
  : globals(new Dict),
    callstack(new CallStack),
    err(new ErrorState),
    main(Value(__create_main_function(unit_ctx))),
    stack_registers(stk_registers),
    unit_ctx(unit_ctx) {
  __register_allocate(this);
  __label_allocate(this, unit_ctx.label_count);
  __call(this, main.u.clsr);
  __label_load(this);
  __declare_core_lib(this);
}

State::~State() {
  delete globals;
  delete callstack;
  delete err;

  __register_deallocate(this);
  __label_deallocate(this);
}

} // namespace via
