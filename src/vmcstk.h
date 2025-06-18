// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CALL_STACK_H
#define VIA_CALL_STACK_H

#include "common.h"
#include "vmval.h"
#include "vminstr.h"
#include "vmfunc.h"

namespace via {

inline constexpr size_t CALLSTACK_MAX_FRAMES = 200;
inline constexpr size_t CALLFRAME_MAX_LOCALS = 200;

struct CallInfo {
  bool is_protected = false;
  Closure* closure = nullptr;
  Value* locals = nullptr;
  size_t locals_size = 0;
  Instruction* savedpc = nullptr;

  VIA_NOCOPY(CallInfo);
  VIA_IMPLMOVE(CallInfo);

  CallInfo();
  ~CallInfo();
};

struct CallStack {
  size_t frames_count = 0;
  CallInfo frames[CALLSTACK_MAX_FRAMES];
};

} // namespace via

/** @} */

#endif
