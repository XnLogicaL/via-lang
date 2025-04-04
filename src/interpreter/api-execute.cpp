// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

// !========================================================================================== |
// ! DO NOT FUZZ THIS FILE! ONLY UNIT TEST AFTER CHECKING FOR THE vl_debug MACRO!              |
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
#define vl_vmerror(message)                                                                        \
  do {                                                                                             \
    __set_error_state(this, message);                                                              \
    goto dispatch;                                                                                 \
  } while (0)

// Macro that throws a fatal error
#define vl_vmfatal(message)                                                                        \
  do {                                                                                             \
    std::cerr << "VM terminated with message: " << message << '\n';                                \
    std::abort();                                                                                  \
  } while (0)

// Macro that completes an execution cycle
#define vl_vmnext()                                                                                \
  do {                                                                                             \
    ++pc;                                                                                          \
    goto dispatch;                                                                                 \
  } while (0)

#define vl_vmdivby0i(divisor)                                                                      \
  if (divisor == 0) {                                                                              \
    vl_vmerror("Division by zero");                                                                \
  }

#define vl_vmdivby0f(divisor)                                                                      \
  if (divisor == 0.0f) {                                                                           \
    vl_vmerror("Division by zero");                                                                \
  }

// ==================================================================================================
// execute.cpp
//
namespace via {

using enum value_type;
using enum opcode;

using namespace impl;

void vm_save_snapshot(state* vl_restrict V) {
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
  for (value_obj* ptr = V->sbp; ptr < V->sbp + V->sp; ptr++) {
    uint32_t pos = ptr - V->sbp;
    stack << "|" << std::setw(2) << std::setfill('0') << pos << "| "
          << impl::__to_cxx_string(V, *ptr) << ' ' << get_raw_memory_dump(ptr, sizeof(value_obj));
  }
  stack << "==== stack ====\n";

  // Generate register map
  for (operand_t reg = 0; reg < vl_regcount; reg++) {
    value_obj* val = impl::__get_register(V, reg);
    if (!val->is_nil()) {
      registers << "|R" << std::setw(2) << std::setfill('0') << reg << "| "
                << impl::__to_cxx_string(V, *val) << ' '
                << get_raw_memory_dump(val, sizeof(value_obj));
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
void state::execute() {
  goto dispatch;

dispatch: {

#ifdef vl_debug
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
    goto exit;
  }

  switch (pc->op) {
  // Handle special/internal opcodes
  case NOP:
  case LABEL:
    vl_vmnext();

  case ADD: {
    operand_t lhs = pc->operand0;
    operand_t rhs = pc->operand1;

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);

    if vl_likely (lhs_val->is_int()) {
      if vl_likely (rhs_val->is_int()) {
        lhs_val->val_integer += rhs_val->val_integer;
      }
      else if vl_unlikely (rhs_val->is_float()) {
        lhs_val->val_floating_point =
          static_cast<TFloat>(lhs_val->val_integer) + rhs_val->val_floating_point;
        lhs_val->type = floating_point;
      }
    }
    else if (lhs_val->is_float()) {
      if vl_likely (rhs_val->is_int()) {
        lhs_val->val_floating_point += static_cast<TFloat>(rhs_val->val_integer);
      }
      else if vl_unlikely (rhs_val->is_float()) {
        lhs_val->val_floating_point += rhs_val->val_floating_point;
      }
    }

    vl_vmnext();
  }
  case ADDK: {
    operand_t lhs = pc->operand0;
    operand_t idx = pc->operand1;

    value_obj* lhs_val = __get_register(this, lhs);
    const value_obj& rhs_val = __get_constant(this, idx);

    if vl_likely (lhs_val->is_int()) {
      if vl_likely (rhs_val.is_int()) {
        lhs_val->val_integer += rhs_val.val_integer;
      }
      else if vl_unlikely (rhs_val.is_float()) {
        lhs_val->val_floating_point =
          static_cast<TFloat>(lhs_val->val_integer) + rhs_val.val_floating_point;
        lhs_val->type = floating_point;
      }
    }
    else if (lhs_val->is_float()) {
      if vl_likely (rhs_val.is_int()) {
        lhs_val->val_floating_point += static_cast<TFloat>(rhs_val.val_integer);
      }
      else if vl_unlikely (rhs_val.is_float()) {
        lhs_val->val_floating_point += rhs_val.val_floating_point;
      }
    }

    vl_vmnext();
  }
  case ADDINT: {
    operand_t lhs = pc->operand0;
    operand_t int_high = pc->operand1;
    operand_t int_low = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    TInteger imm = reinterpret_u16_as_i32(int_high, int_low);

    if vl_likely (lhs_val->is_int()) {
      lhs_val->val_integer += imm;
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point += imm;
    }

    vl_vmnext();
  }
  case ADDFLOAT: {
    operand_t lhs = pc->operand0;
    operand_t flt_high = pc->operand1;
    operand_t flt_low = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    TFloat imm = reinterpret_u16_as_f32(flt_high, flt_low);

    if vl_likely (lhs_val->is_int()) {
      lhs_val->val_integer += imm;
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point += imm;
    }

    vl_vmnext();
  }

  case SUB: {
    operand_t lhs = pc->operand0;
    operand_t rhs = pc->operand1;

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);

    if vl_likely (lhs_val->is_int()) {
      if vl_likely (rhs_val->is_int()) {
        lhs_val->val_integer -= rhs_val->val_integer;
      }
      else if vl_unlikely (rhs_val->is_float()) {
        lhs_val->val_floating_point =
          static_cast<TFloat>(lhs_val->val_integer) - rhs_val->val_floating_point;
        lhs_val->type = floating_point;
      }
    }
    else if (lhs_val->is_float()) {
      if vl_likely (rhs_val->is_int()) {
        lhs_val->val_floating_point -= static_cast<TFloat>(rhs_val->val_integer);
      }
      else if vl_unlikely (rhs_val->is_float()) {
        lhs_val->val_floating_point -= rhs_val->val_floating_point;
      }
    }

    vl_vmnext();
  }
  case SUBK: {
    operand_t lhs = pc->operand0;
    operand_t idx = pc->operand1;

    value_obj* lhs_val = __get_register(this, lhs);
    const value_obj& rhs_val = __get_constant(this, idx);

    if vl_likely (lhs_val->is_int()) {
      if vl_likely (rhs_val.is_int()) {
        lhs_val->val_integer -= rhs_val.val_integer;
      }
      else if vl_unlikely (rhs_val.is_float()) {
        lhs_val->val_floating_point =
          static_cast<TFloat>(lhs_val->val_integer) - rhs_val.val_floating_point;
        lhs_val->type = floating_point;
      }
    }
    else if (lhs_val->is_float()) {
      if vl_likely (rhs_val.is_int()) {
        lhs_val->val_floating_point -= static_cast<TFloat>(rhs_val.val_integer);
      }
      else if vl_unlikely (rhs_val.is_float()) {
        lhs_val->val_floating_point -= rhs_val.val_floating_point;
      }
    }

    vl_vmnext();
  }
  case SUBINT: {
    operand_t lhs = pc->operand0;
    operand_t int_high = pc->operand1;
    operand_t int_low = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    TInteger imm = reinterpret_u16_as_i32(int_high, int_low);

    if vl_likely (lhs_val->is_int()) {
      lhs_val->val_integer -= imm;
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point -= imm;
    }

    vl_vmnext();
  }
  case SUBFLOAT: {
    operand_t lhs = pc->operand0;
    operand_t flt_high = pc->operand1;
    operand_t flt_low = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    TFloat imm = reinterpret_u16_as_f32(flt_high, flt_low);

    if vl_likely (lhs_val->is_int()) {
      lhs_val->val_integer -= imm;
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point -= imm;
    }

    vl_vmnext();
  }

  case MUL: {
    operand_t lhs = pc->operand0;
    operand_t rhs = pc->operand1;

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);

    if vl_likely (lhs_val->is_int()) {
      if vl_likely (rhs_val->is_int()) {
        lhs_val->val_integer *= rhs_val->val_integer;
      }
      else if vl_unlikely (rhs_val->is_float()) {
        lhs_val->val_floating_point =
          static_cast<TFloat>(lhs_val->val_integer) * rhs_val->val_floating_point;
        lhs_val->type = floating_point;
      }
    }
    else if (lhs_val->is_float()) {
      if vl_likely (rhs_val->is_int()) {
        lhs_val->val_floating_point *= static_cast<TFloat>(rhs_val->val_integer);
      }
      else if vl_unlikely (rhs_val->is_float()) {
        lhs_val->val_floating_point *= rhs_val->val_floating_point;
      }
    }

    vl_vmnext();
  }
  case MULK: {
    operand_t lhs = pc->operand0;
    operand_t idx = pc->operand1;

    value_obj* lhs_val = __get_register(this, lhs);
    const value_obj& rhs_val = __get_constant(this, idx);

    if vl_likely (lhs_val->is_int()) {
      if vl_likely (rhs_val.is_int()) {
        lhs_val->val_integer *= rhs_val.val_integer;
      }
      else if vl_unlikely (rhs_val.is_float()) {
        lhs_val->val_floating_point =
          static_cast<TFloat>(lhs_val->val_integer) * rhs_val.val_floating_point;
        lhs_val->type = floating_point;
      }
    }
    else if (lhs_val->is_float()) {
      if vl_likely (rhs_val.is_int()) {
        lhs_val->val_floating_point *= static_cast<TFloat>(rhs_val.val_integer);
      }
      else if vl_unlikely (rhs_val.is_float()) {
        lhs_val->val_floating_point *= rhs_val.val_floating_point;
      }
    }

    vl_vmnext();
  }
  case MULINT: {
    operand_t lhs = pc->operand0;
    operand_t int_high = pc->operand1;
    operand_t int_low = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    TInteger imm = reinterpret_u16_as_i32(int_high, int_low);

    if vl_likely (lhs_val->is_int()) {
      lhs_val->val_integer *= imm;
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point *= imm;
    }

    vl_vmnext();
  }
  case MULFLOAT: {
    operand_t lhs = pc->operand0;
    operand_t flt_high = pc->operand1;
    operand_t flt_low = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    TFloat imm = reinterpret_u16_as_f32(flt_high, flt_low);

    if vl_likely (lhs_val->is_int()) {
      lhs_val->val_integer *= imm;
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point *= imm;
    }

    vl_vmnext();
  }

  case DIV: {
    operand_t lhs = pc->operand0;
    operand_t rhs = pc->operand1;

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);

    if vl_likely (lhs_val->is_int()) {
      if vl_likely (rhs_val->is_int()) {
        vl_vmdivby0i(rhs_val);

        lhs_val->val_integer /= rhs_val->val_integer;
      }
      else if vl_unlikely (rhs_val->is_float()) {
        vl_vmdivby0f(rhs_val->val_floating_point);

        lhs_val->val_floating_point =
          static_cast<TFloat>(lhs_val->val_integer) / rhs_val->val_floating_point;
        lhs_val->type = floating_point;
      }
    }
    else if (lhs_val->is_float()) {
      if vl_likely (rhs_val->is_int()) {
        vl_vmdivby0i(rhs_val);

        lhs_val->val_floating_point /= static_cast<TFloat>(rhs_val->val_integer);
      }
      else if vl_unlikely (rhs_val->is_float()) {
        vl_vmdivby0f(rhs_val->val_floating_point);

        lhs_val->val_floating_point /= rhs_val->val_floating_point;
      }
    }

    vl_vmnext();
  }
  case DIVK: {
    operand_t lhs = pc->operand0;
    operand_t idx = pc->operand1;

    value_obj* lhs_val = __get_register(this, lhs);
    const value_obj& rhs_val = __get_constant(this, idx);

    if vl_likely (lhs_val->is_int()) {
      if vl_likely (rhs_val.is_int()) {
        vl_vmdivby0i(rhs_val.val_integer);

        lhs_val->val_integer /= rhs_val.val_integer;
      }
      else if vl_unlikely (rhs_val.is_float()) {
        vl_vmdivby0f(rhs_val.val_floating_point);

        lhs_val->val_floating_point =
          static_cast<TFloat>(lhs_val->val_integer) / rhs_val.val_floating_point;
        lhs_val->type = floating_point;
      }
    }
    else if (lhs_val->is_float()) {
      if vl_likely (rhs_val.is_int()) {
        vl_vmdivby0i(rhs_val.val_integer);

        lhs_val->val_floating_point /= static_cast<TFloat>(rhs_val.val_integer);
      }
      else if vl_unlikely (rhs_val.is_float()) {
        vl_vmdivby0f(rhs_val.val_floating_point);

        lhs_val->val_floating_point /= rhs_val.val_floating_point;
      }
    }

    vl_vmnext();
  }
  case DIVINT: {
    operand_t lhs = pc->operand0;
    operand_t int_high = pc->operand1;
    operand_t int_low = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    TInteger imm = reinterpret_u16_as_i32(int_high, int_low);

    vl_vmdivby0i(imm);

    if vl_likely (lhs_val->is_int()) {
      lhs_val->val_integer /= imm;
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point /= imm;
    }

    vl_vmnext();
  }
  case DIVFLOAT: {
    operand_t lhs = pc->operand0;
    operand_t flt_high = pc->operand1;
    operand_t flt_low = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    TFloat imm = reinterpret_u16_as_f32(flt_high, flt_low);

    vl_vmdivby0f(imm);

    if vl_likely (lhs_val->is_int()) {
      lhs_val->val_integer /= imm;
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point /= imm;
    }

    vl_vmnext();
  }

  case POW: {
    operand_t lhs = pc->operand0;
    operand_t rhs = pc->operand1;

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);

    if vl_likely (lhs_val->is_int()) {
      if vl_likely (rhs_val->is_int()) {
        lhs_val->val_integer = std::pow(lhs_val->val_integer, rhs_val->val_integer);
      }
      else if vl_unlikely (rhs_val->is_float()) {
        lhs_val->val_floating_point =
          std::pow(static_cast<TFloat>(lhs_val->val_integer), rhs_val->val_floating_point);
        lhs_val->type = floating_point;
      }
    }
    else if (lhs_val->is_float()) {
      if vl_likely (rhs_val->is_int()) {
        lhs_val->val_floating_point =
          std::pow(lhs_val->val_floating_point, static_cast<TFloat>(rhs_val->val_integer));
      }
      else if vl_unlikely (rhs_val->is_float()) {
        lhs_val->val_floating_point =
          std::pow(lhs_val->val_floating_point, rhs_val->val_floating_point);
      }
    }

    vl_vmnext();
  }
  case POWK: {
    operand_t lhs = pc->operand0;
    operand_t idx = pc->operand1;

    value_obj* lhs_val = __get_register(this, lhs);
    const value_obj& rhs_val = __get_constant(this, idx);

    if vl_likely (lhs_val->is_int()) {
      if vl_likely (rhs_val.is_int()) {
        lhs_val->val_integer = std::pow(lhs_val->val_integer, rhs_val.val_integer);
      }
      else if vl_unlikely (rhs_val.is_float()) {
        lhs_val->val_floating_point =
          std::pow(static_cast<TFloat>(lhs_val->val_integer), rhs_val.val_floating_point);
        lhs_val->type = floating_point;
      }
    }
    else if (lhs_val->is_float()) {
      if vl_likely (rhs_val.is_int()) {
        lhs_val->val_floating_point =
          std::pow(lhs_val->val_floating_point, static_cast<TFloat>(rhs_val.val_integer));
      }
      else if vl_unlikely (rhs_val.is_float()) {
        lhs_val->val_floating_point =
          std::pow(lhs_val->val_floating_point, rhs_val.val_floating_point);
      }
    }

    vl_vmnext();
  }
  case POWINT: {
    operand_t lhs = pc->operand0;
    operand_t int_high = pc->operand1;
    operand_t int_low = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    TInteger imm = reinterpret_u16_as_i32(int_high, int_low);

    if vl_likely (lhs_val->is_int()) {
      lhs_val->val_integer = std::pow(lhs_val->val_integer, imm);
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point = std::pow(lhs_val->val_floating_point, imm);
    }

    vl_vmnext();
  }
  case POWFLOAT: {
    operand_t lhs = pc->operand0;
    operand_t flt_high = pc->operand1;
    operand_t flt_low = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    TFloat imm = reinterpret_u16_as_f32(flt_high, flt_low);

    if vl_likely (lhs_val->is_int()) {
      lhs_val->val_integer = std::pow(lhs_val->val_integer, imm);
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point = std::pow(lhs_val->val_floating_point, imm);
    }

    vl_vmnext();
  }

  case MOD: {
    operand_t lhs = pc->operand0;
    operand_t rhs = pc->operand1;

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);

    if vl_likely (lhs_val->is_int()) {
      if vl_likely (rhs_val->is_int()) {
        lhs_val->val_integer %= rhs_val->val_integer;
      }
      else if vl_unlikely (rhs_val->is_float()) {
        lhs_val->val_floating_point =
          std::fmod(static_cast<TFloat>(lhs_val->val_integer), rhs_val->val_floating_point);
        lhs_val->type = floating_point;
      }
    }
    else if (lhs_val->is_float()) {
      if vl_likely (rhs_val->is_int()) {
        lhs_val->val_floating_point =
          std::fmod(lhs_val->val_floating_point, static_cast<TFloat>(rhs_val->val_integer));
      }
      else if vl_unlikely (rhs_val->is_float()) {
        lhs_val->val_floating_point =
          std::fmod(lhs_val->val_floating_point, rhs_val->val_floating_point);
      }
    }

    vl_vmnext();
  }
  case MODK: {
    operand_t lhs = pc->operand0;
    operand_t idx = pc->operand1;

    value_obj* lhs_val = __get_register(this, lhs);
    const value_obj& rhs_val = __get_constant(this, idx);

    if vl_likely (lhs_val->is_int()) {
      if vl_likely (rhs_val.is_int()) {
        lhs_val->val_integer %= rhs_val.val_integer;
      }
      else if vl_unlikely (rhs_val.is_float()) {
        lhs_val->val_floating_point =
          std::fmod(static_cast<TFloat>(lhs_val->val_integer), rhs_val.val_floating_point);
        lhs_val->type = floating_point;
      }
    }
    else if (lhs_val->is_float()) {
      if vl_likely (rhs_val.is_int()) {
        lhs_val->val_floating_point =
          std::fmod(lhs_val->val_floating_point, static_cast<TFloat>(rhs_val.val_integer));
      }
      else if vl_unlikely (rhs_val.is_float()) {
        lhs_val->val_floating_point =
          std::fmod(lhs_val->val_floating_point, rhs_val.val_floating_point);
      }
    }

    vl_vmnext();
  }
  case MODINT: {
    operand_t lhs = pc->operand0;
    operand_t int_high = pc->operand1;
    operand_t int_low = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    TInteger imm = reinterpret_u16_as_i32(int_high, int_low);



    if vl_likely (lhs_val->is_int()) {
      lhs_val->val_integer = std::fmod(lhs_val->val_integer, imm);
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point = std::fmod(lhs_val->val_floating_point, imm);
    }

    vl_vmnext();
  }
  case MODFLOAT: {
    operand_t lhs = pc->operand0;
    operand_t flt_high = pc->operand1;
    operand_t flt_low = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    TFloat imm = reinterpret_u16_as_f32(flt_high, flt_low);



    if vl_likely (lhs_val->is_int()) {
      lhs_val->val_integer = std::fmod(lhs_val->val_integer, imm);
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point = std::fmod(lhs_val->val_floating_point, imm);
    }

    vl_vmnext();
  }

  case NEG: {
    operand_t dst = pc->operand0;
    value_obj* val = __get_register(this, dst);
    value_type type = val->type;

    if (type == integer) {
      val->val_integer = -val->val_integer;
    }
    else if (type == floating_point) {
      val->val_floating_point = -val->val_floating_point;
    }

    vl_vmnext();
  }

  case MOVE: {
    operand_t rdst = pc->operand0;
    operand_t rsrc = pc->operand1;
    value_obj* src_val = __get_register(this, rsrc);

    __set_register(this, rdst, *src_val);
    vl_vmnext();
  }

  case LOADK: {
    operand_t dst = pc->operand0;
    operand_t idx = pc->operand1;

    const value_obj& kval = __get_constant(this, idx);

    __set_register(this, dst, kval);
    vl_vmnext();
  }

  case LOADNIL: {
    operand_t dst = pc->operand0;

    __set_register(this, dst, _Nil);
    vl_vmnext();
  }

  case LOADINT: {
    operand_t dst = pc->operand0;
    TInteger imm = reinterpret_u16_as_u32(pc->operand1, pc->operand2);

    __set_register(this, dst, value_obj(imm));
    vl_vmnext();
  }

  case LOADFLOAT: {
    operand_t dst = pc->operand0;
    TFloat imm = reinterpret_u16_as_f32(pc->operand1, pc->operand2);

    __set_register(this, dst, value_obj(imm));
    vl_vmnext();
  }

  case LOADTRUE: {
    operand_t dst = pc->operand0;
    __set_register(this, dst, value_obj(true));
    vl_vmnext();
  }

  case LOADFALSE: {
    operand_t dst = pc->operand0;
    __set_register(this, dst, value_obj(false));
    vl_vmnext();
  }

  case NEWTABLE: {
    operand_t dst = pc->operand0;
    value_obj ttable(new table_obj());

    __set_register(this, dst, ttable);
    vl_vmnext();
  }

  case NEWCLOSURE: {
    operand_t dst = pc->operand0;
    function_obj* func = new function_obj();

    __closure_bytecode_load(this, func);
    __set_register(this, dst, value_obj(function, func));
    vl_vmnext();
  }

  case GETUPVALUE: {
    operand_t dst = pc->operand0;
    operand_t upv_id = pc->operand1;
    upv_obj* upv = __closure_upv_get(frame, upv_id);

    dump_struct(*upv->value);

    __set_register(this, dst, *upv->value);
    vl_vmnext();
  }

  case SETUPVALUE: {
    operand_t src = pc->operand0;
    operand_t upv_id = pc->operand1;
    value_obj* val = __get_register(this, src);

    __closure_upv_set(frame, upv_id, *val);
    vl_vmnext();
  }

  case PUSH: {
    operand_t src = pc->operand0;
    value_obj* val = __get_register(this, src);

    __push(this, *val);
    vl_vmnext();
  }

  case PUSHK: {
    operand_t const_idx = pc->operand0;
    value_obj constant = __get_constant(this, const_idx);

    __push(this, constant);
    vl_vmnext();
  }

  case PUSHNIL: {
    __push(this, value_obj());
    vl_vmnext();
  }

  case PUSHINT: {
    TInteger imm = reinterpret_u16_as_u32(pc->operand0, pc->operand1);
    __push(this, value_obj(imm));
    vl_vmnext();
  }

  case PUSHFLOAT: {
    TFloat imm = reinterpret_u16_as_f32(pc->operand0, pc->operand1);
    __push(this, value_obj(imm));
    vl_vmnext();
  }

  case PUSHTRUE: {
    __push(this, value_obj(true));
    vl_vmnext();
  }

  case PUSHFALSE: {
    __push(this, value_obj(false));
    vl_vmnext();
  }

  case POP: {
    operand_t dst = pc->operand0;
    value_obj val = __pop(this);

    __set_register(this, dst, val);
    vl_vmnext();
  }

  case DROP: {
    __pop(this);
    vl_vmnext();
  }

  case GETSTACK: {
    operand_t dst = pc->operand0;
    operand_t off = pc->operand1;

    const value_obj& val = __get_stack(this, off);

    __set_register(this, dst, val);
    vl_vmnext();
  }

  case SETSTACK: {
    operand_t src = pc->operand0;
    operand_t off = pc->operand1;

    value_obj* val = __get_register(this, src);

    sbp[off] = std::move(*val);
    vl_vmnext();
  }

  case GETARGUMENT: {
    operand_t dst = pc->operand0;
    operand_t off = pc->operand1;

    const value_obj& val = __get_argument(this, off);

    __set_register(this, dst, val);
    vl_vmnext();
  }

  case GETGLOBAL: {
    operand_t dst = pc->operand0;
    operand_t key = pc->operand1;

    const value_obj& global = glb->gtable.get(key);

    __set_register(this, dst, global);
    vl_vmnext();
  }

  case SETGLOBAL: {
    operand_t src = pc->operand0;
    operand_t key = pc->operand1;

    value_obj* global = __get_register(this, src);

    glb->gtable.set(key, *global);
    vl_vmnext();
  }

  case EQUAL: {
    operand_t dst = pc->operand0;
    operand_t lhs = pc->operand1;
    operand_t rhs = pc->operand2;

    if vl_unlikely (lhs == rhs) {
      __set_register(this, dst, value_obj(true));
      vl_vmnext();
    }

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);

    if vl_unlikely (lhs_val == rhs_val) {
      __set_register(this, dst, value_obj(true));
      vl_vmnext();
    }

    bool result = __compare(*lhs_val, *rhs_val);
    __set_register(this, dst, value_obj(result));

    vl_vmnext();
  }

  case NOTEQUAL: {
    operand_t dst = pc->operand0;
    operand_t lhs = pc->operand1;
    operand_t rhs = pc->operand2;

    if vl_likely (lhs != rhs) {
      __set_register(this, dst, value_obj(true));
      vl_vmnext();
    }

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);

    if vl_likely (lhs_val != rhs_val) {
      __set_register(this, dst, value_obj(true));
      vl_vmnext();
    }

    bool result = __compare(*lhs_val, *rhs_val);
    __set_register(this, dst, value_obj(result));

    vl_vmnext();
  }

  case AND: {
    operand_t dst = pc->operand0;
    operand_t lhs = pc->operand1;
    operand_t rhs = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);
    bool cond = __to_cxx_bool(*lhs_val) && __to_cxx_bool(*rhs_val);

    __set_register(this, dst, value_obj(cond));
    vl_vmnext();
  }

  case OR: {
    operand_t dst = pc->operand0;
    operand_t lhs = pc->operand1;
    operand_t rhs = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);
    bool cond = __to_cxx_bool(*lhs_val) || __to_cxx_bool(*rhs_val);

    __set_register(this, dst, value_obj(cond));
    vl_vmnext();
  }

  case NOT: {
    operand_t dst = pc->operand0;
    operand_t lhs = pc->operand1;

    value_obj* lhs_val = __get_register(this, lhs);
    bool cond = !__to_cxx_bool(*lhs_val);

    __set_register(this, dst, value_obj(cond));
    vl_vmnext();
  }

  case LESS: {
    operand_t dst = pc->operand0;
    operand_t lhs = pc->operand1;
    operand_t rhs = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);

    if vl_likely (lhs_val->is_int()) {
      if vl_likely (rhs_val->is_int()) {
        __set_register(this, dst, value_obj(lhs_val->val_integer < rhs_val->val_integer));
      }
      else if vl_unlikely (rhs_val->is_float()) {
        __set_register(
          this,
          dst,
          value_obj(static_cast<TFloat>(lhs_val->val_integer) < rhs_val->val_floating_point)
        );
      }
    }
    else if vl_unlikely (lhs_val->is_float()) {
      if vl_likely (rhs_val->is_int()) {
        __set_register(
          this,
          dst,
          value_obj(lhs_val->val_floating_point < static_cast<TFloat>(rhs_val->val_integer))
        );
      }
      else if vl_unlikely (rhs_val->is_float()) {
        __set_register(
          this, dst, value_obj(lhs_val->val_floating_point < rhs_val->val_floating_point)
        );
      }
    }

    vl_vmnext();
  }

  case GREATER: {
    operand_t dst = pc->operand0;
    operand_t lhs = pc->operand1;
    operand_t rhs = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);

    if vl_likely (lhs_val->is_int()) {
      if vl_likely (rhs_val->is_int()) {
        __set_register(this, dst, value_obj(lhs_val->val_integer > rhs_val->val_integer));
      }
      else if vl_unlikely (rhs_val->is_float()) {
        __set_register(
          this,
          dst,
          value_obj(static_cast<TFloat>(lhs_val->val_integer) > rhs_val->val_floating_point)
        );
      }
    }
    else if vl_unlikely (lhs_val->is_float()) {
      if vl_likely (rhs_val->is_int()) {
        __set_register(
          this,
          dst,
          value_obj(lhs_val->val_floating_point > static_cast<TFloat>(rhs_val->val_integer))
        );
      }
      else if vl_unlikely (rhs_val->is_float()) {
        __set_register(
          this, dst, value_obj(lhs_val->val_floating_point > rhs_val->val_floating_point)
        );
      }
    }

    vl_vmnext();
  }

  case LESSOREQUAL: {
    operand_t dst = pc->operand0;
    operand_t lhs = pc->operand1;
    operand_t rhs = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);

    if vl_likely (lhs_val->is_int()) {
      if vl_likely (rhs_val->is_int()) {
        __set_register(this, dst, value_obj(lhs_val->val_integer <= rhs_val->val_integer));
      }
      else if vl_unlikely (rhs_val->is_float()) {
        __set_register(
          this,
          dst,
          value_obj(static_cast<TFloat>(lhs_val->val_integer) <= rhs_val->val_floating_point)
        );
      }
    }
    else if vl_unlikely (lhs_val->is_float()) {
      if vl_likely (rhs_val->is_int()) {
        __set_register(
          this,
          dst,
          value_obj(lhs_val->val_floating_point <= static_cast<TFloat>(rhs_val->val_integer))
        );
      }
      else if vl_unlikely (rhs_val->is_float()) {
        __set_register(
          this, dst, value_obj(lhs_val->val_floating_point <= rhs_val->val_floating_point)
        );
      }
    }

    vl_vmnext();
  }

  case GREATEROREQUAL: {
    operand_t dst = pc->operand0;
    operand_t lhs = pc->operand1;
    operand_t rhs = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);

    if vl_likely (lhs_val->is_int()) {
      if vl_likely (rhs_val->is_int()) {
        __set_register(this, dst, value_obj(lhs_val->val_integer >= rhs_val->val_integer));
      }
      else if vl_unlikely (rhs_val->is_float()) {
        __set_register(
          this,
          dst,
          value_obj(static_cast<TFloat>(lhs_val->val_integer) >= rhs_val->val_floating_point)
        );
      }
    }
    else if vl_unlikely (lhs_val->is_float()) {
      if vl_likely (rhs_val->is_int()) {
        __set_register(
          this,
          dst,
          value_obj(lhs_val->val_floating_point >= static_cast<TFloat>(rhs_val->val_integer))
        );
      }
      else if vl_unlikely (rhs_val->is_float()) {
        __set_register(
          this, dst, value_obj(lhs_val->val_floating_point >= rhs_val->val_floating_point)
        );
      }
    }

    vl_vmnext();
  }

  case EXIT: {
    goto exit;
  }

  case JUMP: {
    signed_operand_t offset = pc->operand0;
    pc += offset;
    goto dispatch;
  }

  case JUMPIF: {
    operand_t cond = pc->operand0;
    signed_operand_t offset = pc->operand1;

    value_obj* cond_val = __get_register(this, cond);
    if (__to_cxx_bool(*cond_val)) {
      pc += offset;
    }

    goto dispatch;
  }

  case JUMPIFNOT: {
    operand_t cond = pc->operand0;
    signed_operand_t offset = pc->operand1;

    value_obj* cond_val = __get_register(this, cond);
    if (!__to_cxx_bool(*cond_val)) {
      pc += offset;
    }

    goto dispatch;
  }

  case JUMPIFEQUAL: {
    operand_t cond_lhs = pc->operand0;
    operand_t cond_rhs = pc->operand1;
    signed_operand_t offset = pc->operand2;

    if vl_unlikely (cond_lhs == cond_rhs) {
      pc += offset;
    }
    else {
      value_obj* lhs_val = __get_register(this, cond_lhs);
      value_obj* rhs_val = __get_register(this, cond_rhs);

      if vl_unlikely (lhs_val == rhs_val || __compare(*lhs_val, *rhs_val)) {
        pc += offset;
      }
    }

    goto dispatch;
  }

  case JUMPIFNOTEQUAL: {
    operand_t cond_lhs = pc->operand0;
    operand_t cond_rhs = pc->operand1;
    signed_operand_t offset = pc->operand2;

    if vl_likely (cond_lhs != cond_rhs) {
      pc += offset;
    }
    else {
      value_obj* lhs_val = __get_register(this, cond_lhs);
      value_obj* rhs_val = __get_register(this, cond_rhs);

      if vl_likely (lhs_val != rhs_val || !__compare(*lhs_val, *rhs_val)) {
        pc += offset;
      }
    }

    goto dispatch;
  }

  case JUMPIFLESS: {
    operand_t cond_lhs = pc->operand0;
    operand_t cond_rhs = pc->operand1;
    signed_operand_t offset = pc->operand2;

    value_obj* lhs_val = __get_register(this, cond_lhs);
    value_obj* rhs_val = __get_register(this, cond_rhs);

    if vl_likely (lhs_val->is_int()) {
      if vl_likely (rhs_val->is_int()) {
        if (lhs_val->val_integer < rhs_val->val_integer) {
          pc += offset;
        }
      }
      else if vl_unlikely (rhs_val->is_float()) {
        if (static_cast<TFloat>(lhs_val->val_integer) < rhs_val->val_floating_point) {
          pc += offset;
        }
      }
    }
    else if vl_unlikely (lhs_val->is_float()) {
      if vl_likely (rhs_val->is_int()) {
        if (lhs_val->val_floating_point < static_cast<TFloat>(rhs_val->val_integer)) {
          pc += offset;
        }
      }
      else if vl_unlikely (rhs_val->is_float()) {
        if (lhs_val->val_floating_point < rhs_val->val_floating_point) {
          pc += offset;
        }
      }
    }

    goto dispatch;
  }

  case JUMPIFGREATER: {
    operand_t cond_lhs = pc->operand0;
    operand_t cond_rhs = pc->operand1;
    signed_operand_t offset = pc->operand2;

    value_obj* lhs_val = __get_register(this, cond_lhs);
    value_obj* rhs_val = __get_register(this, cond_rhs);

    if vl_likely (lhs_val->is_int()) {
      if vl_likely (rhs_val->is_int()) {
        if (lhs_val->val_integer > rhs_val->val_integer) {
          pc += offset;
        }
      }
      else if vl_unlikely (rhs_val->is_float()) {
        if (static_cast<TFloat>(lhs_val->val_integer) > rhs_val->val_floating_point) {
          pc += offset;
        }
      }
    }
    else if vl_unlikely (lhs_val->is_float()) {
      if vl_likely (rhs_val->is_int()) {
        if (lhs_val->val_floating_point > static_cast<TFloat>(rhs_val->val_integer)) {
          pc += offset;
        }
      }
      else if vl_unlikely (rhs_val->is_float()) {
        if (lhs_val->val_floating_point > rhs_val->val_floating_point) {
          pc += offset;
        }
      }
    }

    goto dispatch;
  }

  case JUMPIFLESSOREQUAL: {
    operand_t cond_lhs = pc->operand0;
    operand_t cond_rhs = pc->operand1;
    signed_operand_t offset = pc->operand2;

    value_obj* lhs_val = __get_register(this, cond_lhs);
    value_obj* rhs_val = __get_register(this, cond_rhs);

    if vl_likely (lhs_val->is_int()) {
      if vl_likely (rhs_val->is_int()) {
        if (lhs_val->val_integer <= rhs_val->val_integer) {
          pc += offset;
        }
      }
      else if vl_unlikely (rhs_val->is_float()) {
        if (static_cast<TFloat>(lhs_val->val_integer) <= rhs_val->val_floating_point) {
          pc += offset;
        }
      }
    }
    else if vl_unlikely (lhs_val->is_float()) {
      if vl_likely (rhs_val->is_int()) {
        if (lhs_val->val_floating_point <= static_cast<TFloat>(rhs_val->val_integer)) {
          pc += offset;
        }
      }
      else if vl_unlikely (rhs_val->is_float()) {
        if (lhs_val->val_floating_point <= rhs_val->val_floating_point) {
          pc += offset;
        }
      }
    }

    goto dispatch;
  }

  case JUMPIFGREATEROREQUAL: {
    operand_t cond_lhs = pc->operand0;
    operand_t cond_rhs = pc->operand1;
    signed_operand_t offset = pc->operand2;

    value_obj* lhs_val = __get_register(this, cond_lhs);
    value_obj* rhs_val = __get_register(this, cond_rhs);

    if vl_likely (lhs_val->is_int()) {
      if vl_likely (rhs_val->is_int()) {
        if (lhs_val->val_integer >= rhs_val->val_integer) {
          pc += offset;
        }
      }
      else if vl_unlikely (rhs_val->is_float()) {
        if (static_cast<TFloat>(lhs_val->val_integer) >= rhs_val->val_floating_point) {
          pc += offset;
        }
      }
    }
    else if vl_unlikely (lhs_val->is_float()) {
      if vl_likely (rhs_val->is_int()) {
        if (lhs_val->val_floating_point >= static_cast<TFloat>(rhs_val->val_integer)) {
          pc += offset;
        }
      }
      else if vl_unlikely (rhs_val->is_float()) {
        if (lhs_val->val_floating_point >= rhs_val->val_floating_point) {
          pc += offset;
        }
      }
    }

    goto dispatch;
  }

  case JUMPLABEL: {
    operand_t label = pc->operand0;

    pc = __label_get(this, label);

    goto dispatch;
  }

  case JUMPLABELIF: {
    operand_t cond = pc->operand0;
    operand_t label = pc->operand1;

    value_obj* cond_val = __get_register(this, cond);
    if (__to_cxx_bool(*cond_val)) {
      pc = __label_get(this, label);
    }

    goto dispatch;
  }

  case JUMPLABELIFNOT: {
    operand_t cond = pc->operand0;
    operand_t label = pc->operand1;

    value_obj* cond_val = __get_register(this, cond);
    if (!__to_cxx_bool(*cond_val)) {
      pc = __label_get(this, label);
    }

    goto dispatch;
  }

  case JUMPLABELIFEQUAL: {
    operand_t cond_lhs = pc->operand0;
    operand_t cond_rhs = pc->operand1;
    operand_t label = pc->operand2;

    if vl_unlikely (cond_lhs == cond_rhs) {
      pc = __label_get(this, label);
    }
    else {
      value_obj* lhs_val = __get_register(this, cond_lhs);
      value_obj* rhs_val = __get_register(this, cond_rhs);

      if vl_unlikely (lhs_val == rhs_val || __compare(*lhs_val, *rhs_val)) {
        pc = __label_get(this, label);
      }
    }

    goto dispatch;
  }

  case JUMPLABELIFNOTEQUAL: {
    operand_t cond_lhs = pc->operand0;
    operand_t cond_rhs = pc->operand1;
    operand_t label = pc->operand2;

    if vl_likely (cond_lhs != cond_rhs) {
      pc = __label_get(this, label);
    }
    else {
      value_obj* lhs_val = __get_register(this, cond_lhs);
      value_obj* rhs_val = __get_register(this, cond_rhs);

      if vl_likely (lhs_val != rhs_val || !__compare(*lhs_val, *rhs_val)) {
        pc = __label_get(this, label);
      }
    }

    goto dispatch;
  }

  case JUMPLABELIFLESS: {
    operand_t cond_lhs = pc->operand0;
    operand_t cond_rhs = pc->operand1;
    operand_t label = pc->operand2;

    value_obj* lhs_val = __get_register(this, cond_lhs);
    value_obj* rhs_val = __get_register(this, cond_rhs);

    if vl_likely (lhs_val->is_int()) {
      if vl_likely (rhs_val->is_int()) {
        if (lhs_val->val_integer < rhs_val->val_integer) {
          pc = __label_get(this, label);
        }
      }
      else if vl_unlikely (rhs_val->is_float()) {
        if (static_cast<TFloat>(lhs_val->val_integer) < rhs_val->val_floating_point) {
          pc = __label_get(this, label);
        }
      }
    }
    else if vl_unlikely (lhs_val->is_float()) {
      if vl_likely (rhs_val->is_int()) {
        if (lhs_val->val_floating_point < static_cast<TFloat>(rhs_val->val_integer)) {
          pc = __label_get(this, label);
        }
      }
      else if vl_unlikely (rhs_val->is_float()) {
        if (lhs_val->val_floating_point < rhs_val->val_floating_point) {
          pc = __label_get(this, label);
        }
      }
    }

    goto dispatch;
  }

  case JUMPLABELIFGREATER: {
    operand_t cond_lhs = pc->operand0;
    operand_t cond_rhs = pc->operand1;
    operand_t label = pc->operand2;

    value_obj* lhs_val = __get_register(this, cond_lhs);
    value_obj* rhs_val = __get_register(this, cond_rhs);

    if vl_likely (lhs_val->is_int()) {
      if vl_likely (rhs_val->is_int()) {
        if (lhs_val->val_integer > rhs_val->val_integer) {
          pc = __label_get(this, label);
        }
      }
      else if vl_unlikely (rhs_val->is_float()) {
        if (static_cast<TFloat>(lhs_val->val_integer) > rhs_val->val_floating_point) {
          pc = __label_get(this, label);
        }
      }
    }
    else if vl_unlikely (lhs_val->is_float()) {
      if vl_likely (rhs_val->is_int()) {
        if (lhs_val->val_floating_point > static_cast<TFloat>(rhs_val->val_integer)) {
          pc = __label_get(this, label);
        }
      }
      else if vl_unlikely (rhs_val->is_float()) {
        if (lhs_val->val_floating_point > rhs_val->val_floating_point) {
          pc = __label_get(this, label);
        }
      }
    }

    goto dispatch;
  }

  case JUMPLABELIFLESSOREQUAL: {
    operand_t cond_lhs = pc->operand0;
    operand_t cond_rhs = pc->operand1;
    operand_t label = pc->operand2;

    value_obj* lhs_val = __get_register(this, cond_lhs);
    value_obj* rhs_val = __get_register(this, cond_rhs);

    if vl_likely (lhs_val->is_int()) {
      if vl_likely (rhs_val->is_int()) {
        if (lhs_val->val_integer <= rhs_val->val_integer) {
          pc = __label_get(this, label);
        }
      }
      else if vl_unlikely (rhs_val->is_float()) {
        if (static_cast<TFloat>(lhs_val->val_integer) <= rhs_val->val_floating_point) {
          pc = __label_get(this, label);
        }
      }
    }
    else if vl_unlikely (lhs_val->is_float()) {
      if vl_likely (rhs_val->is_int()) {
        if (lhs_val->val_floating_point <= static_cast<TFloat>(rhs_val->val_integer)) {
          pc = __label_get(this, label);
        }
      }
      else if vl_unlikely (rhs_val->is_float()) {
        if (lhs_val->val_floating_point <= rhs_val->val_floating_point) {
          pc = __label_get(this, label);
        }
      }
    }

    goto dispatch;
  }

  case JUMPLABELIFGREATEROREQUAL: {
    operand_t cond_lhs = pc->operand0;
    operand_t cond_rhs = pc->operand1;
    operand_t label = pc->operand2;

    value_obj* lhs_val = __get_register(this, cond_lhs);
    value_obj* rhs_val = __get_register(this, cond_rhs);

    if vl_likely (lhs_val->is_int()) {
      if vl_likely (rhs_val->is_int()) {
        if (lhs_val->val_integer >= rhs_val->val_integer) {
          pc = __label_get(this, label);
        }
      }
      else if vl_unlikely (rhs_val->is_float()) {
        if (static_cast<TFloat>(lhs_val->val_integer) >= rhs_val->val_floating_point) {
          pc = __label_get(this, label);
        }
      }
    }
    else if vl_unlikely (lhs_val->is_float()) {
      if vl_likely (rhs_val->is_int()) {
        if (lhs_val->val_floating_point >= static_cast<TFloat>(rhs_val->val_integer)) {
          pc = __label_get(this, label);
        }
      }
      else if vl_unlikely (rhs_val->is_float()) {
        if (lhs_val->val_floating_point >= rhs_val->val_floating_point) {
          pc = __label_get(this, label);
        }
      }
    }

    goto dispatch;
  }

  case CALL: {
    operand_t fn = pc->operand0;
    operand_t argc = pc->operand1;
    value_obj* fn_val = __get_register(this, fn);

    __call(this, *fn_val, argc);
    vl_vmnext();
  }

  case EXTERNCALL: {
    operand_t fn = pc->operand0;
    operand_t argc = pc->operand1;
    value_obj* cfunc = __get_register(this, fn);

    __extern_call(this, *cfunc, argc);
    vl_vmnext();
  }

  case NATIVECALL: {
    operand_t fn = pc->operand0;
    operand_t argc = pc->operand1;
    value_obj* func = __get_register(this, fn);

    __native_call(this, func->cast_ptr<function_obj>(), argc);
    vl_vmnext();
  }

  case METHODCALL: {
    operand_t obj = pc->operand0;
    operand_t fn = pc->operand1;
    operand_t argc = pc->operand2;

    value_obj* func = __get_register(this, fn);
    value_obj* object = __get_register(this, obj);

    __push(this, object->clone());
    __native_call(this, func->cast_ptr<function_obj>(), argc + 1);
    vl_vmnext();
  }

  case RETURN: {
    operand_t src = pc->operand0;
    value_obj* val = __get_register(this, src);

    __closure_close_upvalues(frame);
    __native_return(this, *val);
    vl_vmnext();
  }

  case GETTABLE: {
    operand_t dst = pc->operand0;
    operand_t tbl = pc->operand1;
    operand_t key = pc->operand2;

    value_obj* tbl_val = __get_register(this, tbl);
    value_obj* key_val = __get_register(this, key);

    const value_obj& index = __table_get(tbl_val->cast_ptr<table_obj>(), *key_val);

    __set_register(this, dst, index);
    vl_vmnext();
  }

  case SETTABLE: {
    operand_t src = pc->operand0;
    operand_t tbl = pc->operand1;
    operand_t ky = pc->operand2;

    value_obj* table = __get_register(this, tbl);
    value_obj* value = __get_register(this, src);
    value_obj* key = __get_register(this, ky);

    __table_set(table->cast_ptr<table_obj>(), value_obj(key), *value);
    vl_vmnext();
  }

  case NEXTTABLE: {
    static std::unordered_map<void*, operand_t> next_table;

    operand_t dst = pc->operand0;
    operand_t valr = pc->operand1;

    value_obj* val = __get_register(this, valr);
    void* ptr = __to_pointer(*val);
    operand_t key = 0;

    auto it = next_table.find(ptr);
    if (it != next_table.end()) {
      key = ++it->second;
    }
    else {
      next_table[ptr] = 0;
    }

    const value_obj& field = __table_get(val->cast_ptr<table_obj>(), value_obj(key));
    __set_register(this, dst, field);
    vl_vmnext();
  }

  case LENTABLE: {
    operand_t dst = pc->operand0;
    operand_t tbl = pc->operand1;

    value_obj* val = __get_register(this, tbl);
    TInteger size = __table_size(val->cast_ptr<table_obj>());

    __set_register(this, dst, value_obj(size));
    vl_vmnext();
  }

  case LENSTRING: {
    operand_t rdst = pc->operand0;
    operand_t objr = pc->operand1;

    value_obj* val = __get_register(this, objr);
    TInteger len = val->cast_ptr<string_obj>()->len;

    __set_register(this, rdst, value_obj(len));
    vl_vmnext();
  }

  case CONCAT: {
    operand_t left = pc->operand0;
    operand_t right = pc->operand1;

    value_obj* left_val = __get_register(this, left);
    value_obj* right_val = __get_register(this, right);

    string_obj* left_str = left_val->cast_ptr<string_obj>();
    string_obj* right_str = right_val->cast_ptr<string_obj>();

    size_t new_length = left_str->len + right_str->len;
    char* new_string = new char[new_length + 1];

    std::memcpy(new_string, left_str->data, left_str->len);
    std::memcpy(new_string + left_str->len, right_str->data, right_str->len);

    string_obj* new_str = new string_obj(this, new_string);

    __set_register(this, left, value_obj(string, new_str));

    delete[] new_string;

    vl_vmnext();
  }

  case GETSTRING: {
    operand_t dst = pc->operand0;
    operand_t str = pc->operand1;
    operand_t idx = pc->operand2;

    value_obj* str_val = __get_register(this, str);
    string_obj* tstr = str_val->cast_ptr<string_obj>();

    char chr = tstr->data[idx];
    string_obj* result = new string_obj(this, &chr);

    __set_register(this, dst, value_obj(string, result));
    vl_vmnext();
  }

  case SETSTRING: {
    operand_t str = pc->operand0;
    operand_t src = pc->operand1;
    operand_t idx = pc->operand2;

    value_obj* str_val = __get_register(this, str);
    string_obj* tstr = str_val->cast_ptr<string_obj>();

    char chr = static_cast<char>(src);
    char* str_cpy = duplicate_string(tstr->data);
    str_cpy[idx] = chr;

    string_obj* result = new string_obj(this, str_cpy);
    __set_register(this, str, value_obj(result));

    delete[] str_cpy;
    vl_vmnext();
  }

  case INTCAST: {
    operand_t dst = pc->operand0;
    operand_t src = pc->operand1;

    value_obj* target = __get_register(this, src);
    value_obj result = __to_int(this, *target);

    __set_register(this, dst, result);
    vl_vmnext();
  }

  case FLOATCAST: {
    operand_t dst = pc->operand0;
    operand_t src = pc->operand1;

    value_obj* target = __get_register(this, src);
    value_obj result = __to_float(this, *target);

    __set_register(this, dst, result);
    vl_vmnext();
  }

  case STRINGCAST: {
    operand_t dst = pc->operand0;
    operand_t src = pc->operand1;

    value_obj* target = __get_register(this, src);
    value_obj result = __to_string(this, *target);

    __set_register(this, dst, result);
    vl_vmnext();
  }

  case BOOLCAST: {
    operand_t dst = pc->operand0;
    operand_t src = pc->operand1;

    value_obj* target = __get_register(this, src);
    value_obj result = __to_bool(*target);

    __set_register(this, dst, result);
    vl_vmnext();
  }

  default: {
    vl_vmfatal(std::format("unknown opcode 0x{:x}", static_cast<int>(pc->op)));
  }
  }
}

exit:;
}

} // namespace via
