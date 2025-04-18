// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "call-stack.h"

namespace via {

CallFrame::CallFrame()
  : locals(new Value[CALLFRAME_MAX_LOCALS]) {}

CallFrame::CallFrame(CallFrame&& other)
  : closure(other.closure),
    locals(other.locals),
    savedpc(other.savedpc) {
  other.closure = nullptr;
  other.locals = nullptr;
  other.savedpc = nullptr;
}

CallFrame& CallFrame::operator=(CallFrame&& other) {
  if (this != &other) {
    this->closure = other.closure;
    this->locals = other.locals;
    this->savedpc = other.savedpc;

    other.closure = nullptr;
    other.locals = nullptr;
    other.savedpc = nullptr;
  }

  return *this;
}

CallFrame::~CallFrame() {
  delete closure;
  delete[] locals;
}

} // namespace via
