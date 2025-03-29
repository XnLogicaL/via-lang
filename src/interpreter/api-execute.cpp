// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

// !========================================================================================== |
// ! DO NOT FUZZ THIS FILE! ONLY UNIT TEST AFTER CHECKING FOR THE VIA_DEBUG MACRO!             |
// !========================================================================================== |

#include "bit-utility.h"
#include "file-io.h"
#include "api-impl.h"
#include "api-aux.h"
#include "common.h"
#include "state.h"
#include "constant.h"
#include <cmath>

// Macro that throws a virtual error
#define VM_ERROR(message)                                                                          \
  do {                                                                                             \
    __set_error_state(this, message);                                                              \
    sig_error.fire();                                                                              \
    goto dispatch;                                                                                 \
  } while (0)

// Macro that throws a fatal error
#define VM_FATAL(message)                                                                          \
  do {                                                                                             \
    std::cerr << "VM terminated with message: " << message << '\n';                                \
    sig_fatal.fire();                                                                              \
    std::abort();                                                                                  \
  } while (0)

// Macro that completes an execution cycle
#define VM_NEXT()                                                                                  \
  do {                                                                                             \
    ++pc;                                                                                          \
    goto dispatch;                                                                                 \
  } while (0)

#define VM_CHECK_ZERO_DIVISON_INT(divisor)                                                         \
  if (divisor == 0) {                                                                              \
    VM_ERROR("Division by zero");                                                                  \
  }

#define VM_CHECK_ZERO_DIVISON_FLT(divisor)                                                         \
  if (divisor == 0.0f) {                                                                           \
    VM_ERROR("Division by zero");                                                                  \
  }

// ==================================================================================================
// execute.cpp
//
VIA_NAMESPACE_BEGIN

using enum ValueType;
using enum OpCode;
using enum ThreadState;

using namespace impl;

void vm_save_snapshot(State* VIA_RESTRICT V) {
  uint64_t pos = V->pc - V->ibp;
  std::string file = std::format("vm_snapshot.{}.log", pos);

  std::ostringstream headers;
  headers << "opcode: " << magic_enum::enum_name(V->pc->op) << "\n";
  headers << "operand0: " << V->pc->operand0 << "\n";
  headers << "operand1: " << V->pc->operand1 << "\n";
  headers << "operand2: " << V->pc->operand2 << "\n";

  std::ostringstream registers;
  registers << "==== registers ====\n";

  std::ostringstream stack;
  stack << "==== stack ====\n";

  // Generate stack map
  for (TValue* ptr = V->sbp; ptr < V->sbp + V->sp; ptr++) {
    uint32_t pos = ptr - V->sbp;
    stack << "|" << std::setw(2) << std::setfill('0') << pos << "| "
          << impl::__to_cxx_string(V, *ptr) << ' ' << get_raw_memory_dump(ptr, sizeof(TValue));
  }
  stack << "==== stack ====\n";

  // Generate register map
  for (Operand reg = 0; reg < VIA_REGISTER_COUNT; reg++) {
    TValue* val = impl::__get_register(V, reg);
    if (!val->is_nil()) {
      registers << "|R" << std::setw(2) << std::setfill('0') << reg << "| "
                << impl::__to_cxx_string(V, *val) << ' '
                << get_raw_memory_dump(val, sizeof(TValue));
    }
  }
  registers << "==== registers ====\n";

  // Combine all parts and write to file
  std::ostringstream output;
  output << headers.str() << "\n" << to_string(V) << "\n" << stack.str() << "\n" << registers.str();

  bool success = utils::write_to_file(std::format("./__viacache__/{}", file), output.str());
  if (!success) {
    return;
  }
}

// Starts VM execution cycle by altering it's state and "iterating" over
// the instruction pipeline.
void State::execute() {
  VIA_ASSERT(tstate == PAUSED, "Execute called on non-paused state");
  tstate = RUNNING;

  goto dispatch;

dispatch: {

#ifdef VIA_DEBUG
  vm_save_snapshot(this);
#endif

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

  // Abort is second priority due to verbosity.
  if (abort) {
    sig_abort.fire();
    goto exit;
  }

  switch (pc->op) {
  // Handle special/internal opcodes
  case NOP:
  case LABEL:
    VM_NEXT();

  case ADD: {
    Operand lhs = pc->operand0;
    Operand rhs = pc->operand1;

    TValue* lhs_val = __get_register(this, lhs);
    TValue* rhs_val = __get_register(this, rhs);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        lhs_val->val_integer += rhs_val->val_integer;
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        lhs_val->val_floating_point =
          static_cast<TFloat>(lhs_val->val_integer) + rhs_val->val_floating_point;
        lhs_val->type = floating_point;
      }
    }
    else if (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        lhs_val->val_floating_point += static_cast<TFloat>(rhs_val->val_integer);
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        lhs_val->val_floating_point += rhs_val->val_floating_point;
      }
    }

    VM_NEXT();
  }
  case ADDK: {
    Operand lhs = pc->operand0;
    Operand idx = pc->operand1;

    TValue* lhs_val = __get_register(this, lhs);
    const TValue& rhs_val = __get_constant(this, idx);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val.is_int()) {
        lhs_val->val_integer += rhs_val.val_integer;
      }
      else if VIA_UNLIKELY (rhs_val.is_float()) {
        lhs_val->val_floating_point =
          static_cast<TFloat>(lhs_val->val_integer) + rhs_val.val_floating_point;
        lhs_val->type = floating_point;
      }
    }
    else if (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val.is_int()) {
        lhs_val->val_floating_point += static_cast<TFloat>(rhs_val.val_integer);
      }
      else if VIA_UNLIKELY (rhs_val.is_float()) {
        lhs_val->val_floating_point += rhs_val.val_floating_point;
      }
    }

    VM_NEXT();
  }
  case ADDINT: {
    Operand lhs = pc->operand0;
    Operand int_high = pc->operand1;
    Operand int_low = pc->operand2;

    TValue* lhs_val = __get_register(this, lhs);
    TInteger imm = reinterpret_u16_as_i32(int_high, int_low);

    if VIA_LIKELY (lhs_val->is_int()) {
      lhs_val->val_integer += imm;
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point += imm;
    }

    VM_NEXT();
  }
  case ADDFLOAT: {
    Operand lhs = pc->operand0;
    Operand flt_high = pc->operand1;
    Operand flt_low = pc->operand2;

    TValue* lhs_val = __get_register(this, lhs);
    TFloat imm = reinterpret_u16_as_f32(flt_high, flt_low);

    if VIA_LIKELY (lhs_val->is_int()) {
      lhs_val->val_integer += imm;
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point += imm;
    }

    VM_NEXT();
  }

  case SUB: {
    Operand lhs = pc->operand0;
    Operand rhs = pc->operand1;

    TValue* lhs_val = __get_register(this, lhs);
    TValue* rhs_val = __get_register(this, rhs);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        lhs_val->val_integer -= rhs_val->val_integer;
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        lhs_val->val_floating_point =
          static_cast<TFloat>(lhs_val->val_integer) - rhs_val->val_floating_point;
        lhs_val->type = floating_point;
      }
    }
    else if (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        lhs_val->val_floating_point -= static_cast<TFloat>(rhs_val->val_integer);
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        lhs_val->val_floating_point -= rhs_val->val_floating_point;
      }
    }

    VM_NEXT();
  }
  case SUBK: {
    Operand lhs = pc->operand0;
    Operand idx = pc->operand1;

    TValue* lhs_val = __get_register(this, lhs);
    const TValue& rhs_val = __get_constant(this, idx);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val.is_int()) {
        lhs_val->val_integer -= rhs_val.val_integer;
      }
      else if VIA_UNLIKELY (rhs_val.is_float()) {
        lhs_val->val_floating_point =
          static_cast<TFloat>(lhs_val->val_integer) - rhs_val.val_floating_point;
        lhs_val->type = floating_point;
      }
    }
    else if (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val.is_int()) {
        lhs_val->val_floating_point -= static_cast<TFloat>(rhs_val.val_integer);
      }
      else if VIA_UNLIKELY (rhs_val.is_float()) {
        lhs_val->val_floating_point -= rhs_val.val_floating_point;
      }
    }

    VM_NEXT();
  }
  case SUBINT: {
    Operand lhs = pc->operand0;
    Operand int_high = pc->operand1;
    Operand int_low = pc->operand2;

    TValue* lhs_val = __get_register(this, lhs);
    TInteger imm = reinterpret_u16_as_i32(int_high, int_low);

    if VIA_LIKELY (lhs_val->is_int()) {
      lhs_val->val_integer -= imm;
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point -= imm;
    }

    VM_NEXT();
  }
  case SUBFLOAT: {
    Operand lhs = pc->operand0;
    Operand flt_high = pc->operand1;
    Operand flt_low = pc->operand2;

    TValue* lhs_val = __get_register(this, lhs);
    TFloat imm = reinterpret_u16_as_f32(flt_high, flt_low);

    if VIA_LIKELY (lhs_val->is_int()) {
      lhs_val->val_integer -= imm;
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point -= imm;
    }

    VM_NEXT();
  }

  case MUL: {
    Operand lhs = pc->operand0;
    Operand rhs = pc->operand1;

    TValue* lhs_val = __get_register(this, lhs);
    TValue* rhs_val = __get_register(this, rhs);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        lhs_val->val_integer *= rhs_val->val_integer;
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        lhs_val->val_floating_point =
          static_cast<TFloat>(lhs_val->val_integer) * rhs_val->val_floating_point;
        lhs_val->type = floating_point;
      }
    }
    else if (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        lhs_val->val_floating_point *= static_cast<TFloat>(rhs_val->val_integer);
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        lhs_val->val_floating_point *= rhs_val->val_floating_point;
      }
    }

    VM_NEXT();
  }
  case MULK: {
    Operand lhs = pc->operand0;
    Operand idx = pc->operand1;

    TValue* lhs_val = __get_register(this, lhs);
    const TValue& rhs_val = __get_constant(this, idx);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val.is_int()) {
        lhs_val->val_integer *= rhs_val.val_integer;
      }
      else if VIA_UNLIKELY (rhs_val.is_float()) {
        lhs_val->val_floating_point =
          static_cast<TFloat>(lhs_val->val_integer) * rhs_val.val_floating_point;
        lhs_val->type = floating_point;
      }
    }
    else if (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val.is_int()) {
        lhs_val->val_floating_point *= static_cast<TFloat>(rhs_val.val_integer);
      }
      else if VIA_UNLIKELY (rhs_val.is_float()) {
        lhs_val->val_floating_point *= rhs_val.val_floating_point;
      }
    }

    VM_NEXT();
  }
  case MULINT: {
    Operand lhs = pc->operand0;
    Operand int_high = pc->operand1;
    Operand int_low = pc->operand2;

    TValue* lhs_val = __get_register(this, lhs);
    TInteger imm = reinterpret_u16_as_i32(int_high, int_low);

    if VIA_LIKELY (lhs_val->is_int()) {
      lhs_val->val_integer *= imm;
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point *= imm;
    }

    VM_NEXT();
  }
  case MULFLOAT: {
    Operand lhs = pc->operand0;
    Operand flt_high = pc->operand1;
    Operand flt_low = pc->operand2;

    TValue* lhs_val = __get_register(this, lhs);
    TFloat imm = reinterpret_u16_as_f32(flt_high, flt_low);

    if VIA_LIKELY (lhs_val->is_int()) {
      lhs_val->val_integer *= imm;
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point *= imm;
    }

    VM_NEXT();
  }

  case DIV: {
    Operand lhs = pc->operand0;
    Operand rhs = pc->operand1;

    TValue* lhs_val = __get_register(this, lhs);
    TValue* rhs_val = __get_register(this, rhs);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        VM_CHECK_ZERO_DIVISON_INT(rhs_val);

        lhs_val->val_integer /= rhs_val->val_integer;
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        VM_CHECK_ZERO_DIVISON_FLT(rhs_val->val_floating_point);

        lhs_val->val_floating_point =
          static_cast<TFloat>(lhs_val->val_integer) / rhs_val->val_floating_point;
        lhs_val->type = floating_point;
      }
    }
    else if (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        VM_CHECK_ZERO_DIVISON_INT(rhs_val);

        lhs_val->val_floating_point /= static_cast<TFloat>(rhs_val->val_integer);
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        VM_CHECK_ZERO_DIVISON_FLT(rhs_val->val_floating_point);

        lhs_val->val_floating_point /= rhs_val->val_floating_point;
      }
    }

    VM_NEXT();
  }
  case DIVK: {
    Operand lhs = pc->operand0;
    Operand idx = pc->operand1;

    TValue* lhs_val = __get_register(this, lhs);
    const TValue& rhs_val = __get_constant(this, idx);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val.is_int()) {
        VM_CHECK_ZERO_DIVISON_INT(rhs_val.val_integer);

        lhs_val->val_integer /= rhs_val.val_integer;
      }
      else if VIA_UNLIKELY (rhs_val.is_float()) {
        VM_CHECK_ZERO_DIVISON_FLT(rhs_val.val_floating_point);

        lhs_val->val_floating_point =
          static_cast<TFloat>(lhs_val->val_integer) / rhs_val.val_floating_point;
        lhs_val->type = floating_point;
      }
    }
    else if (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val.is_int()) {
        VM_CHECK_ZERO_DIVISON_INT(rhs_val.val_integer);

        lhs_val->val_floating_point /= static_cast<TFloat>(rhs_val.val_integer);
      }
      else if VIA_UNLIKELY (rhs_val.is_float()) {
        VM_CHECK_ZERO_DIVISON_FLT(rhs_val.val_floating_point);

        lhs_val->val_floating_point /= rhs_val.val_floating_point;
      }
    }

    VM_NEXT();
  }
  case DIVINT: {
    Operand lhs = pc->operand0;
    Operand int_high = pc->operand1;
    Operand int_low = pc->operand2;

    TValue* lhs_val = __get_register(this, lhs);
    TInteger imm = reinterpret_u16_as_i32(int_high, int_low);

    VM_CHECK_ZERO_DIVISON_INT(imm);

    if VIA_LIKELY (lhs_val->is_int()) {
      lhs_val->val_integer /= imm;
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point /= imm;
    }

    VM_NEXT();
  }
  case DIVFLOAT: {
    Operand lhs = pc->operand0;
    Operand flt_high = pc->operand1;
    Operand flt_low = pc->operand2;

    TValue* lhs_val = __get_register(this, lhs);
    TFloat imm = reinterpret_u16_as_f32(flt_high, flt_low);

    VM_CHECK_ZERO_DIVISON_FLT(imm);

    if VIA_LIKELY (lhs_val->is_int()) {
      lhs_val->val_integer /= imm;
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point /= imm;
    }

    VM_NEXT();
  }

  case POW: {
    Operand lhs = pc->operand0;
    Operand rhs = pc->operand1;

    TValue* lhs_val = __get_register(this, lhs);
    TValue* rhs_val = __get_register(this, rhs);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        lhs_val->val_integer = std::pow(lhs_val->val_integer, rhs_val->val_integer);
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        lhs_val->val_floating_point =
          std::pow(static_cast<TFloat>(lhs_val->val_integer), rhs_val->val_floating_point);
        lhs_val->type = floating_point;
      }
    }
    else if (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        lhs_val->val_floating_point =
          std::pow(lhs_val->val_floating_point, static_cast<TFloat>(rhs_val->val_integer));
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        lhs_val->val_floating_point =
          std::pow(lhs_val->val_floating_point, rhs_val->val_floating_point);
      }
    }

    VM_NEXT();
  }
  case POWK: {
    Operand lhs = pc->operand0;
    Operand idx = pc->operand1;

    TValue* lhs_val = __get_register(this, lhs);
    const TValue& rhs_val = __get_constant(this, idx);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val.is_int()) {
        lhs_val->val_integer = std::pow(lhs_val->val_integer, rhs_val.val_integer);
      }
      else if VIA_UNLIKELY (rhs_val.is_float()) {
        lhs_val->val_floating_point =
          std::pow(static_cast<TFloat>(lhs_val->val_integer), rhs_val.val_floating_point);
        lhs_val->type = floating_point;
      }
    }
    else if (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val.is_int()) {
        lhs_val->val_floating_point =
          std::pow(lhs_val->val_floating_point, static_cast<TFloat>(rhs_val.val_integer));
      }
      else if VIA_UNLIKELY (rhs_val.is_float()) {
        lhs_val->val_floating_point =
          std::pow(lhs_val->val_floating_point, rhs_val.val_floating_point);
      }
    }

    VM_NEXT();
  }
  case POWINT: {
    Operand lhs = pc->operand0;
    Operand int_high = pc->operand1;
    Operand int_low = pc->operand2;

    TValue* lhs_val = __get_register(this, lhs);
    TInteger imm = reinterpret_u16_as_i32(int_high, int_low);

    if VIA_LIKELY (lhs_val->is_int()) {
      lhs_val->val_integer = std::pow(lhs_val->val_integer, imm);
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point = std::pow(lhs_val->val_floating_point, imm);
    }

    VM_NEXT();
  }
  case POWFLOAT: {
    Operand lhs = pc->operand0;
    Operand flt_high = pc->operand1;
    Operand flt_low = pc->operand2;

    TValue* lhs_val = __get_register(this, lhs);
    TFloat imm = reinterpret_u16_as_f32(flt_high, flt_low);

    if VIA_LIKELY (lhs_val->is_int()) {
      lhs_val->val_integer = std::pow(lhs_val->val_integer, imm);
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point = std::pow(lhs_val->val_floating_point, imm);
    }

    VM_NEXT();
  }

  case MOD: {
    Operand lhs = pc->operand0;
    Operand rhs = pc->operand1;

    TValue* lhs_val = __get_register(this, lhs);
    TValue* rhs_val = __get_register(this, rhs);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        lhs_val->val_integer %= rhs_val->val_integer;
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        lhs_val->val_floating_point =
          std::fmod(static_cast<TFloat>(lhs_val->val_integer), rhs_val->val_floating_point);
        lhs_val->type = floating_point;
      }
    }
    else if (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        lhs_val->val_floating_point =
          std::fmod(lhs_val->val_floating_point, static_cast<TFloat>(rhs_val->val_integer));
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        lhs_val->val_floating_point =
          std::fmod(lhs_val->val_floating_point, rhs_val->val_floating_point);
      }
    }

    VM_NEXT();
  }
  case MODK: {
    Operand lhs = pc->operand0;
    Operand idx = pc->operand1;

    TValue* lhs_val = __get_register(this, lhs);
    const TValue& rhs_val = __get_constant(this, idx);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val.is_int()) {
        lhs_val->val_integer %= rhs_val.val_integer;
      }
      else if VIA_UNLIKELY (rhs_val.is_float()) {
        lhs_val->val_floating_point =
          std::fmod(static_cast<TFloat>(lhs_val->val_integer), rhs_val.val_floating_point);
        lhs_val->type = floating_point;
      }
    }
    else if (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val.is_int()) {
        lhs_val->val_floating_point =
          std::fmod(lhs_val->val_floating_point, static_cast<TFloat>(rhs_val.val_integer));
      }
      else if VIA_UNLIKELY (rhs_val.is_float()) {
        lhs_val->val_floating_point =
          std::fmod(lhs_val->val_floating_point, rhs_val.val_floating_point);
      }
    }

    VM_NEXT();
  }
  case MODINT: {
    Operand lhs = pc->operand0;
    Operand int_high = pc->operand1;
    Operand int_low = pc->operand2;

    TValue* lhs_val = __get_register(this, lhs);
    TInteger imm = reinterpret_u16_as_i32(int_high, int_low);



    if VIA_LIKELY (lhs_val->is_int()) {
      lhs_val->val_integer = std::fmod(lhs_val->val_integer, imm);
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point = std::fmod(lhs_val->val_floating_point, imm);
    }

    VM_NEXT();
  }
  case MODFLOAT: {
    Operand lhs = pc->operand0;
    Operand flt_high = pc->operand1;
    Operand flt_low = pc->operand2;

    TValue* lhs_val = __get_register(this, lhs);
    TFloat imm = reinterpret_u16_as_f32(flt_high, flt_low);



    if VIA_LIKELY (lhs_val->is_int()) {
      lhs_val->val_integer = std::fmod(lhs_val->val_integer, imm);
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point = std::fmod(lhs_val->val_floating_point, imm);
    }

    VM_NEXT();
  }

  case NEG: {
    Operand dst = pc->operand0;
    TValue* val = __get_register(this, dst);
    ValueType type = val->type;

    if (type == integer) {
      val->val_integer = -val->val_integer;
    }
    else if (type == floating_point) {
      val->val_floating_point = -val->val_floating_point;
    }

    VM_NEXT();
  }

  case MOVE: {
    Operand rdst = pc->operand0;
    Operand rsrc = pc->operand1;
    TValue* src_val = __get_register(this, rsrc);

    __set_register(this, rdst, *src_val);
    VM_NEXT();
  }

  case LOADK: {
    Operand dst = pc->operand0;
    Operand idx = pc->operand1;

    const TValue& kval = __get_constant(this, idx);

    __set_register(this, dst, kval);
    VM_NEXT();
  }

  case LOADNIL: {
    Operand dst = pc->operand0;

    __set_register(this, dst, _Nil);
    VM_NEXT();
  }

  case LOADINT: {
    Operand dst = pc->operand0;
    TInteger imm = reinterpret_u16_as_u32(pc->operand1, pc->operand2);

    __set_register(this, dst, TValue(imm));
    VM_NEXT();
  }

  case LOADFLOAT: {
    Operand dst = pc->operand0;
    TFloat imm = reinterpret_u16_as_f32(pc->operand1, pc->operand2);

    __set_register(this, dst, TValue(imm));
    VM_NEXT();
  }

  case LOADTRUE: {
    Operand dst = pc->operand0;
    __set_register(this, dst, TValue(true));
    VM_NEXT();
  }

  case LOADFALSE: {
    Operand dst = pc->operand0;
    __set_register(this, dst, TValue(false));
    VM_NEXT();
  }

  case LOADTABLE: {
    Operand dst = pc->operand0;
    TValue ttable(new TTable());

    __set_register(this, dst, ttable);
    VM_NEXT();
  }

  case LOADFUNCTION: {
    Operand dst = pc->operand0;
    TFunction* func = new TFunction();

    __closure_bytecode_load(this, func);
    __set_register(this, dst, TValue(function, func));
    VM_NEXT();
  }

  case GETUPVALUE: {
    Operand dst = pc->operand0;
    Operand upv_id = pc->operand1;
    UpValue* upv = __closure_upv_get(frame, upv_id);

    dump_struct(*upv->value);

    __set_register(this, dst, *upv->value);
    VM_NEXT();
  }

  case SETUPVALUE: {
    Operand src = pc->operand0;
    Operand upv_id = pc->operand1;
    TValue* val = __get_register(this, src);

    __closure_upv_set(frame, upv_id, *val);
    VM_NEXT();
  }

  case PUSH: {
    Operand src = pc->operand0;
    TValue* val = __get_register(this, src);

    __push(this, *val);
    VM_NEXT();
  }

  case PUSHK: {
    Operand const_idx = pc->operand0;
    TValue constant = __get_constant(this, const_idx);

    __push(this, constant);
    VM_NEXT();
  }

  case PUSHNIL: {
    __push(this, TValue());
    VM_NEXT();
  }

  case PUSHINT: {
    TInteger imm = reinterpret_u16_as_u32(pc->operand0, pc->operand1);
    __push(this, TValue(imm));
    VM_NEXT();
  }

  case PUSHFLOAT: {
    TFloat imm = reinterpret_u16_as_f32(pc->operand0, pc->operand1);
    __push(this, TValue(imm));
    VM_NEXT();
  }

  case PUSHTRUE: {
    __push(this, TValue(true));
    VM_NEXT();
  }

  case PUSHFALSE: {
    __push(this, TValue(false));
    VM_NEXT();
  }

  case POP: {
    Operand dst = pc->operand0;
    TValue val = __pop(this);

    __set_register(this, dst, val);
    VM_NEXT();
  }

  case DROP: {
    __pop(this);
    VM_NEXT();
  }

  case GETSTACK: {
    Operand dst = pc->operand0;
    Operand off = pc->operand1;

    const TValue& val = __get_stack(this, off);

    __set_register(this, dst, val);
    VM_NEXT();
  }

  case SETSTACK: {
    Operand src = pc->operand0;
    Operand off = pc->operand1;

    TValue* val = __get_register(this, src);

    sbp[off] = std::move(*val);
    VM_NEXT();
  }

  case GETARGUMENT: {
    Operand dst = pc->operand0;
    Operand off = pc->operand1;

    const TValue& val = __get_argument(this, off);

    __set_register(this, dst, val);
    VM_NEXT();
  }

  case GETGLOBAL: {
    Operand dst = pc->operand0;

    uint32_t hash = reinterpret_u16_as_u32(pc->operand1, pc->operand2);
    const TValue& global = __get_global(this, hash);

    __set_register(this, dst, global);
    VM_NEXT();
  }

  case SETGLOBAL: {
    Operand src = pc->operand0;

    uint32_t hash = reinterpret_u16_as_u32(pc->operand1, pc->operand2);
    TValue* global = __get_register(this, src);

    __set_global(this, hash, *global);
    VM_NEXT();
  }

  case EQUAL: {
    Operand dst = pc->operand0;
    Operand lhs = pc->operand1;
    Operand rhs = pc->operand2;

    if VIA_UNLIKELY (lhs == rhs) {
      __set_register(this, dst, TValue(true));
      VM_NEXT();
    }

    TValue* lhs_val = __get_register(this, lhs);
    TValue* rhs_val = __get_register(this, rhs);

    if VIA_UNLIKELY (lhs_val == rhs_val) {
      __set_register(this, dst, TValue(true));
      VM_NEXT();
    }

    bool result = __compare(*lhs_val, *rhs_val);
    __set_register(this, dst, TValue(result));

    VM_NEXT();
  }

  case NOTEQUAL: {
    Operand dst = pc->operand0;
    Operand lhs = pc->operand1;
    Operand rhs = pc->operand2;

    if VIA_LIKELY (lhs != rhs) {
      __set_register(this, dst, TValue(true));
      VM_NEXT();
    }

    TValue* lhs_val = __get_register(this, lhs);
    TValue* rhs_val = __get_register(this, rhs);

    if VIA_LIKELY (lhs_val != rhs_val) {
      __set_register(this, dst, TValue(true));
      VM_NEXT();
    }

    bool result = __compare(*lhs_val, *rhs_val);
    __set_register(this, dst, TValue(result));

    VM_NEXT();
  }

  case AND: {
    Operand dst = pc->operand0;
    Operand lhs = pc->operand1;
    Operand rhs = pc->operand2;

    TValue* lhs_val = __get_register(this, lhs);
    TValue* rhs_val = __get_register(this, rhs);
    bool cond = __to_cxx_bool(*lhs_val) && __to_cxx_bool(*rhs_val);

    __set_register(this, dst, TValue(cond));
    VM_NEXT();
  }

  case OR: {
    Operand dst = pc->operand0;
    Operand lhs = pc->operand1;
    Operand rhs = pc->operand2;

    TValue* lhs_val = __get_register(this, lhs);
    TValue* rhs_val = __get_register(this, rhs);
    bool cond = __to_cxx_bool(*lhs_val) || __to_cxx_bool(*rhs_val);

    __set_register(this, dst, TValue(cond));
    VM_NEXT();
  }

  case NOT: {
    Operand dst = pc->operand0;
    Operand lhs = pc->operand1;

    TValue* lhs_val = __get_register(this, lhs);
    bool cond = !__to_cxx_bool(*lhs_val);

    __set_register(this, dst, TValue(cond));
    VM_NEXT();
  }

  case LESS: {
    Operand dst = pc->operand0;
    Operand lhs = pc->operand1;
    Operand rhs = pc->operand2;

    TValue* lhs_val = __get_register(this, lhs);
    TValue* rhs_val = __get_register(this, rhs);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        __set_register(this, dst, TValue(lhs_val->val_integer < rhs_val->val_integer));
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        __set_register(
          this, dst, TValue(static_cast<TFloat>(lhs_val->val_integer) < rhs_val->val_floating_point)
        );
      }
    }
    else if VIA_UNLIKELY (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        __set_register(
          this, dst, TValue(lhs_val->val_floating_point < static_cast<TFloat>(rhs_val->val_integer))
        );
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        __set_register(
          this, dst, TValue(lhs_val->val_floating_point < rhs_val->val_floating_point)
        );
      }
    }

    VM_NEXT();
  }

  case GREATER: {
    Operand dst = pc->operand0;
    Operand lhs = pc->operand1;
    Operand rhs = pc->operand2;

    TValue* lhs_val = __get_register(this, lhs);
    TValue* rhs_val = __get_register(this, rhs);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        __set_register(this, dst, TValue(lhs_val->val_integer > rhs_val->val_integer));
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        __set_register(
          this, dst, TValue(static_cast<TFloat>(lhs_val->val_integer) > rhs_val->val_floating_point)
        );
      }
    }
    else if VIA_UNLIKELY (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        __set_register(
          this, dst, TValue(lhs_val->val_floating_point > static_cast<TFloat>(rhs_val->val_integer))
        );
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        __set_register(
          this, dst, TValue(lhs_val->val_floating_point > rhs_val->val_floating_point)
        );
      }
    }

    VM_NEXT();
  }

  case LESSOREQUAL: {
    Operand dst = pc->operand0;
    Operand lhs = pc->operand1;
    Operand rhs = pc->operand2;

    TValue* lhs_val = __get_register(this, lhs);
    TValue* rhs_val = __get_register(this, rhs);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        __set_register(this, dst, TValue(lhs_val->val_integer <= rhs_val->val_integer));
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        __set_register(
          this,
          dst,
          TValue(static_cast<TFloat>(lhs_val->val_integer) <= rhs_val->val_floating_point)
        );
      }
    }
    else if VIA_UNLIKELY (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        __set_register(
          this,
          dst,
          TValue(lhs_val->val_floating_point <= static_cast<TFloat>(rhs_val->val_integer))
        );
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        __set_register(
          this, dst, TValue(lhs_val->val_floating_point <= rhs_val->val_floating_point)
        );
      }
    }

    VM_NEXT();
  }

  case GREATEROREQUAL: {
    Operand dst = pc->operand0;
    Operand lhs = pc->operand1;
    Operand rhs = pc->operand2;

    TValue* lhs_val = __get_register(this, lhs);
    TValue* rhs_val = __get_register(this, rhs);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        __set_register(this, dst, TValue(lhs_val->val_integer >= rhs_val->val_integer));
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        __set_register(
          this,
          dst,
          TValue(static_cast<TFloat>(lhs_val->val_integer) >= rhs_val->val_floating_point)
        );
      }
    }
    else if VIA_UNLIKELY (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        __set_register(
          this,
          dst,
          TValue(lhs_val->val_floating_point >= static_cast<TFloat>(rhs_val->val_integer))
        );
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        __set_register(
          this, dst, TValue(lhs_val->val_floating_point >= rhs_val->val_floating_point)
        );
      }
    }

    VM_NEXT();
  }

  case EXIT: {
    goto exit;
  }

  case JUMP: {
    OperandS offset = pc->operand0;
    pc += offset;
    goto dispatch;
  }

  case JUMPIF: {
    Operand cond = pc->operand0;
    OperandS offset = pc->operand1;

    TValue* cond_val = __get_register(this, cond);
    if (__to_cxx_bool(*cond_val)) {
      pc += offset;
    }

    goto dispatch;
  }

  case JUMPIFNOT: {
    Operand cond = pc->operand0;
    OperandS offset = pc->operand1;

    TValue* cond_val = __get_register(this, cond);
    if (!__to_cxx_bool(*cond_val)) {
      pc += offset;
    }

    goto dispatch;
  }

  case JUMPIFEQUAL: {
    Operand cond_lhs = pc->operand0;
    Operand cond_rhs = pc->operand1;
    OperandS offset = pc->operand2;

    if VIA_UNLIKELY (cond_lhs == cond_rhs) {
      pc += offset;
    }
    else {
      TValue* lhs_val = __get_register(this, cond_lhs);
      TValue* rhs_val = __get_register(this, cond_rhs);

      if VIA_UNLIKELY (lhs_val == rhs_val || __compare(*lhs_val, *rhs_val)) {
        pc += offset;
      }
    }

    goto dispatch;
  }

  case JUMPIFNOTEQUAL: {
    Operand cond_lhs = pc->operand0;
    Operand cond_rhs = pc->operand1;
    OperandS offset = pc->operand2;

    if VIA_LIKELY (cond_lhs != cond_rhs) {
      pc += offset;
    }
    else {
      TValue* lhs_val = __get_register(this, cond_lhs);
      TValue* rhs_val = __get_register(this, cond_rhs);

      if VIA_LIKELY (lhs_val != rhs_val || !__compare(*lhs_val, *rhs_val)) {
        pc += offset;
      }
    }

    goto dispatch;
  }

  case JUMPIFLESS: {
    Operand cond_lhs = pc->operand0;
    Operand cond_rhs = pc->operand1;
    OperandS offset = pc->operand2;

    TValue* lhs_val = __get_register(this, cond_lhs);
    TValue* rhs_val = __get_register(this, cond_rhs);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        if (lhs_val->val_integer < rhs_val->val_integer) {
          pc += offset;
        }
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        if (static_cast<TFloat>(lhs_val->val_integer) < rhs_val->val_floating_point) {
          pc += offset;
        }
      }
    }
    else if VIA_UNLIKELY (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        if (lhs_val->val_floating_point < static_cast<TFloat>(rhs_val->val_integer)) {
          pc += offset;
        }
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        if (lhs_val->val_floating_point < rhs_val->val_floating_point) {
          pc += offset;
        }
      }
    }

    goto dispatch;
  }

  case JUMPIFGREATER: {
    Operand cond_lhs = pc->operand0;
    Operand cond_rhs = pc->operand1;
    OperandS offset = pc->operand2;

    TValue* lhs_val = __get_register(this, cond_lhs);
    TValue* rhs_val = __get_register(this, cond_rhs);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        if (lhs_val->val_integer > rhs_val->val_integer) {
          pc += offset;
        }
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        if (static_cast<TFloat>(lhs_val->val_integer) > rhs_val->val_floating_point) {
          pc += offset;
        }
      }
    }
    else if VIA_UNLIKELY (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        if (lhs_val->val_floating_point > static_cast<TFloat>(rhs_val->val_integer)) {
          pc += offset;
        }
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        if (lhs_val->val_floating_point > rhs_val->val_floating_point) {
          pc += offset;
        }
      }
    }

    goto dispatch;
  }

  case JUMPIFLESSOREQUAL: {
    Operand cond_lhs = pc->operand0;
    Operand cond_rhs = pc->operand1;
    OperandS offset = pc->operand2;

    TValue* lhs_val = __get_register(this, cond_lhs);
    TValue* rhs_val = __get_register(this, cond_rhs);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        if (lhs_val->val_integer <= rhs_val->val_integer) {
          pc += offset;
        }
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        if (static_cast<TFloat>(lhs_val->val_integer) <= rhs_val->val_floating_point) {
          pc += offset;
        }
      }
    }
    else if VIA_UNLIKELY (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        if (lhs_val->val_floating_point <= static_cast<TFloat>(rhs_val->val_integer)) {
          pc += offset;
        }
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        if (lhs_val->val_floating_point <= rhs_val->val_floating_point) {
          pc += offset;
        }
      }
    }

    goto dispatch;
  }

  case JUMPIFGREATEROREQUAL: {
    Operand cond_lhs = pc->operand0;
    Operand cond_rhs = pc->operand1;
    OperandS offset = pc->operand2;

    TValue* lhs_val = __get_register(this, cond_lhs);
    TValue* rhs_val = __get_register(this, cond_rhs);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        if (lhs_val->val_integer >= rhs_val->val_integer) {
          pc += offset;
        }
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        if (static_cast<TFloat>(lhs_val->val_integer) >= rhs_val->val_floating_point) {
          pc += offset;
        }
      }
    }
    else if VIA_UNLIKELY (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        if (lhs_val->val_floating_point >= static_cast<TFloat>(rhs_val->val_integer)) {
          pc += offset;
        }
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        if (lhs_val->val_floating_point >= rhs_val->val_floating_point) {
          pc += offset;
        }
      }
    }

    goto dispatch;
  }

  case JUMPLABEL: {
    Operand label = pc->operand0;

    pc = __label_get(this, label);

    goto dispatch;
  }

  case JUMPLABELIF: {
    Operand cond = pc->operand0;
    Operand label = pc->operand1;

    TValue* cond_val = __get_register(this, cond);
    if (__to_cxx_bool(*cond_val)) {
      pc = __label_get(this, label);
    }

    goto dispatch;
  }

  case JUMPLABELIFNOT: {
    Operand cond = pc->operand0;
    Operand label = pc->operand1;

    TValue* cond_val = __get_register(this, cond);
    if (!__to_cxx_bool(*cond_val)) {
      pc = __label_get(this, label);
    }

    goto dispatch;
  }

  case JUMPLABELIFEQUAL: {
    Operand cond_lhs = pc->operand0;
    Operand cond_rhs = pc->operand1;
    Operand label = pc->operand2;

    if VIA_UNLIKELY (cond_lhs == cond_rhs) {
      pc = __label_get(this, label);
    }
    else {
      TValue* lhs_val = __get_register(this, cond_lhs);
      TValue* rhs_val = __get_register(this, cond_rhs);

      if VIA_UNLIKELY (lhs_val == rhs_val || __compare(*lhs_val, *rhs_val)) {
        pc = __label_get(this, label);
      }
    }

    goto dispatch;
  }

  case JUMPLABELIFNOTEQUAL: {
    Operand cond_lhs = pc->operand0;
    Operand cond_rhs = pc->operand1;
    Operand label = pc->operand2;

    if VIA_LIKELY (cond_lhs != cond_rhs) {
      pc = __label_get(this, label);
    }
    else {
      TValue* lhs_val = __get_register(this, cond_lhs);
      TValue* rhs_val = __get_register(this, cond_rhs);

      if VIA_LIKELY (lhs_val != rhs_val || !__compare(*lhs_val, *rhs_val)) {
        pc = __label_get(this, label);
      }
    }

    goto dispatch;
  }

  case JUMPLABELIFLESS: {
    Operand cond_lhs = pc->operand0;
    Operand cond_rhs = pc->operand1;
    Operand label = pc->operand2;

    TValue* lhs_val = __get_register(this, cond_lhs);
    TValue* rhs_val = __get_register(this, cond_rhs);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        if (lhs_val->val_integer < rhs_val->val_integer) {
          pc = __label_get(this, label);
        }
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        if (static_cast<TFloat>(lhs_val->val_integer) < rhs_val->val_floating_point) {
          pc = __label_get(this, label);
        }
      }
    }
    else if VIA_UNLIKELY (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        if (lhs_val->val_floating_point < static_cast<TFloat>(rhs_val->val_integer)) {
          pc = __label_get(this, label);
        }
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        if (lhs_val->val_floating_point < rhs_val->val_floating_point) {
          pc = __label_get(this, label);
        }
      }
    }

    goto dispatch;
  }

  case JUMPLABELIFGREATER: {
    Operand cond_lhs = pc->operand0;
    Operand cond_rhs = pc->operand1;
    Operand label = pc->operand2;

    TValue* lhs_val = __get_register(this, cond_lhs);
    TValue* rhs_val = __get_register(this, cond_rhs);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        if (lhs_val->val_integer > rhs_val->val_integer) {
          pc = __label_get(this, label);
        }
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        if (static_cast<TFloat>(lhs_val->val_integer) > rhs_val->val_floating_point) {
          pc = __label_get(this, label);
        }
      }
    }
    else if VIA_UNLIKELY (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        if (lhs_val->val_floating_point > static_cast<TFloat>(rhs_val->val_integer)) {
          pc = __label_get(this, label);
        }
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        if (lhs_val->val_floating_point > rhs_val->val_floating_point) {
          pc = __label_get(this, label);
        }
      }
    }

    goto dispatch;
  }

  case JUMPLABELIFLESSOREQUAL: {
    Operand cond_lhs = pc->operand0;
    Operand cond_rhs = pc->operand1;
    Operand label = pc->operand2;

    TValue* lhs_val = __get_register(this, cond_lhs);
    TValue* rhs_val = __get_register(this, cond_rhs);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        if (lhs_val->val_integer <= rhs_val->val_integer) {
          pc = __label_get(this, label);
        }
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        if (static_cast<TFloat>(lhs_val->val_integer) <= rhs_val->val_floating_point) {
          pc = __label_get(this, label);
        }
      }
    }
    else if VIA_UNLIKELY (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        if (lhs_val->val_floating_point <= static_cast<TFloat>(rhs_val->val_integer)) {
          pc = __label_get(this, label);
        }
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        if (lhs_val->val_floating_point <= rhs_val->val_floating_point) {
          pc = __label_get(this, label);
        }
      }
    }

    goto dispatch;
  }

  case JUMPLABELIFGREATEROREQUAL: {
    Operand cond_lhs = pc->operand0;
    Operand cond_rhs = pc->operand1;
    Operand label = pc->operand2;

    TValue* lhs_val = __get_register(this, cond_lhs);
    TValue* rhs_val = __get_register(this, cond_rhs);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        if (lhs_val->val_integer >= rhs_val->val_integer) {
          pc = __label_get(this, label);
        }
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        if (static_cast<TFloat>(lhs_val->val_integer) >= rhs_val->val_floating_point) {
          pc = __label_get(this, label);
        }
      }
    }
    else if VIA_UNLIKELY (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        if (lhs_val->val_floating_point >= static_cast<TFloat>(rhs_val->val_integer)) {
          pc = __label_get(this, label);
        }
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        if (lhs_val->val_floating_point >= rhs_val->val_floating_point) {
          pc = __label_get(this, label);
        }
      }
    }

    goto dispatch;
  }

  case CALL: {
    Operand fn = pc->operand0;
    Operand argc = pc->operand1;
    TValue* fn_val = __get_register(this, fn);

    __call(this, *fn_val, argc);
    VM_NEXT();
  }

  case EXTERNCALL: {
    Operand fn = pc->operand0;
    Operand argc = pc->operand1;
    TValue* cfunc = __get_register(this, fn);

    __extern_call(this, cfunc->cast_ptr<TCFunction>(), argc);
    VM_NEXT();
  }

  case NATIVECALL: {
    Operand fn = pc->operand0;
    Operand argc = pc->operand1;
    TValue* func = __get_register(this, fn);

    __native_call(this, func->cast_ptr<TFunction>(), argc);
    VM_NEXT();
  }

  case METHODCALL: {
    Operand obj = pc->operand0;
    Operand fn = pc->operand1;
    Operand argc = pc->operand2;

    TValue* func = __get_register(this, fn);
    TValue* object = __get_register(this, obj);

    __push(this, object->clone());
    __native_call(this, func->cast_ptr<TFunction>(), argc + 1);
    VM_NEXT();
  }

  case RETURN: {
    Operand src = pc->operand0;
    TValue* val = __get_register(this, src);

    __closure_close_upvalues(frame);
    __native_return(this, *val);
    VM_NEXT();
  }

  case GETTABLE: {
    Operand dst = pc->operand0;
    Operand tbl = pc->operand1;
    Operand key = pc->operand2;

    TValue* tbl_val = __get_register(this, tbl);
    TValue* key_val = __get_register(this, key);

    const TValue& index = __table_get(tbl_val->cast_ptr<TTable>(), *key_val);

    __set_register(this, dst, index);
    VM_NEXT();
  }

  case SETTABLE: {
    Operand src = pc->operand0;
    Operand tbl = pc->operand1;
    Operand ky = pc->operand2;

    TValue* table = __get_register(this, tbl);
    TValue* value = __get_register(this, src);
    TValue* key = __get_register(this, ky);

    __table_set(table->cast_ptr<TTable>(), TValue(key), *value);
    VM_NEXT();
  }

  case NEXTTABLE: {
    static std::unordered_map<void*, Operand> next_table;

    Operand dst = pc->operand0;
    Operand valr = pc->operand1;

    TValue* val = __get_register(this, valr);
    void* ptr = __to_pointer(*val);
    Operand key = 0;

    auto it = next_table.find(ptr);
    if (it != next_table.end()) {
      key = ++it->second;
    }
    else {
      next_table[ptr] = 0;
    }

    const TValue& field = __table_get(val->cast_ptr<TTable>(), TValue(key));
    __set_register(this, dst, field);
    VM_NEXT();
  }

  case LENTABLE: {
    Operand dst = pc->operand0;
    Operand tbl = pc->operand1;

    TValue* val = __get_register(this, tbl);
    TInteger size = __table_size(val->cast_ptr<TTable>());

    __set_register(this, dst, TValue(size));
    VM_NEXT();
  }

  case LENSTRING: {
    Operand rdst = pc->operand0;
    Operand objr = pc->operand1;

    TValue* val = __get_register(this, objr);
    TInteger len = val->cast_ptr<TString>()->len;

    __set_register(this, rdst, TValue(len));
    VM_NEXT();
  }

  case CONCAT: {
    Operand left = pc->operand0;
    Operand right = pc->operand1;

    TValue* left_val = __get_register(this, left);
    TValue* right_val = __get_register(this, right);

    TString* left_str = left_val->cast_ptr<TString>();
    TString* right_str = right_val->cast_ptr<TString>();

    size_t new_length = left_str->len + right_str->len;
    char* new_string = new char[new_length + 1];

    std::memcpy(new_string, left_str->data, left_str->len);
    std::memcpy(new_string + left_str->len, right_str->data, right_str->len);

    TString* new_str = new TString(this, new_string);

    __set_register(this, left, TValue(string, new_str));

    delete[] new_string;

    VM_NEXT();
  }

  case GETSTRING: {
    Operand dst = pc->operand0;
    Operand str = pc->operand1;
    Operand idx = pc->operand2;

    TValue* str_val = __get_register(this, str);
    TString* tstr = str_val->cast_ptr<TString>();

    char chr = tstr->data[idx];
    TString* result = new TString(this, &chr);

    __set_register(this, dst, TValue(string, result));
    VM_NEXT();
  }

  case SETSTRING: {
    Operand str = pc->operand0;
    Operand src = pc->operand1;
    Operand idx = pc->operand2;

    TValue* str_val = __get_register(this, str);
    TString* tstr = str_val->cast_ptr<TString>();

    char chr = static_cast<char>(src);
    char* str_cpy = duplicate_string(tstr->data);
    str_cpy[idx] = chr;

    TString* result = new TString(this, str_cpy);
    __set_register(this, str, TValue(result));

    delete[] str_cpy;
    VM_NEXT();
  }

  case INTCAST: {
    Operand dst = pc->operand0;
    Operand src = pc->operand1;

    TValue* target = __get_register(this, src);
    TValue result = __to_int(this, *target);

    __set_register(this, dst, result);
    VM_NEXT();
  }

  case FLOATCAST: {
    Operand dst = pc->operand0;
    Operand src = pc->operand1;

    TValue* target = __get_register(this, src);
    TValue result = __to_float(this, *target);

    __set_register(this, dst, result);
    VM_NEXT();
  }

  case STRINGCAST: {
    Operand dst = pc->operand0;
    Operand src = pc->operand1;

    TValue* target = __get_register(this, src);
    TValue result = __to_string(this, *target);

    __set_register(this, dst, result);
    VM_NEXT();
  }

  case BOOLCAST: {
    Operand dst = pc->operand0;
    Operand src = pc->operand1;

    TValue* target = __get_register(this, src);
    TValue result = __to_bool(*target);

    __set_register(this, dst, result);
    VM_NEXT();
  }

  default: {
    VM_FATAL(std::format("unknown opcode 0x{:x}", static_cast<int>(pc->op)));
  }
  }
}

exit:
  sig_exit.fire();
  tstate = PAUSED;
}

// Permanently kills the thread. Does not clean up the state object.
void State::kill() {
  if (tstate == RUNNING) {
    abort = true;
    sig_exit.wait();
  }

  // Mark as dead thread
  tstate = DEAD;
  G->threads.fetch_add(-1);
}

// Temporarily pauses the thread.
void State::pause() {
  if (tstate == RUNNING) {
    abort = true;
    sig_exit.wait();
  }

  tstate = PAUSED;
}

VIA_NAMESPACE_END
