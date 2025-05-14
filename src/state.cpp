// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "state.h"
#include "api-impl.h"

namespace via {

using namespace impl;

// Initializes and returns a new state object
State::State(StkRegFile& stk_registers, Context& lctx)
  : globals(new Dict),
    callstack(new CallStack),
    err(new ErrorState),
    main(Value(__create_main_function(lctx))),
    stack_registers(stk_registers),
    lctx(lctx) {
  __register_allocate(this);
  __label_allocate(this, lctx.label_count);
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
