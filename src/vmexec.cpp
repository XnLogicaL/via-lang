// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "vmstate.h"
#include "vmapi.h"
#include "vmval.h"
#include <cmath>

#define VM_PROTECT()

// Macro for completing an execution cycle
#define VM_NEXT()                                                                                  \
  do {                                                                                             \
    if constexpr (SingleStep) {                                                                    \
      if constexpr (OverrideProgramCounter)                                                        \
        S->pc = insn;                                                                              \
      else                                                                                         \
        S->pc++;                                                                                   \
      goto exit;                                                                                   \
    }                                                                                              \
    S->pc++;                                                                                       \
    goto dispatch;                                                                                 \
  } while (0)

#define VM_CHECK_RETURN()                                                                          \
  if VIA_UNLIKELY (S->ci_top == S->ci_stk.data) {                                                  \
    goto exit;                                                                                     \
  }

// Stolen from Luau :)
// Whether to use a dispatch table for instruction loading.
// #define VM_USE_CGOTO VIA_COMPILER == C_GCC || VIA_COMPILER == C_CLANG

#if VM_USE_CGOTO
#define VM_CASE(op) CASE_##op:
#else
#define VM_CASE(op) case op:
#endif

#define VM_DISPATCH_OP(op)  &&CASE_##op
#define VM_DISPATCH_TABLE() VM_DISPATCH_OP(VOP_NOP)

namespace via {

template<const bool SingleStep = false, const bool OverrideProgramCounter = false>
void execute(State* S, Instruction* ovrd = NULL) {
#if VM_USE_CGOTO
  static constexpr void* dispatch_table[0xFF] = {VM_DISPATCH_TABLE()};
#endif

dispatch:
  const Instruction* insn = S->pc;

  if constexpr (SingleStep && OverrideProgramCounter && ovrd != NULL) {
    S->pc = ovrd;
  }

#if VM_USE_CGOTO
  goto* dispatch_table[(u16)S->pc->op];
#else
  switch (S->pc->op)
#endif
  {
    VM_CASE(VOP_NOP) {
      VM_NEXT();
    }



#ifndef VM_USE_CGOTO
  default:
    break;
#endif
  }

exit:;
}

} // namespace via
