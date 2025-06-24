// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "vmstate.h"

namespace via {

// Initializes and returns a new state object
State::State(const Header& H)
  : H(H),
    stk(sizeof(Value) * VIA_MAXSTACK),
    ci_stk(sizeof(CallInfo) * VIA_MAXCSTACK),
    ator(VIA_STATICMEM),
    heap(mi_heap_new()),
    top(stk.data),
    ci_top(ci_stk.data) {}

State::~State() {
  delete gt;
  mi_heap_delete(heap);
}

} // namespace via
