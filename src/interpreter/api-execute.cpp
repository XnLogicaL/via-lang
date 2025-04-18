// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "bit-utility.h"
#include "file-io.h"
#include "api-impl.h"
#include "common.h"
#include "state.h"
#include "constant.h"
#include "tvalue.h"
#include "tarray.h"
#include "tstring.h"
#include "tdict.h"
#include "tfunction.h"
#include <cmath>

// Macro that throws a virtual error
#define VM_ERROR(message)                                                                          \
  do {                                                                                             \
    __set_error_state(this, message);                                                              \
    VM_NEXT();                                                                                     \
  } while (0)

// Macro that throws a fatal error
#define VM_FATAL(message)                                                                          \
  do {                                                                                             \
    std::cerr << "VM terminated with message: " << message << '\n';                                \
    std::abort();                                                                                  \
  } while (0)

// Macro for completing an execution cycle
#define VM_NEXT()                                                                                  \
  do {                                                                                             \
    ++pc;                                                                                          \
    goto dispatch;                                                                                 \
  } while (0)

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
  VM_DISPATCH_OP(NOP), VM_DISPATCH_OP(LBL), VM_DISPATCH_OP(EXIT), VM_DISPATCH_OP(ADD),             \
    VM_DISPATCH_OP(ADDI), VM_DISPATCH_OP(ADDF), VM_DISPATCH_OP(SUB), VM_DISPATCH_OP(SUBI),         \
    VM_DISPATCH_OP(SUBF), VM_DISPATCH_OP(MUL), VM_DISPATCH_OP(MULI), VM_DISPATCH_OP(MULF),         \
    VM_DISPATCH_OP(DIV), VM_DISPATCH_OP(DIVI), VM_DISPATCH_OP(DIVF), VM_DISPATCH_OP(MOD),          \
    VM_DISPATCH_OP(MODI), VM_DISPATCH_OP(MODF), VM_DISPATCH_OP(POW), VM_DISPATCH_OP(POWI),         \
    VM_DISPATCH_OP(POWF), VM_DISPATCH_OP(NEG), VM_DISPATCH_OP(MOV), VM_DISPATCH_OP(LOADK),         \
    VM_DISPATCH_OP(LOADNIL), VM_DISPATCH_OP(LOADI), VM_DISPATCH_OP(LOADF), VM_DISPATCH_OP(LOADBT), \
    VM_DISPATCH_OP(LOADBF), VM_DISPATCH_OP(LOADARR), VM_DISPATCH_OP(LOADDICT),                     \
    VM_DISPATCH_OP(CLOSURE), VM_DISPATCH_OP(PUSH), VM_DISPATCH_OP(PUSHK), VM_DISPATCH_OP(PUSHNIL), \
    VM_DISPATCH_OP(PUSHI), VM_DISPATCH_OP(PUSHF), VM_DISPATCH_OP(PUSHBT), VM_DISPATCH_OP(PUSHBF),  \
    VM_DISPATCH_OP(DROP), VM_DISPATCH_OP(GETLOCAL), VM_DISPATCH_OP(SETLOCAL),                      \
    VM_DISPATCH_OP(GETGLOBAL), VM_DISPATCH_OP(SETGLOBAL), VM_DISPATCH_OP(SETUPV),                  \
    VM_DISPATCH_OP(GETUPV), VM_DISPATCH_OP(INC), VM_DISPATCH_OP(DEC), VM_DISPATCH_OP(EQ),          \
    VM_DISPATCH_OP(NEQ), VM_DISPATCH_OP(AND), VM_DISPATCH_OP(OR), VM_DISPATCH_OP(NOT),             \
    VM_DISPATCH_OP(LT), VM_DISPATCH_OP(GT), VM_DISPATCH_OP(LTEQ), VM_DISPATCH_OP(GTEQ),            \
    VM_DISPATCH_OP(JMP), VM_DISPATCH_OP(JMPIF), VM_DISPATCH_OP(JMPIFN), VM_DISPATCH_OP(JMPIFEQ),   \
    VM_DISPATCH_OP(JMPIFNEQ), VM_DISPATCH_OP(JMPIFLT), VM_DISPATCH_OP(JMPIFGT),                    \
    VM_DISPATCH_OP(JMPIFLTEQ), VM_DISPATCH_OP(JMPIFGTEQ), VM_DISPATCH_OP(LJMP),                    \
    VM_DISPATCH_OP(LJMPIF), VM_DISPATCH_OP(LJMPIFN), VM_DISPATCH_OP(LJMPIFEQ),                     \
    VM_DISPATCH_OP(LJMPIFNEQ), VM_DISPATCH_OP(LJMPIFLT), VM_DISPATCH_OP(LJMPIFGT),                 \
    VM_DISPATCH_OP(LJMPIFLTEQ), VM_DISPATCH_OP(LJMPIFGTEQ), VM_DISPATCH_OP(CALL),                  \
    VM_DISPATCH_OP(RET), VM_DISPATCH_OP(RETNIL), VM_DISPATCH_OP(RETGET), VM_DISPATCH_OP(GETARR),   \
    VM_DISPATCH_OP(SETARR), VM_DISPATCH_OP(NEXTARR), VM_DISPATCH_OP(LENARR),                       \
    VM_DISPATCH_OP(GETDICT), VM_DISPATCH_OP(SETDICT), VM_DISPATCH_OP(LENDICT),                     \
    VM_DISPATCH_OP(NEXTDICT), VM_DISPATCH_OP(CONSTR), VM_DISPATCH_OP(GETSTR),                      \
    VM_DISPATCH_OP(SETSTR), VM_DISPATCH_OP(LENSTR), VM_DISPATCH_OP(ICAST), VM_DISPATCH_OP(FCAST),  \
    VM_DISPATCH_OP(STRCAST), VM_DISPATCH_OP(BCAST)

namespace via {

using enum Value::Tag;
using enum IOpCode;

// We use implementation functions only in this file.
using namespace impl;

// Starts VM execution cycle by altering it's state and "iterating" over
// the instruction pipeline.
void State::execute() {
#if VM_USE_CGOTO
  static const void* dispatch_table[0xFF] = {VM_DISPATCH_TABLE()};
#endif

dispatch:
  // Check for errors and attempt handling them.
  // The __handle_error function works by unwinding the stack until
  // either hitting a stack frame flagged as error handler, or, the root
  // stack frame, and the root stack frame cannot be an error handler
  // under any circumstances. Therefore the error will act as a fatal
  // error, being automatically thrown by __handle_error, along with a
  // callstack and debug information.
  if (__has_error(this) && !__handle_error(this)) {
    goto exit;
  }

#if VM_USE_CGOTO
  goto* dispatch_table[static_cast<uint8_t>(pc->op)];
#else
  switch (pc->op)
#endif
  {
    // Handle special/internal opcodes
    VM_CASE(NOP)
    VM_CASE(GETDICT)
    VM_CASE(SETDICT)
    VM_CASE(LENDICT)
    VM_CASE(NEXTDICT)
    VM_CASE(LBL) {
      VM_NEXT();
    }

    VM_CASE(ADD) {
      operand_t lhs = pc->operand0;
      operand_t rhs = pc->operand1;

      Value* lhs_val = __get_register(this, lhs);
      Value* rhs_val = __get_register(this, rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          lhs_val->u.i += rhs_val->u.i;
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          lhs_val->u.f = static_cast<float>(lhs_val->u.i) + rhs_val->u.f;
          lhs_val->type = Float;
        }
      }
      else if (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          lhs_val->u.f += static_cast<float>(rhs_val->u.i);
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          lhs_val->u.f += rhs_val->u.f;
        }
      }

      VM_NEXT();
    }
    VM_CASE(ADDI) {
      operand_t lhs = pc->operand0;
      operand_t int_high = pc->operand1;
      operand_t int_low = pc->operand2;

      Value* lhs_val = __get_register(this, lhs);
      int imm = reinterpret_u16_as_i32(int_high, int_low);

      if VIA_LIKELY (lhs_val->is_int()) {
        lhs_val->u.i += imm;
      }
      else if (lhs_val->is_float()) {
        lhs_val->u.f += imm;
      }

      VM_NEXT();
    }
    VM_CASE(ADDF) {
      operand_t lhs = pc->operand0;
      operand_t flt_high = pc->operand1;
      operand_t flt_low = pc->operand2;

      Value* lhs_val = __get_register(this, lhs);
      float imm = reinterpret_u16_as_f32(flt_high, flt_low);

      if VIA_LIKELY (lhs_val->is_int()) {
        lhs_val->u.i += imm;
      }
      else if (lhs_val->is_float()) {
        lhs_val->u.f += imm;
      }

      VM_NEXT();
    }

    VM_CASE(SUB) {
      operand_t lhs = pc->operand0;
      operand_t rhs = pc->operand1;

      Value* lhs_val = __get_register(this, lhs);
      Value* rhs_val = __get_register(this, rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          lhs_val->u.i -= rhs_val->u.i;
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          lhs_val->u.f = static_cast<float>(lhs_val->u.i) - rhs_val->u.f;
          lhs_val->type = Float;
        }
      }
      else if (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          lhs_val->u.f -= static_cast<float>(rhs_val->u.i);
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          lhs_val->u.f -= rhs_val->u.f;
        }
      }

      VM_NEXT();
    }
    VM_CASE(SUBI) {
      operand_t lhs = pc->operand0;
      operand_t int_high = pc->operand1;
      operand_t int_low = pc->operand2;

      Value* lhs_val = __get_register(this, lhs);
      int imm = reinterpret_u16_as_i32(int_high, int_low);

      if VIA_LIKELY (lhs_val->is_int()) {
        lhs_val->u.i -= imm;
      }
      else if (lhs_val->is_float()) {
        lhs_val->u.f -= imm;
      }

      VM_NEXT();
    }
    VM_CASE(SUBF) {
      operand_t lhs = pc->operand0;
      operand_t flt_high = pc->operand1;
      operand_t flt_low = pc->operand2;

      Value* lhs_val = __get_register(this, lhs);
      float imm = reinterpret_u16_as_f32(flt_high, flt_low);

      if VIA_LIKELY (lhs_val->is_int()) {
        lhs_val->u.i -= imm;
      }
      else if (lhs_val->is_float()) {
        lhs_val->u.f -= imm;
      }

      VM_NEXT();
    }

    VM_CASE(MUL) {
      operand_t lhs = pc->operand0;
      operand_t rhs = pc->operand1;

      Value* lhs_val = __get_register(this, lhs);
      Value* rhs_val = __get_register(this, rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          lhs_val->u.i *= rhs_val->u.i;
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          lhs_val->u.f = static_cast<float>(lhs_val->u.i) * rhs_val->u.f;
          lhs_val->type = Float;
        }
      }
      else if (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          lhs_val->u.f *= static_cast<float>(rhs_val->u.i);
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          lhs_val->u.f *= rhs_val->u.f;
        }
      }

      VM_NEXT();
    }
    VM_CASE(MULI) {
      operand_t lhs = pc->operand0;
      operand_t int_high = pc->operand1;
      operand_t int_low = pc->operand2;

      Value* lhs_val = __get_register(this, lhs);
      int imm = reinterpret_u16_as_i32(int_high, int_low);

      if VIA_LIKELY (lhs_val->is_int()) {
        lhs_val->u.i *= imm;
      }
      else if (lhs_val->is_float()) {
        lhs_val->u.f *= imm;
      }

      VM_NEXT();
    }
    VM_CASE(MULF) {
      operand_t lhs = pc->operand0;
      operand_t flt_high = pc->operand1;
      operand_t flt_low = pc->operand2;

      Value* lhs_val = __get_register(this, lhs);
      float imm = reinterpret_u16_as_f32(flt_high, flt_low);

      if VIA_LIKELY (lhs_val->is_int()) {
        lhs_val->u.i *= imm;
      }
      else if (lhs_val->is_float()) {
        lhs_val->u.f *= imm;
      }

      VM_NEXT();
    }

    VM_CASE(DIV) {
      operand_t lhs = pc->operand0;
      operand_t rhs = pc->operand1;

      Value* lhs_val = __get_register(this, lhs);
      Value* rhs_val = __get_register(this, rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (rhs_val->u.i == 0) {
            VM_ERROR("Division by zero");
          }

          lhs_val->u.i /= rhs_val->u.i;
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (rhs_val->u.f == 0.0f) {
            VM_ERROR("Division by zero");
          }

          lhs_val->u.f = static_cast<float>(lhs_val->u.i) / rhs_val->u.f;
          lhs_val->type = Float;
        }
      }
      else if (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (rhs_val->u.i == 0) {
            VM_ERROR("Division by zero");
          }

          lhs_val->u.f /= static_cast<float>(rhs_val->u.i);
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (rhs_val->u.f == 0.0f) {
            VM_ERROR("Division by zero");
          }

          lhs_val->u.f /= rhs_val->u.f;
        }
      }

      VM_NEXT();
    }
    VM_CASE(DIVI) {
      operand_t lhs = pc->operand0;
      operand_t int_high = pc->operand1;
      operand_t int_low = pc->operand2;

      Value* lhs_val = __get_register(this, lhs);
      int imm = reinterpret_u16_as_i32(int_high, int_low);
      if (imm == 0) {
        VM_ERROR("Division by zero");
      }

      if VIA_LIKELY (lhs_val->is_int()) {
        lhs_val->u.i /= imm;
      }
      else if (lhs_val->is_float()) {
        lhs_val->u.f /= imm;
      }

      VM_NEXT();
    }
    VM_CASE(DIVF) {
      operand_t lhs = pc->operand0;
      operand_t flt_high = pc->operand1;
      operand_t flt_low = pc->operand2;

      Value* lhs_val = __get_register(this, lhs);
      float imm = reinterpret_u16_as_f32(flt_high, flt_low);
      if (imm == 0.0f) {
        VM_ERROR("Division by zero");
      }

      if VIA_LIKELY (lhs_val->is_int()) {
        lhs_val->u.i /= imm;
      }
      else if (lhs_val->is_float()) {
        lhs_val->u.f /= imm;
      }

      VM_NEXT();
    }

    VM_CASE(POW) {
      operand_t lhs = pc->operand0;
      operand_t rhs = pc->operand1;

      Value* lhs_val = __get_register(this, lhs);
      Value* rhs_val = __get_register(this, rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          lhs_val->u.i = std::pow(lhs_val->u.i, rhs_val->u.i);
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          lhs_val->u.f = std::pow(static_cast<float>(lhs_val->u.i), rhs_val->u.f);
          lhs_val->type = Float;
        }
      }
      else if (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          lhs_val->u.f = std::pow(lhs_val->u.f, static_cast<float>(rhs_val->u.i));
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          lhs_val->u.f = std::pow(lhs_val->u.f, rhs_val->u.f);
        }
      }

      VM_NEXT();
    }
    VM_CASE(POWI) {
      operand_t lhs = pc->operand0;
      operand_t int_high = pc->operand1;
      operand_t int_low = pc->operand2;

      Value* lhs_val = __get_register(this, lhs);
      int imm = reinterpret_u16_as_i32(int_high, int_low);

      if VIA_LIKELY (lhs_val->is_int()) {
        lhs_val->u.i = std::pow(lhs_val->u.i, imm);
      }
      else if (lhs_val->is_float()) {
        lhs_val->u.f = std::pow(lhs_val->u.f, imm);
      }

      VM_NEXT();
    }
    VM_CASE(POWF) {
      operand_t lhs = pc->operand0;
      operand_t flt_high = pc->operand1;
      operand_t flt_low = pc->operand2;

      Value* lhs_val = __get_register(this, lhs);
      float imm = reinterpret_u16_as_f32(flt_high, flt_low);

      if VIA_LIKELY (lhs_val->is_int()) {
        lhs_val->u.i = std::pow(lhs_val->u.i, imm);
      }
      else if (lhs_val->is_float()) {
        lhs_val->u.f = std::pow(lhs_val->u.f, imm);
      }

      VM_NEXT();
    }

    VM_CASE(MOD) {
      operand_t lhs = pc->operand0;
      operand_t rhs = pc->operand1;

      Value* lhs_val = __get_register(this, lhs);
      Value* rhs_val = __get_register(this, rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          lhs_val->u.i %= rhs_val->u.i;
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          lhs_val->u.f = std::fmod(static_cast<float>(lhs_val->u.i), rhs_val->u.f);
          lhs_val->type = Float;
        }
      }
      else if (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          lhs_val->u.f = std::fmod(lhs_val->u.f, static_cast<float>(rhs_val->u.i));
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          lhs_val->u.f = std::fmod(lhs_val->u.f, rhs_val->u.f);
        }
      }

      VM_NEXT();
    }
    VM_CASE(MODI) {
      operand_t lhs = pc->operand0;
      operand_t int_high = pc->operand1;
      operand_t int_low = pc->operand2;

      Value* lhs_val = __get_register(this, lhs);
      int imm = reinterpret_u16_as_i32(int_high, int_low);



      if VIA_LIKELY (lhs_val->is_int()) {
        lhs_val->u.i = std::fmod(lhs_val->u.i, imm);
      }
      else if (lhs_val->is_float()) {
        lhs_val->u.f = std::fmod(lhs_val->u.f, imm);
      }

      VM_NEXT();
    }
    VM_CASE(MODF) {
      operand_t lhs = pc->operand0;
      operand_t flt_high = pc->operand1;
      operand_t flt_low = pc->operand2;

      Value* lhs_val = __get_register(this, lhs);
      float imm = reinterpret_u16_as_f32(flt_high, flt_low);

      if VIA_LIKELY (lhs_val->is_int()) {
        lhs_val->u.i = std::fmod(lhs_val->u.i, imm);
      }
      else if (lhs_val->is_float()) {
        lhs_val->u.f = std::fmod(lhs_val->u.f, imm);
      }

      VM_NEXT();
    }

    VM_CASE(NEG) {
      operand_t dst = pc->operand0;
      Value* val = __get_register(this, dst);
      Value::Tag type = val->type;

      if (type == Int) {
        val->u.i = -val->u.i;
      }
      else if (type == Float) {
        val->u.f = -val->u.f;
      }

      VM_NEXT();
    }

    VM_CASE(MOV) {
      operand_t rdst = pc->operand0;
      operand_t rsrc = pc->operand1;
      Value* src_val = __get_register(this, rsrc);

      __set_register(this, rdst, src_val->clone());
      VM_NEXT();
    }

    VM_CASE(INC) {
      operand_t rdst = pc->operand0;
      Value* dst_val = __get_register(this, rdst);

      if VIA_LIKELY (dst_val->is_int()) {
        dst_val->u.i++;
      }
      else if VIA_UNLIKELY (dst_val->is_float()) {
        dst_val->u.f++;
      }

      VM_NEXT();
    }

    VM_CASE(DEC) {
      operand_t rdst = pc->operand0;
      Value* dst_val = __get_register(this, rdst);

      if VIA_LIKELY (dst_val->is_int()) {
        dst_val->u.i--;
      }
      else if VIA_UNLIKELY (dst_val->is_float()) {
        dst_val->u.f--;
      }

      VM_NEXT();
    }

    VM_CASE(LOADK) {
      operand_t dst = pc->operand0;
      operand_t idx = pc->operand1;

      const Value& kval = __get_constant(this, idx);

      __set_register(this, dst, kval.clone());
      VM_NEXT();
    }

    VM_CASE(LOADNIL) {
      operand_t dst = pc->operand0;

      __set_register(this, dst, Value());
      VM_NEXT();
    }

    VM_CASE(LOADI) {
      operand_t dst = pc->operand0;
      int imm = reinterpret_u16_as_u32(pc->operand1, pc->operand2);

      __set_register(this, dst, Value(imm));
      VM_NEXT();
    }

    VM_CASE(LOADF) {
      operand_t dst = pc->operand0;
      float imm = reinterpret_u16_as_f32(pc->operand1, pc->operand2);

      __set_register(this, dst, Value(imm));
      VM_NEXT();
    }

    VM_CASE(LOADBT) {
      operand_t dst = pc->operand0;
      __set_register(this, dst, Value(true, true));
      VM_NEXT();
    }

    VM_CASE(LOADBF) {
      operand_t dst = pc->operand0;
      __set_register(this, dst, Value(false, true));
      VM_NEXT();
    }

    VM_CASE(LOADARR) {
      operand_t dst = pc->operand0;
      Value arr(new struct Array());

      __set_register(this, dst, std::move(arr));
      VM_NEXT();
    }

    VM_CASE(LOADDICT) {
      operand_t dst = pc->operand0;
      Value dict(new struct Dict());

      __set_register(this, dst, std::move(dict));
      VM_NEXT();
    }

    VM_CASE(CLOSURE) {
      operand_t dst = pc->operand0;
      operand_t len = pc->operand1;
      operand_t argc = pc->operand2;

      struct Function func;
      struct Closure* closure = new struct Closure();
      closure->callee = {
        .type = Callable::Tag::Function,
        .u = {.fn = func},
        .arity = argc,
      };

      __closure_bytecode_load(this, closure, len);
      __set_register(this, dst, Value(closure));

      // Do not increment program counter, as __closure_bytecode_load automatically positions it
      // to the correct instruction.
      goto dispatch;
    }

    VM_CASE(GETUPV) {
      operand_t dst = pc->operand0;
      operand_t upv_id = pc->operand1;
      UpValue* upv = __closure_upv_get(__current_callframe(this)->closure, upv_id);

      __set_register(this, dst, upv->value->clone());
      VM_NEXT();
    }

    VM_CASE(SETUPV) {
      operand_t src = pc->operand0;
      operand_t upv_id = pc->operand1;
      Value* val = __get_register(this, src);

      __closure_upv_set(__current_callframe(this)->closure, upv_id, *val);
      VM_NEXT();
    }

    VM_CASE(RETGET) {
      operand_t dst = pc->operand0;
      __set_register(this, dst, ret.clone());
      VM_NEXT();
    }

    VM_CASE(PUSH) {
      operand_t src = pc->operand0;
      Value* val = __get_register(this, src);

      __push(this, std::move(*val));
      VM_NEXT();
    }

    VM_CASE(PUSHK) {
      operand_t const_idx = pc->operand0;
      Value constant = __get_constant(this, const_idx);

      __push(this, constant.clone());
      VM_NEXT();
    }

    VM_CASE(PUSHNIL) {
      __push(this, Value());
      VM_NEXT();
    }

    VM_CASE(PUSHI) {
      int imm = reinterpret_u16_as_u32(pc->operand0, pc->operand1);
      __push(this, Value(imm));
      VM_NEXT();
    }

    VM_CASE(PUSHF) {
      float imm = reinterpret_u16_as_f32(pc->operand0, pc->operand1);
      __push(this, Value(imm));
      VM_NEXT();
    }

    VM_CASE(PUSHBT) {
      __push(this, Value(true, true));
      VM_NEXT();
    }

    VM_CASE(PUSHBF) {
      __push(this, Value(false, true));
      VM_NEXT();
    }

    VM_CASE(DROP) {
      __drop(this);
      VM_NEXT();
    }

    VM_CASE(GETLOCAL) {
      operand_t dst = pc->operand0;
      operand_t off = pc->operand1;
      Value* val = __get_local(this, off);

      __set_register(this, dst, val->clone());
      VM_NEXT();
    }

    VM_CASE(SETLOCAL) {
      operand_t src = pc->operand0;
      operand_t off = pc->operand1;
      Value* val = __get_register(this, src);

      __set_local(this, off, std::move(*val));
      VM_NEXT();
    }

    VM_CASE(GETGLOBAL) {
      operand_t dst = pc->operand0;
      operand_t key = pc->operand1;

      Value* key_obj = __get_register(this, key);
      struct String* key_str = key_obj->u.str;
      const Value& global = glb.gtable->get(key_str->data);

      __set_register(this, dst, global.clone());
      VM_NEXT();
    }

    VM_CASE(SETGLOBAL) {
      operand_t src = pc->operand0;
      operand_t key = pc->operand1;

      Value* key_obj = __get_register(this, key);
      struct String* key_str = key_obj->u.str;
      Value* global = __get_register(this, src);

      glb.gtable->set(key_str->data, std::move(*global));
      VM_NEXT();
    }

    VM_CASE(EQ) {
      operand_t dst = pc->operand0;
      operand_t lhs = pc->operand1;
      operand_t rhs = pc->operand2;

      if VIA_UNLIKELY (lhs == rhs) {
        __set_register(this, dst, Value(true, true));
        VM_NEXT();
      }

      Value* lhs_val = __get_register(this, lhs);
      Value* rhs_val = __get_register(this, rhs);

      if VIA_UNLIKELY (lhs_val == rhs_val) {
        __set_register(this, dst, Value(true, true));
        VM_NEXT();
      }

      bool result = __compare(*lhs_val, *rhs_val);
      __set_register(this, dst, Value(result, true));

      VM_NEXT();
    }

    VM_CASE(NEQ) {
      operand_t dst = pc->operand0;
      operand_t lhs = pc->operand1;
      operand_t rhs = pc->operand2;

      if VIA_LIKELY (lhs != rhs) {
        __set_register(this, dst, Value(true, true));
        VM_NEXT();
      }

      Value* lhs_val = __get_register(this, lhs);
      Value* rhs_val = __get_register(this, rhs);

      if VIA_LIKELY (lhs_val != rhs_val) {
        __set_register(this, dst, Value(true, true));
        VM_NEXT();
      }

      bool result = __compare(*lhs_val, *rhs_val);
      __set_register(this, dst, Value(result, true));

      VM_NEXT();
    }

    VM_CASE(AND) {
      operand_t dst = pc->operand0;
      operand_t lhs = pc->operand1;
      operand_t rhs = pc->operand2;

      Value* lhs_val = __get_register(this, lhs);
      Value* rhs_val = __get_register(this, rhs);
      bool cond = __to_cxx_bool(*lhs_val) && __to_cxx_bool(*rhs_val);

      __set_register(this, dst, Value(cond, true));
      VM_NEXT();
    }

    VM_CASE(OR) {
      operand_t dst = pc->operand0;
      operand_t lhs = pc->operand1;
      operand_t rhs = pc->operand2;

      Value* lhs_val = __get_register(this, lhs);
      Value* rhs_val = __get_register(this, rhs);
      bool cond = __to_cxx_bool(*lhs_val) || __to_cxx_bool(*rhs_val);

      __set_register(this, dst, Value(cond, true));
      VM_NEXT();
    }

    VM_CASE(NOT) {
      operand_t dst = pc->operand0;
      operand_t lhs = pc->operand1;

      Value* lhs_val = __get_register(this, lhs);
      bool cond = !__to_cxx_bool(*lhs_val);

      __set_register(this, dst, Value(cond, true));
      VM_NEXT();
    }

    VM_CASE(LT) {
      operand_t dst = pc->operand0;
      operand_t lhs = pc->operand1;
      operand_t rhs = pc->operand2;

      Value* lhs_val = __get_register(this, lhs);
      Value* rhs_val = __get_register(this, rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          __set_register(this, dst, Value(lhs_val->u.i < rhs_val->u.i, true));
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          __set_register(this, dst, Value(static_cast<float>(lhs_val->u.i) < rhs_val->u.f, true));
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          __set_register(this, dst, Value(lhs_val->u.f < static_cast<float>(rhs_val->u.i), true));
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          __set_register(this, dst, Value(lhs_val->u.f < rhs_val->u.f, true));
        }
      }

      VM_NEXT();
    }

    VM_CASE(GT) {
      operand_t dst = pc->operand0;
      operand_t lhs = pc->operand1;
      operand_t rhs = pc->operand2;

      Value* lhs_val = __get_register(this, lhs);
      Value* rhs_val = __get_register(this, rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          __set_register(this, dst, Value(lhs_val->u.i > rhs_val->u.i, true));
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          __set_register(this, dst, Value(static_cast<float>(lhs_val->u.i) > rhs_val->u.f, true));
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          __set_register(this, dst, Value(lhs_val->u.f > static_cast<float>(rhs_val->u.i), true));
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          __set_register(this, dst, Value(lhs_val->u.f > rhs_val->u.f, true));
        }
      }

      VM_NEXT();
    }

    VM_CASE(LTEQ) {
      operand_t dst = pc->operand0;
      operand_t lhs = pc->operand1;
      operand_t rhs = pc->operand2;

      Value* lhs_val = __get_register(this, lhs);
      Value* rhs_val = __get_register(this, rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          __set_register(this, dst, Value(lhs_val->u.i <= rhs_val->u.i, true));
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          __set_register(this, dst, Value(static_cast<float>(lhs_val->u.i) <= rhs_val->u.f, true));
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          __set_register(this, dst, Value(lhs_val->u.f <= static_cast<float>(rhs_val->u.i), true));
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          __set_register(this, dst, Value(lhs_val->u.f <= rhs_val->u.f, true));
        }
      }

      VM_NEXT();
    }

    VM_CASE(GTEQ) {
      operand_t dst = pc->operand0;
      operand_t lhs = pc->operand1;
      operand_t rhs = pc->operand2;

      Value* lhs_val = __get_register(this, lhs);
      Value* rhs_val = __get_register(this, rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          __set_register(this, dst, Value(lhs_val->u.i >= rhs_val->u.i, true));
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          __set_register(this, dst, Value(static_cast<float>(lhs_val->u.i) >= rhs_val->u.f, true));
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          __set_register(this, dst, Value(lhs_val->u.f >= static_cast<float>(rhs_val->u.i), true));
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          __set_register(this, dst, Value(lhs_val->u.f >= rhs_val->u.f, true));
        }
      }

      VM_NEXT();
    }

    VM_CASE(EXIT) {
      goto exit;
    }

    VM_CASE(JMP) {
      signed_operand_t offset = pc->operand0;
      pc += offset;
      goto dispatch;
    }

    VM_CASE(JMPIF) {
      operand_t cond = pc->operand0;
      signed_operand_t offset = pc->operand1;

      Value* cond_val = __get_register(this, cond);
      if (__to_cxx_bool(*cond_val)) {
        pc += offset;
        goto dispatch;
      }

      VM_NEXT();
    }

    VM_CASE(JMPIFN) {
      operand_t cond = pc->operand0;
      signed_operand_t offset = pc->operand1;

      Value* cond_val = __get_register(this, cond);
      if (!__to_cxx_bool(*cond_val)) {
        pc += offset;
        goto dispatch;
      }

      VM_NEXT();
    }

    VM_CASE(JMPIFEQ) {
      operand_t cond_lhs = pc->operand0;
      operand_t cond_rhs = pc->operand1;
      signed_operand_t offset = pc->operand2;

      if VIA_UNLIKELY (cond_lhs == cond_rhs) {
        pc += offset;
        goto dispatch;
      }
      else {
        Value* lhs_val = __get_register(this, cond_lhs);
        Value* rhs_val = __get_register(this, cond_rhs);

        if VIA_UNLIKELY (lhs_val == rhs_val || __compare(*lhs_val, *rhs_val)) {
          pc += offset;
          goto dispatch;
        }
      }

      VM_NEXT();
    }

    VM_CASE(JMPIFNEQ) {
      operand_t cond_lhs = pc->operand0;
      operand_t cond_rhs = pc->operand1;
      signed_operand_t offset = pc->operand2;

      if VIA_LIKELY (cond_lhs != cond_rhs) {
        pc += offset;
        goto dispatch;
      }
      else {
        Value* lhs_val = __get_register(this, cond_lhs);
        Value* rhs_val = __get_register(this, cond_rhs);

        if VIA_LIKELY (lhs_val != rhs_val || !__compare(*lhs_val, *rhs_val)) {
          pc += offset;
          goto dispatch;
        }
      }

      VM_NEXT();
    }

    VM_CASE(JMPIFLT) {
      operand_t cond_lhs = pc->operand0;
      operand_t cond_rhs = pc->operand1;
      signed_operand_t offset = pc->operand2;

      Value* lhs_val = __get_register(this, cond_lhs);
      Value* rhs_val = __get_register(this, cond_rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.i < rhs_val->u.i) {
            pc += offset;
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (static_cast<float>(lhs_val->u.i) < rhs_val->u.f) {
            pc += offset;
            goto dispatch;
          }
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.f < static_cast<float>(rhs_val->u.i)) {
            pc += offset;
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (lhs_val->u.f < rhs_val->u.f) {
            pc += offset;
            goto dispatch;
          }
        }
      }

      VM_NEXT();
    }

    VM_CASE(JMPIFGT) {
      operand_t cond_lhs = pc->operand0;
      operand_t cond_rhs = pc->operand1;
      signed_operand_t offset = pc->operand2;

      Value* lhs_val = __get_register(this, cond_lhs);
      Value* rhs_val = __get_register(this, cond_rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.i > rhs_val->u.i) {
            pc += offset;
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (static_cast<float>(lhs_val->u.i) > rhs_val->u.f) {
            pc += offset;
            goto dispatch;
          }
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.f > static_cast<float>(rhs_val->u.i)) {
            pc += offset;
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (lhs_val->u.f > rhs_val->u.f) {
            pc += offset;
            goto dispatch;
          }
        }
      }

      VM_NEXT();
    }

    VM_CASE(JMPIFLTEQ) {
      operand_t cond_lhs = pc->operand0;
      operand_t cond_rhs = pc->operand1;
      signed_operand_t offset = pc->operand2;

      Value* lhs_val = __get_register(this, cond_lhs);
      Value* rhs_val = __get_register(this, cond_rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.i <= rhs_val->u.i) {
            pc += offset;
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (static_cast<float>(lhs_val->u.i) <= rhs_val->u.f) {
            pc += offset;
            goto dispatch;
          }
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.f <= static_cast<float>(rhs_val->u.i)) {
            pc += offset;
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (lhs_val->u.f <= rhs_val->u.f) {
            pc += offset;
            goto dispatch;
          }
        }
      }

      VM_NEXT();
    }

    VM_CASE(JMPIFGTEQ) {
      operand_t cond_lhs = pc->operand0;
      operand_t cond_rhs = pc->operand1;
      signed_operand_t offset = pc->operand2;

      Value* lhs_val = __get_register(this, cond_lhs);
      Value* rhs_val = __get_register(this, cond_rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.i >= rhs_val->u.i) {
            pc += offset;
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (static_cast<float>(lhs_val->u.i) >= rhs_val->u.f) {
            pc += offset;
            goto dispatch;
          }
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.f >= static_cast<float>(rhs_val->u.i)) {
            pc += offset;
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (lhs_val->u.f >= rhs_val->u.f) {
            pc += offset;
            goto dispatch;
          }
        }
      }

      VM_NEXT();
    }

    VM_CASE(LJMP) {
      operand_t label = pc->operand0;

      pc = __label_get(this, label);

      goto dispatch;
    }

    VM_CASE(LJMPIF) {
      operand_t cond = pc->operand0;
      operand_t label = pc->operand1;

      Value* cond_val = __get_register(this, cond);
      if (__to_cxx_bool(*cond_val)) {
        pc = __label_get(this, label);
        goto dispatch;
      }

      VM_NEXT();
    }

    VM_CASE(LJMPIFN) {
      operand_t cond = pc->operand0;
      operand_t label = pc->operand1;

      Value* cond_val = __get_register(this, cond);
      if (!__to_cxx_bool(*cond_val)) {
        pc = __label_get(this, label);
        goto dispatch;
      }

      VM_NEXT();
    }

    VM_CASE(LJMPIFEQ) {
      operand_t cond_lhs = pc->operand0;
      operand_t cond_rhs = pc->operand1;
      operand_t label = pc->operand2;

      if VIA_UNLIKELY (cond_lhs == cond_rhs) {
        pc = __label_get(this, label);
        goto dispatch;
      }
      else {
        Value* lhs_val = __get_register(this, cond_lhs);
        Value* rhs_val = __get_register(this, cond_rhs);

        if VIA_UNLIKELY (lhs_val == rhs_val || __compare(*lhs_val, *rhs_val)) {
          pc = __label_get(this, label);
          goto dispatch;
        }
      }

      VM_NEXT();
    }

    VM_CASE(LJMPIFNEQ) {
      operand_t cond_lhs = pc->operand0;
      operand_t cond_rhs = pc->operand1;
      operand_t label = pc->operand2;

      if VIA_LIKELY (cond_lhs != cond_rhs) {
        pc = __label_get(this, label);
        goto dispatch;
      }
      else {
        Value* lhs_val = __get_register(this, cond_lhs);
        Value* rhs_val = __get_register(this, cond_rhs);

        if VIA_LIKELY (lhs_val != rhs_val || !__compare(*lhs_val, *rhs_val)) {
          pc = __label_get(this, label);
          goto dispatch;
        }
      }

      VM_NEXT();
    }

    VM_CASE(LJMPIFLT) {
      operand_t cond_lhs = pc->operand0;
      operand_t cond_rhs = pc->operand1;
      operand_t label = pc->operand2;

      Value* lhs_val = __get_register(this, cond_lhs);
      Value* rhs_val = __get_register(this, cond_rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.i < rhs_val->u.i) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (static_cast<float>(lhs_val->u.i) < rhs_val->u.f) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.f < static_cast<float>(rhs_val->u.i)) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (lhs_val->u.f < rhs_val->u.f) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
      }

      VM_NEXT();
    }

    VM_CASE(LJMPIFGT) {
      operand_t cond_lhs = pc->operand0;
      operand_t cond_rhs = pc->operand1;
      operand_t label = pc->operand2;

      Value* lhs_val = __get_register(this, cond_lhs);
      Value* rhs_val = __get_register(this, cond_rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.i > rhs_val->u.i) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (static_cast<float>(lhs_val->u.i) > rhs_val->u.f) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.f > static_cast<float>(rhs_val->u.i)) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (lhs_val->u.f > rhs_val->u.f) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
      }

      VM_NEXT();
    }

    VM_CASE(LJMPIFLTEQ) {
      operand_t cond_lhs = pc->operand0;
      operand_t cond_rhs = pc->operand1;
      operand_t label = pc->operand2;

      Value* lhs_val = __get_register(this, cond_lhs);
      Value* rhs_val = __get_register(this, cond_rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.i <= rhs_val->u.i) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (static_cast<float>(lhs_val->u.i) <= rhs_val->u.f) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.f <= static_cast<float>(rhs_val->u.i)) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (lhs_val->u.f <= rhs_val->u.f) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
      }

      VM_NEXT();
    }

    VM_CASE(LJMPIFGTEQ) {
      operand_t cond_lhs = pc->operand0;
      operand_t cond_rhs = pc->operand1;
      operand_t label = pc->operand2;

      Value* lhs_val = __get_register(this, cond_lhs);
      Value* rhs_val = __get_register(this, cond_rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.i >= rhs_val->u.i) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (static_cast<float>(lhs_val->u.i) >= rhs_val->u.f) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->u.f >= static_cast<float>(rhs_val->u.i)) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (lhs_val->u.f >= rhs_val->u.f) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
      }

      VM_NEXT();
    }

    VM_CASE(CALL) {
      operand_t fn = pc->operand0;
      Value* fn_val = __get_register(this, fn);

      __call(this, fn_val->u.clsr);
      goto dispatch;
    }

    VM_CASE(RETNIL) {
      __closure_close_upvalues(__current_callframe(this)->closure);
      __return(this, Value());
      VM_NEXT();
    }

    VM_CASE(RET) {
      operand_t src = pc->operand0;
      Value* val = __get_register(this, src);

      __return(this, std::move(*val));
      VM_NEXT();
    }

    VM_CASE(GETARR) {
      operand_t dst = pc->operand0;
      operand_t tbl = pc->operand1;
      operand_t key = pc->operand2;

      Value* value = __get_register(this, tbl);
      Value* index = __get_register(this, key);
      Value* result = __array_get(value->u.arr, index->u.i);

      __set_register(this, dst, result->clone());
      VM_NEXT();
    }

    VM_CASE(SETARR) {
      operand_t src = pc->operand0;
      operand_t tbl = pc->operand1;
      operand_t key = pc->operand2;

      Value* array = __get_register(this, tbl);
      Value* index = __get_register(this, key);
      Value* value = __get_register(this, src);

      __array_set(array->u.arr, index->u.i, std::move(*value));
      VM_NEXT();
    }

    VM_CASE(NEXTARR) {
      static std::unordered_map<void*, operand_t> next_table;

      operand_t dst = pc->operand0;
      operand_t valr = pc->operand1;

      Value* val = __get_register(this, valr);
      void* ptr = __to_pointer(*val);
      operand_t key = 0;

      auto it = next_table.find(ptr);
      if (it != next_table.end()) {
        key = ++it->second;
      }
      else {
        next_table[ptr] = 0;
      }

      Value* field = __array_get(val->u.arr, key);
      __set_register(this, dst, field->clone());
      VM_NEXT();
    }

    VM_CASE(LENARR) {
      operand_t dst = pc->operand0;
      operand_t tbl = pc->operand1;

      Value* val = __get_register(this, tbl);
      int size = __array_size(val->u.arr);

      __set_register(this, dst, Value(size));
      VM_NEXT();
    }

    VM_CASE(LENSTR) {
      operand_t rdst = pc->operand0;
      operand_t objr = pc->operand1;

      Value* val = __get_register(this, objr);
      int len = val->u.str->data_size;

      __set_register(this, rdst, Value(len));
      VM_NEXT();
    }

    VM_CASE(CONSTR) {
      operand_t left = pc->operand0;
      operand_t right = pc->operand1;

      Value* left_val = __get_register(this, left);
      Value* right_val = __get_register(this, right);

      struct String* left_str = left_val->u.str;
      struct String* right_str = right_val->u.str;

      size_t new_length = left_str->data_size + right_str->data_size;
      char* new_string = new char[new_length + 1];

      std::memcpy(new_string, left_str->data, left_str->data_size);
      std::memcpy(new_string + left_str->data_size, right_str->data, right_str->data_size);

      __set_register(this, left, Value(new_string));

      delete[] new_string;

      VM_NEXT();
    }

    VM_CASE(GETSTR) {
      operand_t dst = pc->operand0;
      operand_t str = pc->operand1;
      operand_t idx = pc->operand2;

      Value* str_val = __get_register(this, str);
      struct String* tstr = str_val->u.str;
      char chr = tstr->data[idx];

      __set_register(this, dst, Value(&chr));
      VM_NEXT();
    }

    VM_CASE(SETSTR) {
      operand_t str = pc->operand0;
      operand_t src = pc->operand1;
      operand_t idx = pc->operand2;

      Value* str_val = __get_register(this, str);
      struct String* tstr = str_val->u.str;

      char chr = static_cast<char>(src);
      char* str_cpy = duplicate_string(tstr->data);
      str_cpy[idx] = chr;

      __set_register(this, str, Value(str_cpy));
      delete[] str_cpy;
      VM_NEXT();
    }

    VM_CASE(ICAST) {
      operand_t dst = pc->operand0;
      operand_t src = pc->operand1;

      Value* target = __get_register(this, src);
      Value result = __to_int(this, *target);

      __set_register(this, dst, std::move(result));
      VM_NEXT();
    }

    VM_CASE(FCAST) {
      operand_t dst = pc->operand0;
      operand_t src = pc->operand1;

      Value* target = __get_register(this, src);
      Value result = __to_float(this, *target);

      __set_register(this, dst, std::move(result));
      VM_NEXT();
    }

    VM_CASE(STRCAST) {
      operand_t dst = pc->operand0;
      operand_t src = pc->operand1;

      Value* target = __get_register(this, src);
      Value result = __to_string(*target);

      __set_register(this, dst, std::move(result));
      VM_NEXT();
    }

    VM_CASE(BCAST) {
      operand_t dst = pc->operand0;
      operand_t src = pc->operand1;

      Value* target = __get_register(this, src);
      Value result = __to_bool(*target);

      __set_register(this, dst, std::move(result));
      VM_NEXT();
    }
  }

exit:;
}

} // namespace via
