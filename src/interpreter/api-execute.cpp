// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

// !========================================================================================== |
// ! DO NOT FUZZ THIS FILE! ONLY UNIT TEST AFTER CHECKING FOR THE VIA_DEBUG MACRO!              |
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
#define VIA_VMERROR(message)                                                                       \
  do {                                                                                             \
    __set_error_state(this, message);                                                              \
    goto dispatch;                                                                                 \
  } while (0)

// Macro that throws a fatal error
#define VIA_VMFATAL(message)                                                                       \
  do {                                                                                             \
    std::cerr << "VM terminated with message: " << message << '\n';                                \
    std::abort();                                                                                  \
  } while (0)

// Macro that completes an execution cycle
#define VIA_VMNEXT()                                                                               \
  do {                                                                                             \
    ++pc;                                                                                          \
    goto dispatch;                                                                                 \
  } while (0)

#define VIA_VMDIVBY0I(divisor)                                                                     \
  if (divisor == 0) {                                                                              \
    VIA_VMERROR("Division by zero");                                                               \
  }

#define VIA_VMDIVBY0F(divisor)                                                                     \
  if (divisor == 0.0f) {                                                                           \
    VIA_VMERROR("Division by zero");                                                               \
  }

// ==================================================================================================
// execute.cpp
//
namespace via {

using enum value_type;
using enum opcode;

using namespace impl;

void vm_save_snapshot(state* VIA_RESTRICT V) {
  // Calculate the current program counter position relative to the instruction base.
  uint64_t pos = V->pc - V->ibp;
  std::string filename = std::format("vm_snapshot.{}.log", pos);
  std::string filepath = std::format("./__viacache__/{}", filename);

  // Build state information about the current state.
  std::ostringstream state_info;
  state_info << "==== VM State ====\n";
  state_info << "Program Counter (PC): " << static_cast<const void*>(V->pc) << " -> "
             << (V->pc - V->ibp) << "\n";
  state_info << "Stack Pointer (SP): " << static_cast<const void*>(V->sbp + V->sp) << " -> "
             << V->sp << "\n";
  state_info << "Stack Base Pointer (SBP): " << static_cast<const void*>(V->sbp) << "\n";
  state_info << "Instruction Base Pointer (IBP): " << static_cast<const void*>(V->ibp) << "\n";
  state_info << "==== End of VM State ====\n\n";

  // Build header information about the current instruction.
  std::ostringstream headers;
  headers << "==== Current Instruction ====\n";
  headers << "Opcode: " << magic_enum::enum_name(V->pc->op) << "\n";
  headers << "Operand0: " << V->pc->operand0 << ", "
          << "Operand1: " << V->pc->operand1 << ", "
          << "Operand2: " << V->pc->operand2 << "\n";
  headers << "==== End of Instruction ====\n\n";

  // Generate stack map.
  std::ostringstream stack;
  stack << "==== Stack ====\n";
  for (value_obj* ptr = V->sbp; ptr < V->sbp + V->sp; ++ptr) {
    size_t index = ptr - V->sbp;
    // Format the stack index as two-digit with leading zeros.
    stack << "Stack[" << std::setw(2) << std::setfill('0') << index
          << "] = " << impl::__to_cxx_string(*ptr) << "\n";
  }
  stack << "==== End of Stack ====\n\n";

  // Generate register map.
  std::ostringstream registers;
  registers << "==== Registers ====\n";
  for (operand_t reg = 0; reg < VIA_REGCOUNT; ++reg) {
    value_obj* val = impl::__get_register(V, reg);
    if (!val->is_nil()) {
      registers << "R" << std::setw(2) << std::setfill('0') << reg << " = "
                << impl::__to_cxx_string(*val) << "\n";
    }
  }
  registers << "==== End of Registers ====\n\n";

  // Combine all parts.
  std::ostringstream output;
  output << state_info.str() << headers.str() << stack.str() << registers.str();

  // Write to file.
  if (!utils::write_to_file(filepath, output.str())) {
    // Log error or take alternative action.
    std::cerr << "Failed to write snapshot to file: " << filepath << "\n";
  }
}

// Starts VM execution cycle by altering it's state and "iterating" over
// the instruction pipeline.
void state::execute() {
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
    goto exit;
  }

  switch (pc->op) {
  // Handle special/internal opcodes
  case NOP:
  case LBL:
    VIA_VMNEXT();

  case ADD: {
    operand_t lhs = pc->operand0;
    operand_t rhs = pc->operand1;

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);

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

    VIA_VMNEXT();
  }
  case ADDK: {
    operand_t lhs = pc->operand0;
    operand_t idx = pc->operand1;

    value_obj* lhs_val = __get_register(this, lhs);
    const value_obj& rhs_val = __get_constant(this, idx);

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

    VIA_VMNEXT();
  }
  case ADDI: {
    operand_t lhs = pc->operand0;
    operand_t int_high = pc->operand1;
    operand_t int_low = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    TInteger imm = reinterpret_u16_as_i32(int_high, int_low);

    if VIA_LIKELY (lhs_val->is_int()) {
      lhs_val->val_integer += imm;
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point += imm;
    }

    VIA_VMNEXT();
  }
  case ADDF: {
    operand_t lhs = pc->operand0;
    operand_t flt_high = pc->operand1;
    operand_t flt_low = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    TFloat imm = reinterpret_u16_as_f32(flt_high, flt_low);

    if VIA_LIKELY (lhs_val->is_int()) {
      lhs_val->val_integer += imm;
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point += imm;
    }

    VIA_VMNEXT();
  }

  case SUB: {
    operand_t lhs = pc->operand0;
    operand_t rhs = pc->operand1;

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);

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

    VIA_VMNEXT();
  }
  case SUBK: {
    operand_t lhs = pc->operand0;
    operand_t idx = pc->operand1;

    value_obj* lhs_val = __get_register(this, lhs);
    const value_obj& rhs_val = __get_constant(this, idx);

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

    VIA_VMNEXT();
  }
  case SUBI: {
    operand_t lhs = pc->operand0;
    operand_t int_high = pc->operand1;
    operand_t int_low = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    TInteger imm = reinterpret_u16_as_i32(int_high, int_low);

    if VIA_LIKELY (lhs_val->is_int()) {
      lhs_val->val_integer -= imm;
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point -= imm;
    }

    VIA_VMNEXT();
  }
  case SUBF: {
    operand_t lhs = pc->operand0;
    operand_t flt_high = pc->operand1;
    operand_t flt_low = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    TFloat imm = reinterpret_u16_as_f32(flt_high, flt_low);

    if VIA_LIKELY (lhs_val->is_int()) {
      lhs_val->val_integer -= imm;
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point -= imm;
    }

    VIA_VMNEXT();
  }

  case MUL: {
    operand_t lhs = pc->operand0;
    operand_t rhs = pc->operand1;

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);

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

    VIA_VMNEXT();
  }
  case MULK: {
    operand_t lhs = pc->operand0;
    operand_t idx = pc->operand1;

    value_obj* lhs_val = __get_register(this, lhs);
    const value_obj& rhs_val = __get_constant(this, idx);

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

    VIA_VMNEXT();
  }
  case MULI: {
    operand_t lhs = pc->operand0;
    operand_t int_high = pc->operand1;
    operand_t int_low = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    TInteger imm = reinterpret_u16_as_i32(int_high, int_low);

    if VIA_LIKELY (lhs_val->is_int()) {
      lhs_val->val_integer *= imm;
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point *= imm;
    }

    VIA_VMNEXT();
  }
  case MULF: {
    operand_t lhs = pc->operand0;
    operand_t flt_high = pc->operand1;
    operand_t flt_low = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    TFloat imm = reinterpret_u16_as_f32(flt_high, flt_low);

    if VIA_LIKELY (lhs_val->is_int()) {
      lhs_val->val_integer *= imm;
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point *= imm;
    }

    VIA_VMNEXT();
  }

  case DIV: {
    operand_t lhs = pc->operand0;
    operand_t rhs = pc->operand1;

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        VIA_VMDIVBY0I(rhs_val);

        lhs_val->val_integer /= rhs_val->val_integer;
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        VIA_VMDIVBY0F(rhs_val->val_floating_point);

        lhs_val->val_floating_point =
          static_cast<TFloat>(lhs_val->val_integer) / rhs_val->val_floating_point;
        lhs_val->type = floating_point;
      }
    }
    else if (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        VIA_VMDIVBY0I(rhs_val);

        lhs_val->val_floating_point /= static_cast<TFloat>(rhs_val->val_integer);
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        VIA_VMDIVBY0F(rhs_val->val_floating_point);

        lhs_val->val_floating_point /= rhs_val->val_floating_point;
      }
    }

    VIA_VMNEXT();
  }
  case DIVK: {
    operand_t lhs = pc->operand0;
    operand_t idx = pc->operand1;

    value_obj* lhs_val = __get_register(this, lhs);
    const value_obj& rhs_val = __get_constant(this, idx);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val.is_int()) {
        VIA_VMDIVBY0I(rhs_val.val_integer);

        lhs_val->val_integer /= rhs_val.val_integer;
      }
      else if VIA_UNLIKELY (rhs_val.is_float()) {
        VIA_VMDIVBY0F(rhs_val.val_floating_point);

        lhs_val->val_floating_point =
          static_cast<TFloat>(lhs_val->val_integer) / rhs_val.val_floating_point;
        lhs_val->type = floating_point;
      }
    }
    else if (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val.is_int()) {
        VIA_VMDIVBY0I(rhs_val.val_integer);

        lhs_val->val_floating_point /= static_cast<TFloat>(rhs_val.val_integer);
      }
      else if VIA_UNLIKELY (rhs_val.is_float()) {
        VIA_VMDIVBY0F(rhs_val.val_floating_point);

        lhs_val->val_floating_point /= rhs_val.val_floating_point;
      }
    }

    VIA_VMNEXT();
  }
  case DIVI: {
    operand_t lhs = pc->operand0;
    operand_t int_high = pc->operand1;
    operand_t int_low = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    TInteger imm = reinterpret_u16_as_i32(int_high, int_low);

    VIA_VMDIVBY0I(imm);

    if VIA_LIKELY (lhs_val->is_int()) {
      lhs_val->val_integer /= imm;
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point /= imm;
    }

    VIA_VMNEXT();
  }
  case DIVF: {
    operand_t lhs = pc->operand0;
    operand_t flt_high = pc->operand1;
    operand_t flt_low = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    TFloat imm = reinterpret_u16_as_f32(flt_high, flt_low);

    VIA_VMDIVBY0F(imm);

    if VIA_LIKELY (lhs_val->is_int()) {
      lhs_val->val_integer /= imm;
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point /= imm;
    }

    VIA_VMNEXT();
  }

  case POW: {
    operand_t lhs = pc->operand0;
    operand_t rhs = pc->operand1;

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);

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

    VIA_VMNEXT();
  }
  case POWK: {
    operand_t lhs = pc->operand0;
    operand_t idx = pc->operand1;

    value_obj* lhs_val = __get_register(this, lhs);
    const value_obj& rhs_val = __get_constant(this, idx);

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

    VIA_VMNEXT();
  }
  case POWI: {
    operand_t lhs = pc->operand0;
    operand_t int_high = pc->operand1;
    operand_t int_low = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    TInteger imm = reinterpret_u16_as_i32(int_high, int_low);

    if VIA_LIKELY (lhs_val->is_int()) {
      lhs_val->val_integer = std::pow(lhs_val->val_integer, imm);
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point = std::pow(lhs_val->val_floating_point, imm);
    }

    VIA_VMNEXT();
  }
  case POWF: {
    operand_t lhs = pc->operand0;
    operand_t flt_high = pc->operand1;
    operand_t flt_low = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    TFloat imm = reinterpret_u16_as_f32(flt_high, flt_low);

    if VIA_LIKELY (lhs_val->is_int()) {
      lhs_val->val_integer = std::pow(lhs_val->val_integer, imm);
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point = std::pow(lhs_val->val_floating_point, imm);
    }

    VIA_VMNEXT();
  }

  case MOD: {
    operand_t lhs = pc->operand0;
    operand_t rhs = pc->operand1;

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);

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

    VIA_VMNEXT();
  }
  case MODK: {
    operand_t lhs = pc->operand0;
    operand_t idx = pc->operand1;

    value_obj* lhs_val = __get_register(this, lhs);
    const value_obj& rhs_val = __get_constant(this, idx);

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

    VIA_VMNEXT();
  }
  case MODI: {
    operand_t lhs = pc->operand0;
    operand_t int_high = pc->operand1;
    operand_t int_low = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    TInteger imm = reinterpret_u16_as_i32(int_high, int_low);



    if VIA_LIKELY (lhs_val->is_int()) {
      lhs_val->val_integer = std::fmod(lhs_val->val_integer, imm);
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point = std::fmod(lhs_val->val_floating_point, imm);
    }

    VIA_VMNEXT();
  }
  case MODF: {
    operand_t lhs = pc->operand0;
    operand_t flt_high = pc->operand1;
    operand_t flt_low = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    TFloat imm = reinterpret_u16_as_f32(flt_high, flt_low);



    if VIA_LIKELY (lhs_val->is_int()) {
      lhs_val->val_integer = std::fmod(lhs_val->val_integer, imm);
    }
    else if (lhs_val->is_float()) {
      lhs_val->val_floating_point = std::fmod(lhs_val->val_floating_point, imm);
    }

    VIA_VMNEXT();
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

    VIA_VMNEXT();
  }

  case MOVE: {
    operand_t rdst = pc->operand0;
    operand_t rsrc = pc->operand1;
    value_obj* src_val = __get_register(this, rsrc);

    __set_register(this, rdst, *src_val);
    VIA_VMNEXT();
  }

  case LOADK: {
    operand_t dst = pc->operand0;
    operand_t idx = pc->operand1;

    const value_obj& kval = __get_constant(this, idx);

    __set_register(this, dst, kval);
    VIA_VMNEXT();
  }

  case LOADNIL: {
    operand_t dst = pc->operand0;

    __set_register(this, dst, _Nil);
    VIA_VMNEXT();
  }

  case LOADI: {
    operand_t dst = pc->operand0;
    TInteger imm = reinterpret_u16_as_u32(pc->operand1, pc->operand2);

    __set_register(this, dst, value_obj(imm));
    VIA_VMNEXT();
  }

  case LOADF: {
    operand_t dst = pc->operand0;
    TFloat imm = reinterpret_u16_as_f32(pc->operand1, pc->operand2);

    __set_register(this, dst, value_obj(imm));
    VIA_VMNEXT();
  }

  case LOADBT: {
    operand_t dst = pc->operand0;
    __set_register(this, dst, value_obj(true));
    VIA_VMNEXT();
  }

  case LOADBF: {
    operand_t dst = pc->operand0;
    __set_register(this, dst, value_obj(false));
    VIA_VMNEXT();
  }

  case NEWTBL: {
    operand_t dst = pc->operand0;
    value_obj ttable(new table_obj());

    __set_register(this, dst, ttable);
    VIA_VMNEXT();
  }

  case NEWCLSR: {
    operand_t dst = pc->operand0;
    operand_t len = pc->operand1;

    function_obj* func = new function_obj();

    __closure_bytecode_load(this, func, len);
    __set_register(this, dst, value_obj(func));
    // Do not increment program counter, as __closure_bytecode_load automatically positions it to
    // the correct instruction.
    goto dispatch;
  }

  case UPVGET: {
    operand_t dst = pc->operand0;
    operand_t upv_id = pc->operand1;
    upv_obj* upv = __closure_upv_get(frame, upv_id);

    dump_struct(*upv->value);

    __set_register(this, dst, *upv->value);
    VIA_VMNEXT();
  }

  case UPVSET: {
    operand_t src = pc->operand0;
    operand_t upv_id = pc->operand1;
    value_obj* val = __get_register(this, src);

    __closure_upv_set(frame, upv_id, *val);
    VIA_VMNEXT();
  }

  case PUSH: {
    operand_t src = pc->operand0;
    value_obj* val = __get_register(this, src);

    __push(this, *val);
    VIA_VMNEXT();
  }

  case PUSHK: {
    operand_t const_idx = pc->operand0;
    value_obj constant = __get_constant(this, const_idx);

    __push(this, constant);
    VIA_VMNEXT();
  }

  case PUSHNIL: {
    __push(this, value_obj());
    VIA_VMNEXT();
  }

  case PUSHI: {
    TInteger imm = reinterpret_u16_as_u32(pc->operand0, pc->operand1);
    __push(this, value_obj(imm));
    VIA_VMNEXT();
  }

  case PUSHF: {
    TFloat imm = reinterpret_u16_as_f32(pc->operand0, pc->operand1);
    __push(this, value_obj(imm));
    VIA_VMNEXT();
  }

  case PUSHBT: {
    __push(this, value_obj(true));
    VIA_VMNEXT();
  }

  case PUSHBF: {
    __push(this, value_obj(false));
    VIA_VMNEXT();
  }

  case POP: {
    operand_t dst = pc->operand0;
    value_obj val = __pop(this);

    __set_register(this, dst, val);
    VIA_VMNEXT();
  }

  case DROP: {
    __pop(this);
    VIA_VMNEXT();
  }

  case STKGET: {
    operand_t dst = pc->operand0;
    operand_t off = pc->operand1;

    const value_obj& val = __get_stack(this, off);

    __set_register(this, dst, val);
    VIA_VMNEXT();
  }

  case STKSET: {
    operand_t src = pc->operand0;
    operand_t off = pc->operand1;

    value_obj* val = __get_register(this, src);

    sbp[off] = std::move(*val);
    VIA_VMNEXT();
  }

  case ARGGET: {
    operand_t dst = pc->operand0;
    operand_t off = pc->operand1;

    const value_obj& val = __get_argument(this, off);

    __set_register(this, dst, val);
    VIA_VMNEXT();
  }

  case GGET: {
    operand_t dst = pc->operand0;
    operand_t key = pc->operand1;

    value_obj* key_obj = __get_register(this, key);
    string_obj* key_str = key_obj->cast_ptr<string_obj>();
    const value_obj& global = glb->gtable.get(key_str->data);

    __set_register(this, dst, global);
    VIA_VMNEXT();
  }

  case GSET: {
    operand_t src = pc->operand0;
    operand_t key = pc->operand1;

    value_obj* key_obj = __get_register(this, key);
    string_obj* key_str = key_obj->cast_ptr<string_obj>();
    value_obj* global = __get_register(this, src);

    glb->gtable.set(key_str->data, *global);
    VIA_VMNEXT();
  }

  case EQ: {
    operand_t dst = pc->operand0;
    operand_t lhs = pc->operand1;
    operand_t rhs = pc->operand2;

    if VIA_UNLIKELY (lhs == rhs) {
      __set_register(this, dst, value_obj(true));
      VIA_VMNEXT();
    }

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);

    if VIA_UNLIKELY (lhs_val == rhs_val) {
      __set_register(this, dst, value_obj(true));
      VIA_VMNEXT();
    }

    bool result = __compare(*lhs_val, *rhs_val);
    __set_register(this, dst, value_obj(result));

    VIA_VMNEXT();
  }

  case NEQ: {
    operand_t dst = pc->operand0;
    operand_t lhs = pc->operand1;
    operand_t rhs = pc->operand2;

    if VIA_LIKELY (lhs != rhs) {
      __set_register(this, dst, value_obj(true));
      VIA_VMNEXT();
    }

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);

    if VIA_LIKELY (lhs_val != rhs_val) {
      __set_register(this, dst, value_obj(true));
      VIA_VMNEXT();
    }

    bool result = __compare(*lhs_val, *rhs_val);
    __set_register(this, dst, value_obj(result));

    VIA_VMNEXT();
  }

  case AND: {
    operand_t dst = pc->operand0;
    operand_t lhs = pc->operand1;
    operand_t rhs = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);
    bool cond = __to_cxx_bool(*lhs_val) && __to_cxx_bool(*rhs_val);

    __set_register(this, dst, value_obj(cond));
    VIA_VMNEXT();
  }

  case OR: {
    operand_t dst = pc->operand0;
    operand_t lhs = pc->operand1;
    operand_t rhs = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);
    bool cond = __to_cxx_bool(*lhs_val) || __to_cxx_bool(*rhs_val);

    __set_register(this, dst, value_obj(cond));
    VIA_VMNEXT();
  }

  case NOT: {
    operand_t dst = pc->operand0;
    operand_t lhs = pc->operand1;

    value_obj* lhs_val = __get_register(this, lhs);
    bool cond = !__to_cxx_bool(*lhs_val);

    __set_register(this, dst, value_obj(cond));
    VIA_VMNEXT();
  }

  case LT: {
    operand_t dst = pc->operand0;
    operand_t lhs = pc->operand1;
    operand_t rhs = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        __set_register(this, dst, value_obj(lhs_val->val_integer < rhs_val->val_integer));
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        __set_register(
          this,
          dst,
          value_obj(static_cast<TFloat>(lhs_val->val_integer) < rhs_val->val_floating_point)
        );
      }
    }
    else if VIA_UNLIKELY (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        __set_register(
          this,
          dst,
          value_obj(lhs_val->val_floating_point < static_cast<TFloat>(rhs_val->val_integer))
        );
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        __set_register(
          this, dst, value_obj(lhs_val->val_floating_point < rhs_val->val_floating_point)
        );
      }
    }

    VIA_VMNEXT();
  }

  case GT: {
    operand_t dst = pc->operand0;
    operand_t lhs = pc->operand1;
    operand_t rhs = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        __set_register(this, dst, value_obj(lhs_val->val_integer > rhs_val->val_integer));
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        __set_register(
          this,
          dst,
          value_obj(static_cast<TFloat>(lhs_val->val_integer) > rhs_val->val_floating_point)
        );
      }
    }
    else if VIA_UNLIKELY (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        __set_register(
          this,
          dst,
          value_obj(lhs_val->val_floating_point > static_cast<TFloat>(rhs_val->val_integer))
        );
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        __set_register(
          this, dst, value_obj(lhs_val->val_floating_point > rhs_val->val_floating_point)
        );
      }
    }

    VIA_VMNEXT();
  }

  case LTEQ: {
    operand_t dst = pc->operand0;
    operand_t lhs = pc->operand1;
    operand_t rhs = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        __set_register(this, dst, value_obj(lhs_val->val_integer <= rhs_val->val_integer));
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        __set_register(
          this,
          dst,
          value_obj(static_cast<TFloat>(lhs_val->val_integer) <= rhs_val->val_floating_point)
        );
      }
    }
    else if VIA_UNLIKELY (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        __set_register(
          this,
          dst,
          value_obj(lhs_val->val_floating_point <= static_cast<TFloat>(rhs_val->val_integer))
        );
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        __set_register(
          this, dst, value_obj(lhs_val->val_floating_point <= rhs_val->val_floating_point)
        );
      }
    }

    VIA_VMNEXT();
  }

  case GTEQ: {
    operand_t dst = pc->operand0;
    operand_t lhs = pc->operand1;
    operand_t rhs = pc->operand2;

    value_obj* lhs_val = __get_register(this, lhs);
    value_obj* rhs_val = __get_register(this, rhs);

    if VIA_LIKELY (lhs_val->is_int()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        __set_register(this, dst, value_obj(lhs_val->val_integer >= rhs_val->val_integer));
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        __set_register(
          this,
          dst,
          value_obj(static_cast<TFloat>(lhs_val->val_integer) >= rhs_val->val_floating_point)
        );
      }
    }
    else if VIA_UNLIKELY (lhs_val->is_float()) {
      if VIA_LIKELY (rhs_val->is_int()) {
        __set_register(
          this,
          dst,
          value_obj(lhs_val->val_floating_point >= static_cast<TFloat>(rhs_val->val_integer))
        );
      }
      else if VIA_UNLIKELY (rhs_val->is_float()) {
        __set_register(
          this, dst, value_obj(lhs_val->val_floating_point >= rhs_val->val_floating_point)
        );
      }
    }

    VIA_VMNEXT();
  }

  case EXIT: {
    goto exit;
  }

  case JMP: {
    signed_operand_t offset = pc->operand0;
    pc += offset;
    goto dispatch;
  }

  case JMPIF: {
    operand_t cond = pc->operand0;
    signed_operand_t offset = pc->operand1;

    value_obj* cond_val = __get_register(this, cond);
    if (__to_cxx_bool(*cond_val)) {
      pc += offset;
    }

    goto dispatch;
  }

  case JMPIFN: {
    operand_t cond = pc->operand0;
    signed_operand_t offset = pc->operand1;

    value_obj* cond_val = __get_register(this, cond);
    if (!__to_cxx_bool(*cond_val)) {
      pc += offset;
    }

    goto dispatch;
  }

  case JMPIFEQ: {
    operand_t cond_lhs = pc->operand0;
    operand_t cond_rhs = pc->operand1;
    signed_operand_t offset = pc->operand2;

    if VIA_UNLIKELY (cond_lhs == cond_rhs) {
      pc += offset;
    }
    else {
      value_obj* lhs_val = __get_register(this, cond_lhs);
      value_obj* rhs_val = __get_register(this, cond_rhs);

      if VIA_UNLIKELY (lhs_val == rhs_val || __compare(*lhs_val, *rhs_val)) {
        pc += offset;
      }
    }

    goto dispatch;
  }

  case JMPIFNEQ: {
    operand_t cond_lhs = pc->operand0;
    operand_t cond_rhs = pc->operand1;
    signed_operand_t offset = pc->operand2;

    if VIA_LIKELY (cond_lhs != cond_rhs) {
      pc += offset;
    }
    else {
      value_obj* lhs_val = __get_register(this, cond_lhs);
      value_obj* rhs_val = __get_register(this, cond_rhs);

      if VIA_LIKELY (lhs_val != rhs_val || !__compare(*lhs_val, *rhs_val)) {
        pc += offset;
      }
    }

    goto dispatch;
  }

  case JMPIFLT: {
    operand_t cond_lhs = pc->operand0;
    operand_t cond_rhs = pc->operand1;
    signed_operand_t offset = pc->operand2;

    value_obj* lhs_val = __get_register(this, cond_lhs);
    value_obj* rhs_val = __get_register(this, cond_rhs);

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

  case JMPIFGT: {
    operand_t cond_lhs = pc->operand0;
    operand_t cond_rhs = pc->operand1;
    signed_operand_t offset = pc->operand2;

    value_obj* lhs_val = __get_register(this, cond_lhs);
    value_obj* rhs_val = __get_register(this, cond_rhs);

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

  case JMPIFLTEQ: {
    operand_t cond_lhs = pc->operand0;
    operand_t cond_rhs = pc->operand1;
    signed_operand_t offset = pc->operand2;

    value_obj* lhs_val = __get_register(this, cond_lhs);
    value_obj* rhs_val = __get_register(this, cond_rhs);

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

  case JMPIFGTEQ: {
    operand_t cond_lhs = pc->operand0;
    operand_t cond_rhs = pc->operand1;
    signed_operand_t offset = pc->operand2;

    value_obj* lhs_val = __get_register(this, cond_lhs);
    value_obj* rhs_val = __get_register(this, cond_rhs);

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

  case LJMP: {
    operand_t label = pc->operand0;

    pc = __label_get(this, label);

    goto dispatch;
  }

  case LJMPIF: {
    operand_t cond = pc->operand0;
    operand_t label = pc->operand1;

    value_obj* cond_val = __get_register(this, cond);
    if (__to_cxx_bool(*cond_val)) {
      pc = __label_get(this, label);
    }

    goto dispatch;
  }

  case LJMPIFN: {
    operand_t cond = pc->operand0;
    operand_t label = pc->operand1;

    value_obj* cond_val = __get_register(this, cond);
    if (!__to_cxx_bool(*cond_val)) {
      pc = __label_get(this, label);
    }

    goto dispatch;
  }

  case LJMPIFEQ: {
    operand_t cond_lhs = pc->operand0;
    operand_t cond_rhs = pc->operand1;
    operand_t label = pc->operand2;

    if VIA_UNLIKELY (cond_lhs == cond_rhs) {
      pc = __label_get(this, label);
    }
    else {
      value_obj* lhs_val = __get_register(this, cond_lhs);
      value_obj* rhs_val = __get_register(this, cond_rhs);

      if VIA_UNLIKELY (lhs_val == rhs_val || __compare(*lhs_val, *rhs_val)) {
        pc = __label_get(this, label);
      }
    }

    goto dispatch;
  }

  case LJMPIFNEQ: {
    operand_t cond_lhs = pc->operand0;
    operand_t cond_rhs = pc->operand1;
    operand_t label = pc->operand2;

    if VIA_LIKELY (cond_lhs != cond_rhs) {
      pc = __label_get(this, label);
    }
    else {
      value_obj* lhs_val = __get_register(this, cond_lhs);
      value_obj* rhs_val = __get_register(this, cond_rhs);

      if VIA_LIKELY (lhs_val != rhs_val || !__compare(*lhs_val, *rhs_val)) {
        pc = __label_get(this, label);
      }
    }

    goto dispatch;
  }

  case LJMPIFLT: {
    operand_t cond_lhs = pc->operand0;
    operand_t cond_rhs = pc->operand1;
    operand_t label = pc->operand2;

    value_obj* lhs_val = __get_register(this, cond_lhs);
    value_obj* rhs_val = __get_register(this, cond_rhs);

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

  case LJMPIFGT: {
    operand_t cond_lhs = pc->operand0;
    operand_t cond_rhs = pc->operand1;
    operand_t label = pc->operand2;

    value_obj* lhs_val = __get_register(this, cond_lhs);
    value_obj* rhs_val = __get_register(this, cond_rhs);

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

  case LJMPIFLTEQ: {
    operand_t cond_lhs = pc->operand0;
    operand_t cond_rhs = pc->operand1;
    operand_t label = pc->operand2;

    value_obj* lhs_val = __get_register(this, cond_lhs);
    value_obj* rhs_val = __get_register(this, cond_rhs);

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

  case LJMPIFGTEQ: {
    operand_t cond_lhs = pc->operand0;
    operand_t cond_rhs = pc->operand1;
    operand_t label = pc->operand2;

    value_obj* lhs_val = __get_register(this, cond_lhs);
    value_obj* rhs_val = __get_register(this, cond_rhs);

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
    operand_t fn = pc->operand0;
    operand_t argc = pc->operand1;
    value_obj* fn_val = __get_register(this, fn);

    __call(this, *fn_val, argc);
    goto dispatch;
  }

  case CCALL: {
    operand_t fn = pc->operand0;
    operand_t argc = pc->operand1;
    value_obj* cfunc = __get_register(this, fn);

    __extern_call(this, *cfunc, argc);
    VIA_VMNEXT();
  }

  case NTVCALL: {
    operand_t fn = pc->operand0;
    operand_t argc = pc->operand1;
    value_obj* func = __get_register(this, fn);

    __native_call(this, func->cast_ptr<function_obj>(), argc);
    VIA_VMNEXT();
  }

  case MTDCALL: {
    operand_t obj = pc->operand0;
    operand_t fn = pc->operand1;
    operand_t argc = pc->operand2;

    value_obj* func = __get_register(this, fn);
    value_obj* object = __get_register(this, obj);

    __push(this, object->clone());
    __native_call(this, func->cast_ptr<function_obj>(), argc + 1);
    VIA_VMNEXT();
  }

  case RETNIL: {
    __closure_close_upvalues(frame);
    __native_return(this, _Nil);
    VIA_VMNEXT();
  }

  case RET: {
    operand_t src = pc->operand0;
    value_obj* val = __get_register(this, src);

    __closure_close_upvalues(frame);
    __native_return(this, *val);
    VIA_VMNEXT();
  }

  case TBLGET: {
    operand_t dst = pc->operand0;
    operand_t tbl = pc->operand1;
    operand_t key = pc->operand2;

    value_obj* tbl_val = __get_register(this, tbl);
    value_obj* key_val = __get_register(this, key);

    const value_obj& index = __table_get(tbl_val->cast_ptr<table_obj>(), *key_val);

    __set_register(this, dst, index);
    VIA_VMNEXT();
  }

  case TBLSET: {
    operand_t src = pc->operand0;
    operand_t tbl = pc->operand1;
    operand_t ky = pc->operand2;

    value_obj* table = __get_register(this, tbl);
    value_obj* value = __get_register(this, src);
    value_obj* key = __get_register(this, ky);

    __table_set(table->cast_ptr<table_obj>(), value_obj(key), *value);
    VIA_VMNEXT();
  }

  case TBLNEXT: {
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
    VIA_VMNEXT();
  }

  case TBLLEN: {
    operand_t dst = pc->operand0;
    operand_t tbl = pc->operand1;

    value_obj* val = __get_register(this, tbl);
    TInteger size = __table_size(val->cast_ptr<table_obj>());

    __set_register(this, dst, value_obj(size));
    VIA_VMNEXT();
  }

  case STRLEN: {
    operand_t rdst = pc->operand0;
    operand_t objr = pc->operand1;

    value_obj* val = __get_register(this, objr);
    TInteger len = val->cast_ptr<string_obj>()->len;

    __set_register(this, rdst, value_obj(len));
    VIA_VMNEXT();
  }

  case STRCONCAT: {
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

    __set_register(this, left, value_obj(new_string));

    delete[] new_string;

    VIA_VMNEXT();
  }

  case STRGET: {
    operand_t dst = pc->operand0;
    operand_t str = pc->operand1;
    operand_t idx = pc->operand2;

    value_obj* str_val = __get_register(this, str);
    string_obj* tstr = str_val->cast_ptr<string_obj>();
    char chr = tstr->data[idx];

    __set_register(this, dst, value_obj(&chr));
    VIA_VMNEXT();
  }

  case STRSET: {
    operand_t str = pc->operand0;
    operand_t src = pc->operand1;
    operand_t idx = pc->operand2;

    value_obj* str_val = __get_register(this, str);
    string_obj* tstr = str_val->cast_ptr<string_obj>();

    char chr = static_cast<char>(src);
    char* str_cpy = duplicate_string(tstr->data);
    str_cpy[idx] = chr;

    __set_register(this, str, value_obj(str_cpy));
    delete[] str_cpy;
    VIA_VMNEXT();
  }

  case CASTI: {
    operand_t dst = pc->operand0;
    operand_t src = pc->operand1;

    value_obj* target = __get_register(this, src);
    value_obj result = __to_int(this, *target);

    __set_register(this, dst, result);
    VIA_VMNEXT();
  }

  case CASTF: {
    operand_t dst = pc->operand0;
    operand_t src = pc->operand1;

    value_obj* target = __get_register(this, src);
    value_obj result = __to_float(this, *target);

    __set_register(this, dst, result);
    VIA_VMNEXT();
  }

  case CASTSTR: {
    operand_t dst = pc->operand0;
    operand_t src = pc->operand1;

    value_obj* target = __get_register(this, src);
    value_obj result = __to_string(*target);

    __set_register(this, dst, result);
    VIA_VMNEXT();
  }

  case CASTB: {
    operand_t dst = pc->operand0;
    operand_t src = pc->operand1;

    value_obj* target = __get_register(this, src);
    value_obj result = __to_bool(*target);

    __set_register(this, dst, result);
    VIA_VMNEXT();
  }

  default: {
    VIA_VMFATAL(std::format("unknown opcode 0x{:x}", static_cast<int>(pc->op)));
  }
  }
}

exit:;
}

} // namespace via
