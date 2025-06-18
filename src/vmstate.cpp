// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "vmstate.h"
#include "vmdict.h"

namespace via {

namespace vm {

// Initializes and returns a new state object
State::State()
  : globals(new Dict),
    lctx(lctx),
    stk(sizeof(Value) * 200),
    main(Value()),
    ci_stk(sizeof(CallInfo) * 200) {
  __call(this, main.u.clsr);
  __label_load(this);
}

State::~State() {
  delete gt;
}

} // namespace vm

} // namespace via
