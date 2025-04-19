// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_CALL_STACK_H
#define VIA_HAS_HEADER_CALL_STACK_H

#include "common.h"
#include "tfunction.h"

namespace via {

inline constexpr size_t CALLSTACK_MAX_FRAMES = 200;
inline constexpr size_t CALLFRAME_MAX_LOCALS = 200;

struct CallFrame {
  Closure* closure;
  Value* locals = nullptr;
  size_t locals_size = 0;
  Instruction* savedpc;

  VIA_NOCOPY(CallFrame);
  VIA_IMPLMOVE(CallFrame);

  CallFrame();
  ~CallFrame();
};

struct CallStack {
  CallFrame frames[CALLSTACK_MAX_FRAMES];
  size_t frames_count = 0;
};

} // namespace via

#endif
