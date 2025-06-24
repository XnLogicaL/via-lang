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

#define VM_DISPATCH_OP(op) &&CASE_##op
#define VM_DISPATCH_TABLE()                                                                        \
  VM_DISPATCH_OP(VOP_NOP), VM_DISPATCH_OP(VOP_LBL), VM_DISPATCH_OP(VOP_EXIT),                      \
    VM_DISPATCH_OP(VOP_ADD), VM_DISPATCH_OP(VOP_IADD), VM_DISPATCH_OP(VOP_FADD),                   \
    VM_DISPATCH_OP(VOP_SUB), VM_DISPATCH_OP(VOP_ISUB), VM_DISPATCH_OP(VOP_FSUB),                   \
    VM_DISPATCH_OP(VOP_MUL), VM_DISPATCH_OP(VOP_IMUL), VM_DISPATCH_OP(VOP_FMUL),                   \
    VM_DISPATCH_OP(VOP_DIV), VM_DISPATCH_OP(VOP_IDIV), VM_DISPATCH_OP(VOP_FDIV),                   \
    VM_DISPATCH_OP(VOP_MOD), VM_DISPATCH_OP(VOP_IMOD), VM_DISPATCH_OP(VOP_FMOD),                   \
    VM_DISPATCH_OP(VOP_POW), VM_DISPATCH_OP(VOP_IPOW), VM_DISPATCH_OP(VOP_FPOW),                   \
    VM_DISPATCH_OP(VOP_NEG), VM_DISPATCH_OP(VOP_MOV), VM_DISPATCH_OP(VOP_LOADK),                   \
    VM_DISPATCH_OP(VOP_LOADNIL), VM_DISPATCH_OP(VOP_LOADI), VM_DISPATCH_OP(VOP_LOADF),             \
    VM_DISPATCH_OP(VOP_LOADBT), VM_DISPATCH_OP(VOP_LOADBF), VM_DISPATCH_OP(VOP_LOADARR),           \
    VM_DISPATCH_OP(VOP_LOADDICT), VM_DISPATCH_OP(VOP_CLOSURE), VM_DISPATCH_OP(VOP_PUSH),           \
    VM_DISPATCH_OP(VOP_PUSHK), VM_DISPATCH_OP(VOP_PUSHNIL), VM_DISPATCH_OP(VOP_PUSHI),             \
    VM_DISPATCH_OP(VOP_PUSHF), VM_DISPATCH_OP(VOP_PUSHBT), VM_DISPATCH_OP(VOP_PUSHBF),             \
    VM_DISPATCH_OP(VOP_DROP), VM_DISPATCH_OP(VOP_GETGLOBAL), VM_DISPATCH_OP(VOP_SETGLOBAL),        \
    VM_DISPATCH_OP(VOP_SETUPV), VM_DISPATCH_OP(VOP_GETUPV), VM_DISPATCH_OP(VOP_GETLOCAL),          \
    VM_DISPATCH_OP(VOP_SETLOCAL), VM_DISPATCH_OP(VOP_GETARG), VM_DISPATCH_OP(VOP_CAPTURE),         \
    VM_DISPATCH_OP(VOP_INC), VM_DISPATCH_OP(VOP_DEC), VM_DISPATCH_OP(VOP_EQ),                      \
    VM_DISPATCH_OP(VOP_DEQ), VM_DISPATCH_OP(VOP_NEQ), VM_DISPATCH_OP(VOP_AND),                     \
    VM_DISPATCH_OP(VOP_OR), VM_DISPATCH_OP(VOP_NOT), VM_DISPATCH_OP(VOP_LT),                       \
    VM_DISPATCH_OP(VOP_GT), VM_DISPATCH_OP(VOP_LTEQ), VM_DISPATCH_OP(VOP_GTEQ),                    \
    VM_DISPATCH_OP(VOP_JMP), VM_DISPATCH_OP(VOP_JMPIF), VM_DISPATCH_OP(VOP_JMPIFN),                \
    VM_DISPATCH_OP(VOP_JMPIFEQ), VM_DISPATCH_OP(VOP_JMPIFNEQ), VM_DISPATCH_OP(VOP_JMPIFLT),        \
    VM_DISPATCH_OP(VOP_JMPIFGT), VM_DISPATCH_OP(VOP_JMPIFLTEQ), VM_DISPATCH_OP(VOP_JMPIFGTEQ),     \
    VM_DISPATCH_OP(VOP_LJMP), VM_DISPATCH_OP(VOP_LJMPIF), VM_DISPATCH_OP(VOP_LJMPIFN),             \
    VM_DISPATCH_OP(VOP_LJMPIFEQ), VM_DISPATCH_OP(VOP_LJMPIFNEQ), VM_DISPATCH_OP(VOP_LJMPIFLT),     \
    VM_DISPATCH_OP(VOP_LJMPIFGT), VM_DISPATCH_OP(VOP_LJMPIFLTEQ), VM_DISPATCH_OP(VOP_LJMPIFGTEQ),  \
    VM_DISPATCH_OP(VOP_CALL), VM_DISPATCH_OP(VOP_PCALL), VM_DISPATCH_OP(VOP_RET),                  \
    VM_DISPATCH_OP(VOP_RETBT), VM_DISPATCH_OP(VOP_RETBF), VM_DISPATCH_OP(VOP_RETNIL),              \
    VM_DISPATCH_OP(VOP_GETARR), VM_DISPATCH_OP(VOP_SETARR), VM_DISPATCH_OP(VOP_NEXTARR),           \
    VM_DISPATCH_OP(VOP_LENARR), VM_DISPATCH_OP(VOP_GETDICT), VM_DISPATCH_OP(VOP_SETDICT),          \
    VM_DISPATCH_OP(VOP_NEXTDICT), VM_DISPATCH_OP(VOP_LENDICT), VM_DISPATCH_OP(VOP_CONSTR),         \
    VM_DISPATCH_OP(VOP_GETSTR), VM_DISPATCH_OP(VOP_SETSTR), VM_DISPATCH_OP(VOP_LENSTR),            \
    VM_DISPATCH_OP(VOP_ICAST), VM_DISPATCH_OP(VOP_FCAST), VM_DISPATCH_OP(VOP_STRCAST),             \
    VM_DISPATCH_OP(VOP_BCAST)

namespace via {

namespace vm {

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
  goto* dispatch_table[(uint16_t)S->pc->op];
#else
  switch (S->pc->op)
#endif
  {
    VM_CASE(VOP_NOP) {
      VM_NEXT();
    }

    VM_CASE(VOP_IADDII) {
      uint16_t ra = insn->a;
      uint16_t rb = insn->b;
      uint16_t rc = insn->c;

      Value* vb = get_register(S, rb);
      Value* vc = get_register(S, rc);
    }

#ifndef VM_USE_CGOTO
  default:
    break;
#endif
  }

exit:;
}

} // namespace vm

} // namespace via
