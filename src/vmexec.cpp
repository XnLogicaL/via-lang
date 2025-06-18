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
        S->pc = savedpc;                                                                           \
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
#define VM_USE_CGOTO VIA_COMPILER == C_GCC || VIA_COMPILER == C_CLANG

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

static bool is_arith_opc(Opcode op) {
  return (uint16_t)op >= (uint16_t)VOP_ADD && (uint16_t)op <= (uint16_t)VOP_FPOW;
}

template<typename A, typename B = A>
static inline void run_arith(Opcode op, A& a, B b) {
  switch (op) {
  case VOP_ADD:
  case VOP_IADD:
  case VOP_FADD:
    a += b;
    break;
  case VOP_SUB:
  case VOP_ISUB:
  case VOP_FSUB:
    a -= b;
    break;
  case VOP_MUL:
  case VOP_IMUL:
  case VOP_FMUL:
    a *= b;
    break;
  case VOP_DIV:
  case VOP_IDIV:
  case VOP_FDIV:
    a /= b;
    break;
  case VOP_MOD:
  case VOP_IMOD:
  case VOP_FMOD:
    a = std::fmod(a, b);
    break;
  case VOP_POW:
  case VOP_IPOW:
  case VOP_FPOW:
    a = std::pow(a, b);
    break;
  default:
    break;
  }
}

static inline int arith(State*, Opcode op, Value* lhs, Value* rhs) {
  using enum ValueKind;

  if (!is_arith_opc(op))
    return 1;

  if (lhs->kind == VLK_INT && rhs->kind == VLK_INT)
    run_arith(op, lhs->data->u.i, rhs->data->u.i);
  else {
    auto as_float = [](const Value& v) -> float {
      return v.kind == VLK_INT ? static_cast<float>(v.data->u.i) : v.data->u.f;
    };

    float a = as_float(*lhs);
    float b = as_float(*rhs);

    run_arith(op, a, b);

    lhs->data->u.f = a;
    lhs->kind = VLK_FLOAT;
  }

  return 0;
}

static inline void iarith(State*, Opcode op, Value* lhs, int i) {
  if VIA_LIKELY (lhs->kind == VLK_INT)
    run_arith(op, lhs->data->u.i, i);
  else if (lhs->kind == VLK_FLOAT)
    run_arith(op, lhs->data->u.f, i);
}

static inline void farith(State*, Opcode op, Value* lhs, float f) {
  if VIA_LIKELY (lhs->kind == VLK_INT)
    run_arith(op, lhs->data->u.i, f);
  else if (lhs->kind == VLK_FLOAT)
    run_arith(op, lhs->data->u.f, f);
}

template<const bool SingleStep = false, const bool OverrideProgramCounter = false>
void execute(State* S, Instruction* insn = NULL) {
#if VM_USE_CGOTO
  static constexpr void* dispatch_table[0xFF] = {VM_DISPATCH_TABLE()};
#endif

dispatch:
  const Instruction* savedpc = S->pc;

  if constexpr (SingleStep && OverrideProgramCounter && insn != NULL) {
    S->pc = insn;
  }

#if VM_USE_CGOTO
  goto* dispatch_table[(uint16_t)S->pc->op];
#else
  switch (S->pc->op)
#endif
  {
    // Handle special/opcodes
    VM_CASE(VOP_NOP)
    VM_CASE(VOP_GETDICT)
    VM_CASE(VOP_SETDICT)
    VM_CASE(VOP_LENDICT)
    VM_CASE(VOP_NEXTDICT)
    VM_CASE(VOP_CAPTURE)
    VM_CASE(VOP_LBL) {
      VM_NEXT();
    }

    VM_CASE(VOP_ADD)
    VM_CASE(VOP_SUB)
    VM_CASE(VOP_MUL)
    VM_CASE(VOP_DIV)
    VM_CASE(VOP_MOD)
    VM_CASE(VOP_POW) {
      uint16_t ra = S->pc->a;
      uint16_t rb = S->pc->b;

      Value* lhs = get_register(S, ra);
      Value* rhs = get_register(S, rb);

      arith(S, savedpc->op, lhs, rhs);
      VM_NEXT();
    }

    VM_CASE(VOP_IADD)
    VM_CASE(VOP_ISUB)
    VM_CASE(VOP_IMUL)
    VM_CASE(VOP_IDIV)
    VM_CASE(VOP_IMOD)
    VM_CASE(VOP_IPOW) {
      uint16_t ra = S->pc->a;
      uint16_t ib = S->pc->b;
      uint16_t ic = S->pc->c;

      int imm = ((uint32_t)ic << 16) | ib;
      Value* lhs = get_register(S, ra);

      iarith(S, savedpc->op, lhs, imm);
      VM_NEXT();
    }

    VM_CASE(VOP_FADD)
    VM_CASE(VOP_FSUB)
    VM_CASE(VOP_FMUL)
    VM_CASE(VOP_FDIV)
    VM_CASE(VOP_FMOD)
    VM_CASE(VOP_FPOW) {
      uint16_t ra = S->pc->a;
      uint16_t fb = S->pc->b;
      uint16_t fc = S->pc->c;

      float imm = ((uint32_t)fc << 16) | fb;
      Value* lhs = get_register(S, ra);

      farith(S, savedpc->op, lhs, imm);
      VM_NEXT();
    }

    VM_CASE(VOP_NEG) {
      uint16_t dst = S->pc->a;
      Value* val = get_register(S, dst);

      if (val->kind == VLK_INT)
        val->data->u.i *= -1;
      else if (val->kind == VLK_FLOAT)
        val->data->u.f *= -1;

      VM_NEXT();
    }

    VM_CASE(VOP_MOV) {
      uint16_t rdst = S->pc->a;
      uint16_t rsrc = S->pc->b;
      Value* src_val = get_register(S, rsrc);

      set_register(S, rdst, value_ref(S, src_val));
      VM_NEXT();
    }

    VM_CASE(VOP_LOADK) {
      uint16_t dst = S->pc->a;
      uint16_t idx = S->pc->b;

      const Value& kval = get_constant(S, idx);

      set_register(S, dst, value_ref(S, &kval));
      VM_NEXT();
    }

    VM_CASE(VOP_LOADNIL) {
      uint16_t dst = S->pc->a;

      set_register(S, dst, value_new(S));
      VM_NEXT();
    }

    VM_CASE(VOP_LOADI) {
      uint16_t dst = S->pc->a;
      int imm = ubit_2u16tou32(S->pc->b, S->pc->c);

      set_register(S, dst, value_new(S, imm));
      VM_NEXT();
    }

    VM_CASE(VOP_LOADF) {
      uint16_t dst = S->pc->a;
      float imm = ubit_2u16tof32(S->pc->b, S->pc->c);

      set_register(S, dst, value_new(S, imm));
      VM_NEXT();
    }

    VM_CASE(VOP_LOADBT) {
      uint16_t dst = S->pc->a;
      set_register(S, dst, value_new(S, true));
      VM_NEXT();
    }

    VM_CASE(VOP_LOADBF) {
      uint16_t dst = S->pc->a;
      set_register(S, dst, value_new(S, false));
      VM_NEXT();
    }

    VM_CASE(VOP_LOADARR) {
      uint16_t dst = S->pc->a;
      Value arr = value_new(S, new Array());

      set_register(S, dst, std::move(arr));
      VM_NEXT();
    }

    VM_CASE(VOP_LOADDICT) {
      uint16_t dst = S->pc->a;
      Value dict = value_new(S, new Dict());

      set_register(S, dst, std::move(dict));
      VM_NEXT();
    }

    VM_CASE(VOP_CLOSURE) {
      uint16_t dst = S->pc->a;
      uint16_t len = S->pc->b;
      uint16_t argc = S->pc->c;

      const InstructionData& data = pcdata(S, S->pc);

      size_t idlen = data.comment.size();
      char* buffer = (char*)S->ator.alloc_bytes(idlen + 1);
      std::strcpy(buffer, data.comment.c_str());

      Function f;
      f.id = (char*)buffer;
      f.code = ++S->pc;
      f.code_size = len;

      Closure* closure = new Closure();
      closure->u = {.fun = &f};

      set_register(S, dst, value_new(closure));

      // Do not increment program counter, as closure_init automatically positions it
      // to the correct instruction.
      if constexpr (SingleStep)
        goto exit;
      else
        goto dispatch;
    }

    VM_CASE(VOP_GETUPV) {
      uint16_t dst = S->pc->a;
      uint16_t upv_id = S->pc->b;
      UpValue* upv = closure_upv_get(current_callframe(S)->closure, upv_id);

      set_register(S, dst, upv->value->clone());
      VM_NEXT();
    }

    VM_CASE(VOP_SETUPV) {
      uint16_t src = S->pc->a;
      uint16_t upv_id = S->pc->b;
      Value* val = get_register(S, src);

      closure_upv_set(current_callframe(S)->closure, upv_id, *val);
      VM_NEXT();
    }

    VM_CASE(VOP_PUSH) {
      uint16_t src = S->pc->a;
      Value* val = get_register(S, src);

      push(S, std::move(*val));
      VM_NEXT();
    }

    VM_CASE(VOP_PUSHK) {
      uint16_t const_idx = S->pc->a;
      Value constant = get_constant(S, const_idx);

      push(S, std::move(constant));
      VM_NEXT();
    }

    VM_CASE(VOP_PUSHNIL) {
      push(S, Value());
      VM_NEXT();
    }

    VM_CASE(VOP_PUSHI) {
      int imm = ubit_2u16tou32(S->pc->a, S->pc->b);
      push(S, value_new(imm));
      VM_NEXT();
    }

    VM_CASE(VOP_PUSHF) {
      float imm = ubit_2u16tof32(S->pc->a, S->pc->b);
      push(S, value_new(imm));
      VM_NEXT();
    }

    VM_CASE(VOP_PUSHBT) {
      push(S, value_new(true));
      VM_NEXT();
    }

    VM_CASE(VOP_PUSHBF) {
      push(S, value_new(false));
      VM_NEXT();
    }

    VM_CASE(VOP_DROP) {
      pop(S);
      VM_NEXT();
    }

    VM_CASE(VOP_GETLOCAL) {
      uint16_t dst = S->pc->a;
      uint16_t off = S->pc->b;
      Value* val = get_local(S, off);

      set_register(S, dst, value_ref(S, val));
      VM_NEXT();
    }

    VM_CASE(VOP_SETLOCAL) {
      uint16_t src = S->pc->a;
      uint16_t off = S->pc->b;
      Value* val = get_register(S, src);

      set_local(S, off, std::move(*val));
      VM_NEXT();
    }

    VM_CASE(VOP_GETARG) {
      uint16_t dst = S->pc->a;
      uint16_t off = S->pc->b;
      Value* val = get_argument(S, off);

      set_register(S, dst, value_ref(S, val));
      VM_NEXT();
    }

    VM_CASE(VOP_GETGLOBAL) {
      uint16_t dst = S->pc->a;
      uint16_t key = S->pc->b;

      Value* key_obj = get_register(S, key);
      String* key_str = key_obj->data->u.str;
      Value* global = get_global(S, key_str->data.data);

      set_register(S, dst, value_ref(S, global));
      VM_NEXT();
    }

    VM_CASE(VOP_SETGLOBAL) {
      uint16_t src = S->pc->a;
      uint16_t key = S->pc->b;

      Value* key_obj = get_register(S, key);
      String* key_str = key_obj->data->u.str;
      Value* global = get_register(S, src);

      set_global(S, key_str->data.data, std::move(*global));
      VM_NEXT();
    }

    VM_CASE(VOP_EQ) {
      uint16_t ra = S->pc->a;
      uint16_t rb = S->pc->b;
      uint16_t rc = S->pc->c;

      if VIA_UNLIKELY (rb == rc) {
        set_register(S, ra, value_new(true));
        VM_NEXT();
      }

      Value* lhs_val = get_register(S, rb);
      Value* rhs_val = get_register(S, rc);

      if VIA_UNLIKELY (lhs_val == rhs_val) {
        set_register(S, ra, value_new(true));
        VM_NEXT();
      }

      bool result = value_cmp(S, lhs_val, rhs_val);
      set_register(S, ra, value_new(S, result));
      VM_NEXT();
    }

    VM_CASE(VOP_DEQ) {
      uint16_t dst = S->pc->a;
      uint16_t lhs = S->pc->b;
      uint16_t rhs = S->pc->c;

      if VIA_UNLIKELY (lhs == rhs) {
        set_register(S, dst, Value(true));
        VM_NEXT();
      }

      Value* lhs_val = get_register(S, lhs);
      Value* rhs_val = get_register(S, rhs);

      if VIA_UNLIKELY (lhs_val == rhs_val) {
        set_register(S, dst, Value(true));
        VM_NEXT();
      }

      bool result = compare_deep(*lhs_val, *rhs_val);
      set_register(S, dst, Value(result));

      VM_NEXT();
    }

    VM_CASE(VOP_NEQ) {
      uint16_t dst = S->pc->a;
      uint16_t lhs = S->pc->b;
      uint16_t rhs = S->pc->c;

      if VIA_LIKELY (lhs != rhs) {
        set_register(S, dst, Value(true));
        VM_NEXT();
      }

      Value* lhs_val = get_register(S, lhs);
      Value* rhs_val = get_register(S, rhs);

      if VIA_LIKELY (lhs_val != rhs_val) {
        set_register(S, dst, Value(true));
        VM_NEXT();
      }

      bool result = compare(*lhs_val, *rhs_val);
      set_register(S, dst, Value(result));

      VM_NEXT();
    }

    VM_CASE(VOP_AND) {
      uint16_t dst = S->pc->a;
      uint16_t lhs = S->pc->b;
      uint16_t rhs = S->pc->c;

      Value* lhs_val = get_register(S, lhs);
      Value* rhs_val = get_register(S, rhs);
      bool cond = to_cxx_bool(*lhs_val) && to_cxx_bool(*rhs_val);

      set_register(S, dst, Value(cond));
      VM_NEXT();
    }

    VM_CASE(VOP_OR) {
      uint16_t dst = S->pc->a;
      uint16_t lhs = S->pc->b;
      uint16_t rhs = S->pc->c;

      Value* lhs_val = get_register(S, lhs);
      Value* rhs_val = get_register(S, rhs);
      bool cond = to_cxx_bool(*lhs_val) || to_cxx_bool(*rhs_val);

      set_register(S, dst, Value(cond));
      VM_NEXT();
    }

    VM_CASE(VOP_NOT) {
      uint16_t dst = S->pc->a;
      uint16_t lhs = S->pc->b;

      Value* lhs_val = get_register(S, lhs);
      bool cond = !to_cxx_bool(*lhs_val);

      set_register(S, dst, Value(cond));
      VM_NEXT();
    }

    VM_CASE(VOP_LT) {
      uint16_t dst = S->pc->a;
      uint16_t lhs = S->pc->b;
      uint16_t rhs = S->pc->c;

      Value* lhs_val = get_register(S, lhs);
      Value* rhs_val = get_register(S, rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          set_register(S, dst, Value(lhs_val->u.i < rhs_val->u.i));
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          set_register(S, dst, Value(static_cast<float>(lhs_val->u.i) < rhs_val->u.f));
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          set_register(S, dst, Value(lhs_val->u.f < static_cast<float>(rhs_val->u.i)));
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          set_register(S, dst, Value(lhs_val->u.f < rhs_val->u.f));
        }
      }

      VM_NEXT();
    }

    VM_CASE(VOP_GT) {
      uint16_t dst = S->pc->a;
      uint16_t lhs = S->pc->b;
      uint16_t rhs = S->pc->c;

      Value* lhs_val = get_register(S, lhs);
      Value* rhs_val = get_register(S, rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          set_register(S, dst, Value(lhs_val->u.i > rhs_val->u.i));
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          set_register(S, dst, Value(static_cast<float>(lhs_val->u.i) > rhs_val->u.f));
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          set_register(S, dst, Value(lhs_val->u.f > static_cast<float>(rhs_val->u.i)));
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          set_register(S, dst, Value(lhs_val->u.f > rhs_val->u.f));
        }
      }

      VM_NEXT();
    }

    VM_CASE(VOP_LTEQ) {
      uint16_t dst = S->pc->a;
      uint16_t lhs = S->pc->b;
      uint16_t rhs = S->pc->c;

      Value* lhs_val = get_register(S, lhs);
      Value* rhs_val = get_register(S, rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          set_register(S, dst, Value(lhs_val->u.i <= rhs_val->u.i));
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          set_register(S, dst, Value(static_cast<float>(lhs_val->u.i) <= rhs_val->u.f));
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          set_register(S, dst, Value(lhs_val->u.f <= static_cast<float>(rhs_val->u.i)));
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          set_register(S, dst, Value(lhs_val->u.f <= rhs_val->u.f));
        }
      }

      VM_NEXT();
    }

    VM_CASE(VOP_GTEQ) {
      uint16_t dst = S->pc->a;
      uint16_t lhs = S->pc->b;
      uint16_t rhs = S->pc->c;

      Value* lhs_val = get_register(S, lhs);
      Value* rhs_val = get_register(S, rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          set_register(S, dst, Value(lhs_val->u.i >= rhs_val->u.i));
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          set_register(S, dst, Value(static_cast<float>(lhs_val->u.i) >= rhs_val->u.f));
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          set_register(S, dst, Value(lhs_val->u.f >= static_cast<float>(rhs_val->u.i)));
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          set_register(S, dst, Value(lhs_val->u.f >= rhs_val->u.f));
        }
      }

      VM_NEXT();
    }

    VM_CASE(VOP_EXIT) {
      goto exit;
    }

    VM_CASE(VOP_JMP) {
      signed_operand_t offset = S->pc->a;
      S->pc += offset;
      goto dispatch;
    }

    VM_CASE(VOP_JMPIF) {
      uint16_t cond = S->pc->a;
      signed_operand_t offset = S->pc->b;

      Value* cond_val = get_register(S, cond);
      if (to_cxx_bool(*cond_val)) {
        S->pc += offset;
        goto dispatch;
      }

      VM_NEXT();
    }

    VM_CASE(VOP_JMPIFN) {
      uint16_t cond = S->pc->a;
      signed_operand_t offset = S->pc->b;

      Value* cond_val = get_register(S, cond);
      if (!to_cxx_bool(*cond_val)) {
        S->pc += offset;
        goto dispatch;
      }

      VM_NEXT();
    }

    VM_CASE(VOP_JMPIFEQ) {
      uint16_t cond_lhs = S->pc->a;
      uint16_t cond_rhs = S->pc->b;
      signed_operand_t offset = S->pc->c;

      if VIA_UNLIKELY (cond_lhs == cond_rhs) {
        S->pc += offset;
        goto dispatch;
      }
      else {
        Value* lhs_val = get_register(S, cond_lhs);
        Value* rhs_val = get_register(S, cond_rhs);

        if VIA_UNLIKELY (lhs_val == rhs_val || compare(*lhs_val, *rhs_val)) {
          S->pc += offset;
          goto dispatch;
        }
      }

      VM_NEXT();
    }

    VM_CASE(VOP_JMPIFNEQ) {
      uint16_t cond_lhs = S->pc->a;
      uint16_t cond_rhs = S->pc->b;
      signed_operand_t offset = S->pc->c;

      if VIA_LIKELY (cond_lhs != cond_rhs) {
        S->pc += offset;
        goto dispatch;
      }
      else {
        Value* lhs_val = get_register(S, cond_lhs);
        Value* rhs_val = get_register(S, cond_rhs);

        if VIA_LIKELY (lhs_val != rhs_val || !compare(*lhs_val, *rhs_val)) {
          S->pc += offset;
          goto dispatch;
        }
      }

      VM_NEXT();
    }

    VM_CASE(VOP_JMPIFLT) {
      uint16_t cond_lhs = S->pc->a;
      uint16_t cond_rhs = S->pc->b;
      signed_operand_t offset = S->pc->c;

      Value* lhs_val = get_register(S, cond_lhs);
      Value* rhs_val = get_register(S, cond_rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.i < rhs_val->u.i) {
            S->pc += offset;
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (static_cast<float>(lhs_val->u.i) < rhs_val->u.f) {
            S->pc += offset;
            goto dispatch;
          }
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.f < static_cast<float>(rhs_val->u.i)) {
            S->pc += offset;
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (lhs_val->u.f < rhs_val->u.f) {
            S->pc += offset;
            goto dispatch;
          }
        }
      }

      VM_NEXT();
    }

    VM_CASE(VOP_JMPIFGT) {
      uint16_t cond_lhs = S->pc->a;
      uint16_t cond_rhs = S->pc->b;
      signed_operand_t offset = S->pc->c;

      Value* lhs_val = get_register(S, cond_lhs);
      Value* rhs_val = get_register(S, cond_rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.i > rhs_val->u.i) {
            S->pc += offset;
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (static_cast<float>(lhs_val->u.i) > rhs_val->u.f) {
            S->pc += offset;
            goto dispatch;
          }
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.f > static_cast<float>(rhs_val->u.i)) {
            S->pc += offset;
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (lhs_val->u.f > rhs_val->u.f) {
            S->pc += offset;
            goto dispatch;
          }
        }
      }

      VM_NEXT();
    }

    VM_CASE(VOP_JMPIFLTEQ) {
      uint16_t cond_lhs = S->pc->a;
      uint16_t cond_rhs = S->pc->b;
      signed_operand_t offset = S->pc->c;

      Value* lhs_val = get_register(S, cond_lhs);
      Value* rhs_val = get_register(S, cond_rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.i <= rhs_val->u.i) {
            S->pc += offset;
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (static_cast<float>(lhs_val->u.i) <= rhs_val->u.f) {
            S->pc += offset;
            goto dispatch;
          }
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.f <= static_cast<float>(rhs_val->u.i)) {
            S->pc += offset;
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (lhs_val->u.f <= rhs_val->u.f) {
            S->pc += offset;
            goto dispatch;
          }
        }
      }

      VM_NEXT();
    }

    VM_CASE(VOP_JMPIFGTEQ) {
      uint16_t cond_lhs = S->pc->a;
      uint16_t cond_rhs = S->pc->b;
      signed_operand_t offset = S->pc->c;

      Value* lhs_val = get_register(S, cond_lhs);
      Value* rhs_val = get_register(S, cond_rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.i >= rhs_val->u.i) {
            S->pc += offset;
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (static_cast<float>(lhs_val->u.i) >= rhs_val->u.f) {
            S->pc += offset;
            goto dispatch;
          }
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.f >= static_cast<float>(rhs_val->u.i)) {
            S->pc += offset;
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (lhs_val->u.f >= rhs_val->u.f) {
            S->pc += offset;
            goto dispatch;
          }
        }
      }

      VM_NEXT();
    }

    VM_CASE(VOP_LJMP) {
      uint16_t label = S->pc->a;

      S->pc = label_get(S, label);

      goto dispatch;
    }

    VM_CASE(VOP_LJMPIF) {
      uint16_t cond = S->pc->a;
      uint16_t label = S->pc->b;

      Value* cond_val = get_register(S, cond);
      if (to_cxx_bool(*cond_val)) {
        S->pc = label_get(S, label);
        goto dispatch;
      }

      VM_NEXT();
    }

    VM_CASE(VOP_LJMPIFN) {
      uint16_t cond = S->pc->a;
      uint16_t label = S->pc->b;

      Value* cond_val = get_register(S, cond);
      if (!to_cxx_bool(*cond_val)) {
        S->pc = label_get(S, label);
        goto dispatch;
      }

      VM_NEXT();
    }

    VM_CASE(VOP_LJMPIFEQ) {
      uint16_t cond_lhs = S->pc->a;
      uint16_t cond_rhs = S->pc->b;
      uint16_t label = S->pc->c;

      if VIA_UNLIKELY (cond_lhs == cond_rhs) {
        S->pc = label_get(S, label);
        goto dispatch;
      }
      else {
        Value* lhs_val = get_register(S, cond_lhs);
        Value* rhs_val = get_register(S, cond_rhs);

        if VIA_UNLIKELY (lhs_val == rhs_val || compare(*lhs_val, *rhs_val)) {
          S->pc = label_get(S, label);
          goto dispatch;
        }
      }

      VM_NEXT();
    }

    VM_CASE(VOP_LJMPIFNEQ) {
      uint16_t cond_lhs = S->pc->a;
      uint16_t cond_rhs = S->pc->b;
      uint16_t label = S->pc->c;

      if VIA_LIKELY (cond_lhs != cond_rhs) {
        S->pc = label_get(S, label);
        goto dispatch;
      }
      else {
        Value* lhs_val = get_register(S, cond_lhs);
        Value* rhs_val = get_register(S, cond_rhs);

        if VIA_LIKELY (lhs_val != rhs_val || !compare(*lhs_val, *rhs_val)) {
          S->pc = label_get(S, label);
          goto dispatch;
        }
      }

      VM_NEXT();
    }

    VM_CASE(VOP_LJMPIFLT) {
      uint16_t cond_lhs = S->pc->a;
      uint16_t cond_rhs = S->pc->b;
      uint16_t label = S->pc->c;

      Value* lhs_val = get_register(S, cond_lhs);
      Value* rhs_val = get_register(S, cond_rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.i < rhs_val->u.i) {
            S->pc = label_get(S, label);
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (static_cast<float>(lhs_val->u.i) < rhs_val->u.f) {
            S->pc = label_get(S, label);
            goto dispatch;
          }
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.f < static_cast<float>(rhs_val->u.i)) {
            S->pc = label_get(S, label);
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (lhs_val->u.f < rhs_val->u.f) {
            S->pc = label_get(S, label);
            goto dispatch;
          }
        }
      }

      VM_NEXT();
    }

    VM_CASE(VOP_LJMPIFGT) {
      uint16_t cond_lhs = S->pc->a;
      uint16_t cond_rhs = S->pc->b;
      uint16_t label = S->pc->c;

      Value* lhs_val = get_register(S, cond_lhs);
      Value* rhs_val = get_register(S, cond_rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.i > rhs_val->u.i) {
            S->pc = label_get(S, label);
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (static_cast<float>(lhs_val->u.i) > rhs_val->u.f) {
            S->pc = label_get(S, label);
            goto dispatch;
          }
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.f > static_cast<float>(rhs_val->u.i)) {
            S->pc = label_get(S, label);
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (lhs_val->u.f > rhs_val->u.f) {
            S->pc = label_get(S, label);
            goto dispatch;
          }
        }
      }

      VM_NEXT();
    }

    VM_CASE(VOP_LJMPIFLTEQ) {
      uint16_t cond_lhs = S->pc->a;
      uint16_t cond_rhs = S->pc->b;
      uint16_t label = S->pc->c;

      Value* lhs_val = get_register(S, cond_lhs);
      Value* rhs_val = get_register(S, cond_rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.i <= rhs_val->u.i) {
            S->pc = label_get(S, label);
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (static_cast<float>(lhs_val->u.i) <= rhs_val->u.f) {
            S->pc = label_get(S, label);
            goto dispatch;
          }
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.f <= static_cast<float>(rhs_val->u.i)) {
            S->pc = label_get(S, label);
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (lhs_val->u.f <= rhs_val->u.f) {
            S->pc = label_get(S, label);
            goto dispatch;
          }
        }
      }

      VM_NEXT();
    }

    VM_CASE(VOP_LJMPIFGTEQ) {
      uint16_t cond_lhs = S->pc->a;
      uint16_t cond_rhs = S->pc->b;
      uint16_t label = S->pc->c;

      Value* lhs_val = get_register(S, cond_lhs);
      Value* rhs_val = get_register(S, cond_rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.i >= rhs_val->u.i) {
            S->pc = label_get(S, label);
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (static_cast<float>(lhs_val->u.i) >= rhs_val->u.f) {
            S->pc = label_get(S, label);
            goto dispatch;
          }
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.f >= static_cast<float>(rhs_val->u.i)) {
            S->pc = label_get(S, label);
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (lhs_val->u.f >= rhs_val->u.f) {
            S->pc = label_get(S, label);
            goto dispatch;
          }
        }
      }

      VM_NEXT();
    }

    VM_CASE(VOP_CALL) {
      uint16_t fn = S->pc->a;
      uint16_t ap = S->pc->b;
      uint16_t rr = S->pc->c;

      Value* fn_val = get_register(S, fn);

      S->args = ap;
      S->ret = rr;

      call(S, fn_val->u.clsr);

      if constexpr (SingleStep)
        goto exit;
      else
        goto dispatch;
    }

    VM_CASE(VOP_PCALL) {
      uint16_t fn = S->pc->a;
      uint16_t ap = S->pc->b;
      uint16_t rr = S->pc->c;

      Value* fn_val = get_register(S, fn);

      S->args = ap;
      S->ret = rr;

      pcall(S, fn_val->u.clsr);

      if constexpr (SingleStep)
        goto exit;
      else
        goto dispatch;
    }

    VM_CASE(VOP_RETNIL) {
      closure_close_upvalues(current_callframe(S)->closure);
      return (S, Value());

      VM_CHECK_RETURN();
      VM_NEXT();
    }

    VM_CASE(VOP_RETBT) {
      return (S, Value(true));

      VM_CHECK_RETURN();
      VM_NEXT();
    }

    VM_CASE(VOP_RETBF) {
      return (S, Value(false));

      VM_CHECK_RETURN();
      VM_NEXT();
    }

    VM_CASE(VOP_RET) {
      uint16_t src = S->pc->a;
      Value* val = get_register(S, src);

      return (S, std::move(*val));

      VM_CHECK_RETURN();
      VM_NEXT();
    }

    VM_CASE(VOP_GETARR) {
      uint16_t dst = S->pc->a;
      uint16_t tbl = S->pc->b;
      uint16_t key = S->pc->c;

      Value* value = get_register(S, tbl);
      Value* index = get_register(S, key);
      Value* result = array_get(value->u.arr, index->u.i);

      set_register(S, dst, result->clone());
      VM_NEXT();
    }

    VM_CASE(VOP_SETARR) {
      uint16_t src = S->pc->a;
      uint16_t tbl = S->pc->b;
      uint16_t key = S->pc->c;

      Value* array = get_register(S, tbl);
      Value* index = get_register(S, key);
      Value* value = get_register(S, src);

      array_set(array->u.arr, index->u.i, std::move(*value));
      VM_NEXT();
    }

    VM_CASE(VOP_NEXTARR) {
      static std::unordered_map<void*, uint16_t> next_table;

      uint16_t dst = S->pc->a;
      uint16_t valr = S->pc->b;

      Value* val = get_register(S, valr);
      void* ptr = to_pointer(*val);
      uint16_t key = 0;

      auto it = next_table.find(ptr);
      if (it != next_table.end()) {
        key = ++it->second;
      }
      else {
        next_table[ptr] = 0;
      }

      Value* field = array_get(val->u.arr, key);
      set_register(S, dst, field->clone());
      VM_NEXT();
    }

    VM_CASE(VOP_LENARR) {
      uint16_t dst = S->pc->a;
      uint16_t tbl = S->pc->b;

      Value* val = get_register(S, tbl);
      int size = array_size(val->u.arr);

      set_register(S, dst, Value(size));
      VM_NEXT();
    }

    VM_CASE(VOP_LENSTR) {
      uint16_t rdst = S->pc->a;
      uint16_t objr = S->pc->b;

      Value* val = get_register(S, objr);
      int len = val->u.str->data_size;

      set_register(S, rdst, Value(len));
      VM_NEXT();
    }

    VM_CASE(VOP_CONSTR) {
      uint16_t left = S->pc->a;
      uint16_t right = S->pc->b;

      Value* left_val = get_register(S, left);
      Value* right_val = get_register(S, right);

      struct String* left_str = left_val->u.str;
      struct String* right_str = right_val->u.str;

      size_t new_length = left_str->data_size + right_str->data_size;
      char* new_string = new char[new_length + 1];

      std::memcpy(new_string, left_str->data, left_str->data_size);
      std::memcpy(new_string + left_str->data_size, right_str->data, right_str->data_size);

      set_register(S, left, Value(new_string));

      delete[] new_string;

      VM_NEXT();
    }

    VM_CASE(VOP_GETSTR) {
      uint16_t dst = S->pc->a;
      uint16_t str = S->pc->b;
      uint16_t idx = S->pc->c;

      Value* str_val = get_register(S, str);
      struct String* tstr = str_val->u.str;
      char chr = tstr->data[idx];

      set_register(S, dst, Value(new struct String(&chr)));
      VM_NEXT();
    }

    VM_CASE(VOP_SETSTR) {
      uint16_t str = S->pc->a;
      uint16_t src = S->pc->b;
      uint16_t idx = S->pc->c;

      Value* str_val = get_register(S, str);
      struct String* tstr = str_val->u.str;

      char chr = static_cast<char>(src);
      char* str_cpy = ustrdup(tstr->data);
      str_cpy[idx] = chr;

      set_register(S, str, Value(str_cpy));
      delete[] str_cpy;
      VM_NEXT();
    }

    VM_CASE(VOP_ICAST) {
      uint16_t dst = S->pc->a;
      uint16_t src = S->pc->b;

      Value* target = get_register(S, src);
      Value result = to_int(S, *target);

      set_register(S, dst, std::move(result));
      VM_NEXT();
    }

    VM_CASE(VOP_FCAST) {
      uint16_t dst = S->pc->a;
      uint16_t src = S->pc->b;

      Value* target = get_register(S, src);
      Value result = to_float(S, *target);

      set_register(S, dst, std::move(result));
      VM_NEXT();
    }

    VM_CASE(VOP_STRCAST) {
      uint16_t dst = S->pc->a;
      uint16_t src = S->pc->b;

      Value* target = get_register(S, src);
      Value result = to_string(*target);

      set_register(S, dst, std::move(result));
      VM_NEXT();
    }

    VM_CASE(VOP_BCAST) {
      uint16_t dst = S->pc->a;
      uint16_t src = S->pc->b;

      Value* target = get_register(S, src);
      Value result = to_bool(*target);

      set_register(S, dst, std::move(result));
      VM_NEXT();
    }
  }

exit:;
}

} // namespace vm

} // namespace via
