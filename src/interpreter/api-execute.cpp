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
#define VM_ERROR(message)                                                                          \
  do {                                                                                             \
    __set_error_state(this, message);                                                              \
    goto dispatch;                                                                                 \
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
#define VM_USE_CGOTO VIA_COMPILER == C_GCC || VIA_COMPILER == C_CLANG

#if VM_USE_CGOTO
#define VM_CASE(op) CASE_##op:
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
    VM_DISPATCH_OP(LOADBF), VM_DISPATCH_OP(NEWARR), VM_DISPATCH_OP(NEWDICT),                       \
    VM_DISPATCH_OP(NEWCLSR), VM_DISPATCH_OP(PUSH), VM_DISPATCH_OP(PUSHK), VM_DISPATCH_OP(PUSHNIL), \
    VM_DISPATCH_OP(PUSHI), VM_DISPATCH_OP(PUSHF), VM_DISPATCH_OP(PUSHBT), VM_DISPATCH_OP(PUSHBF),  \
    VM_DISPATCH_OP(POP), VM_DISPATCH_OP(DROP), VM_DISPATCH_OP(STKGET), VM_DISPATCH_OP(STKSET),     \
    VM_DISPATCH_OP(ARGGET), VM_DISPATCH_OP(GGET), VM_DISPATCH_OP(GSET), VM_DISPATCH_OP(UPVSET),    \
    VM_DISPATCH_OP(UPVGET), VM_DISPATCH_OP(INC), VM_DISPATCH_OP(DEC), VM_DISPATCH_OP(EQ),          \
    VM_DISPATCH_OP(NEQ), VM_DISPATCH_OP(AND), VM_DISPATCH_OP(OR), VM_DISPATCH_OP(NOT),             \
    VM_DISPATCH_OP(LT), VM_DISPATCH_OP(GT), VM_DISPATCH_OP(LTEQ), VM_DISPATCH_OP(GTEQ),            \
    VM_DISPATCH_OP(JMP), VM_DISPATCH_OP(JMPIF), VM_DISPATCH_OP(JMPIFN), VM_DISPATCH_OP(JMPIFEQ),   \
    VM_DISPATCH_OP(JMPIFNEQ), VM_DISPATCH_OP(JMPIFLT), VM_DISPATCH_OP(JMPIFGT),                    \
    VM_DISPATCH_OP(JMPIFLTEQ), VM_DISPATCH_OP(JMPIFGTEQ), VM_DISPATCH_OP(LJMP),                    \
    VM_DISPATCH_OP(LJMPIF), VM_DISPATCH_OP(LJMPIFN), VM_DISPATCH_OP(LJMPIFEQ),                     \
    VM_DISPATCH_OP(LJMPIFNEQ), VM_DISPATCH_OP(LJMPIFLT), VM_DISPATCH_OP(LJMPIFGT),                 \
    VM_DISPATCH_OP(LJMPIFLTEQ), VM_DISPATCH_OP(LJMPIFGTEQ), VM_DISPATCH_OP(CALL),                  \
    VM_DISPATCH_OP(CCALL), VM_DISPATCH_OP(NTVCALL), VM_DISPATCH_OP(MTDCALL), VM_DISPATCH_OP(RET),  \
    VM_DISPATCH_OP(RETNIL), VM_DISPATCH_OP(RAISE), VM_DISPATCH_OP(TRY), VM_DISPATCH_OP(CATCH),     \
    VM_DISPATCH_OP(ARRGET), VM_DISPATCH_OP(ARRSET), VM_DISPATCH_OP(ARRNEXT),                       \
    VM_DISPATCH_OP(ARRLEN), VM_DISPATCH_OP(DICTGET), VM_DISPATCH_OP(DICTSET),                      \
    VM_DISPATCH_OP(DICTLEN), VM_DISPATCH_OP(DICTNEXT), VM_DISPATCH_OP(STRCONCAT),                  \
    VM_DISPATCH_OP(STRGET), VM_DISPATCH_OP(STRSET), VM_DISPATCH_OP(STRLEN), VM_DISPATCH_OP(CASTI), \
    VM_DISPATCH_OP(CASTF), VM_DISPATCH_OP(CASTSTR), VM_DISPATCH_OP(CASTB)

// Macro for checking divison by zero.
#define VM_DIVBY0I(divisor)                                                                        \
  if (divisor == 0) {                                                                              \
    VM_ERROR("Division by zero");                                                                  \
  }

#define VM_DIVBY0F(divisor)                                                                        \
  if (divisor == 0.0f) {                                                                           \
    VM_ERROR("Division by zero");                                                                  \
  }

// ==================================================================================================
// execute.cpp
//
namespace via {

using enum IValueType;
using enum IOpCode;

using namespace impl;

void vm_save_snapshot(state* VIA_RESTRICT V) {
  // Calculate the current program counter position relative to the Instruction base.
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

  // Build header information about the current Instruction.
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
  for (IValue* ptr = V->sbp; ptr < V->sbp + V->sp; ++ptr) {
    size_t index = ptr - V->sbp;
    // Format the stack index as two-digit with leading zeros.
    stack << "Stack[" << std::setw(2) << std::setfill('0') << index
          << "] = " << impl::__to_cxx_string(*ptr) << "\n";
  }
  stack << "==== End of Stack ====\n\n";

  // Generate register map.
  std::ostringstream registers;
  registers << "==== Registers ====\n";
  for (operand_t reg = 0; reg < VIA_ALL_REGISTERS - 1; ++reg) {
    IValue* val = impl::__get_register(V, reg);
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
#if VM_USE_CGOTO
  static const void* dispatch_table[0xFF] = {VM_DISPATCH_TABLE()};
#endif

dispatch:
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

#if VM_USE_CGOTO
  goto* dispatch_table[static_cast<uint8_t>(pc->op)];
#else
  switch (pc->op)
#endif
  {
    // Handle special/internal opcodes
    VM_CASE(NOP)
    VM_CASE(RAISE)
    VM_CASE(TRY)
    VM_CASE(CATCH)
    VM_CASE(DICTGET)
    VM_CASE(DICTSET)
    VM_CASE(DICTLEN)
    VM_CASE(DICTNEXT)
    VM_CASE(LBL) {
      VM_NEXT();
    }

    VM_CASE(ADD) {
      operand_t lhs = pc->operand0;
      operand_t rhs = pc->operand1;

      IValue* lhs_val = __get_register(this, lhs);
      IValue* rhs_val = __get_register(this, rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          lhs_val->val_integer += rhs_val->val_integer;
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          lhs_val->val_floating_point =
            static_cast<float>(lhs_val->val_integer) + rhs_val->val_floating_point;
          lhs_val->type = floating_point;
        }
      }
      else if (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          lhs_val->val_floating_point += static_cast<float>(rhs_val->val_integer);
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          lhs_val->val_floating_point += rhs_val->val_floating_point;
        }
      }

      VM_NEXT();
    }
    VM_CASE(ADDI) {
      operand_t lhs = pc->operand0;
      operand_t int_high = pc->operand1;
      operand_t int_low = pc->operand2;

      IValue* lhs_val = __get_register(this, lhs);
      int imm = reinterpret_u16_as_i32(int_high, int_low);

      if VIA_LIKELY (lhs_val->is_int()) {
        lhs_val->val_integer += imm;
      }
      else if (lhs_val->is_float()) {
        lhs_val->val_floating_point += imm;
      }

      VM_NEXT();
    }
    VM_CASE(ADDF) {
      operand_t lhs = pc->operand0;
      operand_t flt_high = pc->operand1;
      operand_t flt_low = pc->operand2;

      IValue* lhs_val = __get_register(this, lhs);
      float imm = reinterpret_u16_as_f32(flt_high, flt_low);

      if VIA_LIKELY (lhs_val->is_int()) {
        lhs_val->val_integer += imm;
      }
      else if (lhs_val->is_float()) {
        lhs_val->val_floating_point += imm;
      }

      VM_NEXT();
    }

    VM_CASE(SUB) {
      operand_t lhs = pc->operand0;
      operand_t rhs = pc->operand1;

      IValue* lhs_val = __get_register(this, lhs);
      IValue* rhs_val = __get_register(this, rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          lhs_val->val_integer -= rhs_val->val_integer;
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          lhs_val->val_floating_point =
            static_cast<float>(lhs_val->val_integer) - rhs_val->val_floating_point;
          lhs_val->type = floating_point;
        }
      }
      else if (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          lhs_val->val_floating_point -= static_cast<float>(rhs_val->val_integer);
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          lhs_val->val_floating_point -= rhs_val->val_floating_point;
        }
      }

      VM_NEXT();
    }
    VM_CASE(SUBI) {
      operand_t lhs = pc->operand0;
      operand_t int_high = pc->operand1;
      operand_t int_low = pc->operand2;

      IValue* lhs_val = __get_register(this, lhs);
      int imm = reinterpret_u16_as_i32(int_high, int_low);

      if VIA_LIKELY (lhs_val->is_int()) {
        lhs_val->val_integer -= imm;
      }
      else if (lhs_val->is_float()) {
        lhs_val->val_floating_point -= imm;
      }

      VM_NEXT();
    }
    VM_CASE(SUBF) {
      operand_t lhs = pc->operand0;
      operand_t flt_high = pc->operand1;
      operand_t flt_low = pc->operand2;

      IValue* lhs_val = __get_register(this, lhs);
      float imm = reinterpret_u16_as_f32(flt_high, flt_low);

      if VIA_LIKELY (lhs_val->is_int()) {
        lhs_val->val_integer -= imm;
      }
      else if (lhs_val->is_float()) {
        lhs_val->val_floating_point -= imm;
      }

      VM_NEXT();
    }

    VM_CASE(MUL) {
      operand_t lhs = pc->operand0;
      operand_t rhs = pc->operand1;

      IValue* lhs_val = __get_register(this, lhs);
      IValue* rhs_val = __get_register(this, rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          lhs_val->val_integer *= rhs_val->val_integer;
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          lhs_val->val_floating_point =
            static_cast<float>(lhs_val->val_integer) * rhs_val->val_floating_point;
          lhs_val->type = floating_point;
        }
      }
      else if (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          lhs_val->val_floating_point *= static_cast<float>(rhs_val->val_integer);
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          lhs_val->val_floating_point *= rhs_val->val_floating_point;
        }
      }

      VM_NEXT();
    }
    VM_CASE(MULI) {
      operand_t lhs = pc->operand0;
      operand_t int_high = pc->operand1;
      operand_t int_low = pc->operand2;

      IValue* lhs_val = __get_register(this, lhs);
      int imm = reinterpret_u16_as_i32(int_high, int_low);

      if VIA_LIKELY (lhs_val->is_int()) {
        lhs_val->val_integer *= imm;
      }
      else if (lhs_val->is_float()) {
        lhs_val->val_floating_point *= imm;
      }

      VM_NEXT();
    }
    VM_CASE(MULF) {
      operand_t lhs = pc->operand0;
      operand_t flt_high = pc->operand1;
      operand_t flt_low = pc->operand2;

      IValue* lhs_val = __get_register(this, lhs);
      float imm = reinterpret_u16_as_f32(flt_high, flt_low);

      if VIA_LIKELY (lhs_val->is_int()) {
        lhs_val->val_integer *= imm;
      }
      else if (lhs_val->is_float()) {
        lhs_val->val_floating_point *= imm;
      }

      VM_NEXT();
    }

    VM_CASE(DIV) {
      operand_t lhs = pc->operand0;
      operand_t rhs = pc->operand1;

      IValue* lhs_val = __get_register(this, lhs);
      IValue* rhs_val = __get_register(this, rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          VM_DIVBY0I(rhs_val);

          lhs_val->val_integer /= rhs_val->val_integer;
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          VM_DIVBY0F(rhs_val->val_floating_point);

          lhs_val->val_floating_point =
            static_cast<float>(lhs_val->val_integer) / rhs_val->val_floating_point;
          lhs_val->type = floating_point;
        }
      }
      else if (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          VM_DIVBY0I(rhs_val);

          lhs_val->val_floating_point /= static_cast<float>(rhs_val->val_integer);
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          VM_DIVBY0F(rhs_val->val_floating_point);

          lhs_val->val_floating_point /= rhs_val->val_floating_point;
        }
      }

      VM_NEXT();
    }
    VM_CASE(DIVI) {
      operand_t lhs = pc->operand0;
      operand_t int_high = pc->operand1;
      operand_t int_low = pc->operand2;

      IValue* lhs_val = __get_register(this, lhs);
      int imm = reinterpret_u16_as_i32(int_high, int_low);

      VM_DIVBY0I(imm);

      if VIA_LIKELY (lhs_val->is_int()) {
        lhs_val->val_integer /= imm;
      }
      else if (lhs_val->is_float()) {
        lhs_val->val_floating_point /= imm;
      }

      VM_NEXT();
    }
    VM_CASE(DIVF) {
      operand_t lhs = pc->operand0;
      operand_t flt_high = pc->operand1;
      operand_t flt_low = pc->operand2;

      IValue* lhs_val = __get_register(this, lhs);
      float imm = reinterpret_u16_as_f32(flt_high, flt_low);

      VM_DIVBY0F(imm);

      if VIA_LIKELY (lhs_val->is_int()) {
        lhs_val->val_integer /= imm;
      }
      else if (lhs_val->is_float()) {
        lhs_val->val_floating_point /= imm;
      }

      VM_NEXT();
    }

    VM_CASE(POW) {
      operand_t lhs = pc->operand0;
      operand_t rhs = pc->operand1;

      IValue* lhs_val = __get_register(this, lhs);
      IValue* rhs_val = __get_register(this, rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          lhs_val->val_integer = std::pow(lhs_val->val_integer, rhs_val->val_integer);
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          lhs_val->val_floating_point =
            std::pow(static_cast<float>(lhs_val->val_integer), rhs_val->val_floating_point);
          lhs_val->type = floating_point;
        }
      }
      else if (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          lhs_val->val_floating_point =
            std::pow(lhs_val->val_floating_point, static_cast<float>(rhs_val->val_integer));
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          lhs_val->val_floating_point =
            std::pow(lhs_val->val_floating_point, rhs_val->val_floating_point);
        }
      }

      VM_NEXT();
    }
    VM_CASE(POWI) {
      operand_t lhs = pc->operand0;
      operand_t int_high = pc->operand1;
      operand_t int_low = pc->operand2;

      IValue* lhs_val = __get_register(this, lhs);
      int imm = reinterpret_u16_as_i32(int_high, int_low);

      if VIA_LIKELY (lhs_val->is_int()) {
        lhs_val->val_integer = std::pow(lhs_val->val_integer, imm);
      }
      else if (lhs_val->is_float()) {
        lhs_val->val_floating_point = std::pow(lhs_val->val_floating_point, imm);
      }

      VM_NEXT();
    }
    VM_CASE(POWF) {
      operand_t lhs = pc->operand0;
      operand_t flt_high = pc->operand1;
      operand_t flt_low = pc->operand2;

      IValue* lhs_val = __get_register(this, lhs);
      float imm = reinterpret_u16_as_f32(flt_high, flt_low);

      if VIA_LIKELY (lhs_val->is_int()) {
        lhs_val->val_integer = std::pow(lhs_val->val_integer, imm);
      }
      else if (lhs_val->is_float()) {
        lhs_val->val_floating_point = std::pow(lhs_val->val_floating_point, imm);
      }

      VM_NEXT();
    }

    VM_CASE(MOD) {
      operand_t lhs = pc->operand0;
      operand_t rhs = pc->operand1;

      IValue* lhs_val = __get_register(this, lhs);
      IValue* rhs_val = __get_register(this, rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          lhs_val->val_integer %= rhs_val->val_integer;
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          lhs_val->val_floating_point =
            std::fmod(static_cast<float>(lhs_val->val_integer), rhs_val->val_floating_point);
          lhs_val->type = floating_point;
        }
      }
      else if (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          lhs_val->val_floating_point =
            std::fmod(lhs_val->val_floating_point, static_cast<float>(rhs_val->val_integer));
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          lhs_val->val_floating_point =
            std::fmod(lhs_val->val_floating_point, rhs_val->val_floating_point);
        }
      }

      VM_NEXT();
    }
    VM_CASE(MODI) {
      operand_t lhs = pc->operand0;
      operand_t int_high = pc->operand1;
      operand_t int_low = pc->operand2;

      IValue* lhs_val = __get_register(this, lhs);
      int imm = reinterpret_u16_as_i32(int_high, int_low);



      if VIA_LIKELY (lhs_val->is_int()) {
        lhs_val->val_integer = std::fmod(lhs_val->val_integer, imm);
      }
      else if (lhs_val->is_float()) {
        lhs_val->val_floating_point = std::fmod(lhs_val->val_floating_point, imm);
      }

      VM_NEXT();
    }
    VM_CASE(MODF) {
      operand_t lhs = pc->operand0;
      operand_t flt_high = pc->operand1;
      operand_t flt_low = pc->operand2;

      IValue* lhs_val = __get_register(this, lhs);
      float imm = reinterpret_u16_as_f32(flt_high, flt_low);

      if VIA_LIKELY (lhs_val->is_int()) {
        lhs_val->val_integer = std::fmod(lhs_val->val_integer, imm);
      }
      else if (lhs_val->is_float()) {
        lhs_val->val_floating_point = std::fmod(lhs_val->val_floating_point, imm);
      }

      VM_NEXT();
    }

    VM_CASE(NEG) {
      operand_t dst = pc->operand0;
      IValue* val = __get_register(this, dst);
      IValueType type = val->type;

      if (type == integer) {
        val->val_integer = -val->val_integer;
      }
      else if (type == floating_point) {
        val->val_floating_point = -val->val_floating_point;
      }

      VM_NEXT();
    }

    VM_CASE(MOV) {
      operand_t rdst = pc->operand0;
      operand_t rsrc = pc->operand1;
      IValue* src_val = __get_register(this, rsrc);

      __set_register(this, rdst, src_val->clone());
      VM_NEXT();
    }

    VM_CASE(INC) {
      operand_t rdst = pc->operand0;
      IValue* dst_val = __get_register(this, rdst);

      if VIA_LIKELY (dst_val->is_int()) {
        dst_val->val_integer++;
      }
      else if VIA_UNLIKELY (dst_val->is_float()) {
        dst_val->val_floating_point++;
      }

      VM_NEXT();
    }

    VM_CASE(DEC) {
      operand_t rdst = pc->operand0;
      IValue* dst_val = __get_register(this, rdst);

      if VIA_LIKELY (dst_val->is_int()) {
        dst_val->val_integer--;
      }
      else if VIA_UNLIKELY (dst_val->is_float()) {
        dst_val->val_floating_point--;
      }

      VM_NEXT();
    }

    VM_CASE(LOADK) {
      operand_t dst = pc->operand0;
      operand_t idx = pc->operand1;

      const IValue& kval = __get_constant(this, idx);

      __set_register(this, dst, kval.clone());
      VM_NEXT();
    }

    VM_CASE(LOADNIL) {
      operand_t dst = pc->operand0;

      __set_register(this, dst, IValue());
      VM_NEXT();
    }

    VM_CASE(LOADI) {
      operand_t dst = pc->operand0;
      int imm = reinterpret_u16_as_u32(pc->operand1, pc->operand2);

      __set_register(this, dst, IValue(imm));
      VM_NEXT();
    }

    VM_CASE(LOADF) {
      operand_t dst = pc->operand0;
      float imm = reinterpret_u16_as_f32(pc->operand1, pc->operand2);

      __set_register(this, dst, IValue(imm));
      VM_NEXT();
    }

    VM_CASE(LOADBT) {
      operand_t dst = pc->operand0;
      __set_register(this, dst, IValue(true));
      VM_NEXT();
    }

    VM_CASE(LOADBF) {
      operand_t dst = pc->operand0;
      __set_register(this, dst, IValue(false));
      VM_NEXT();
    }

    VM_CASE(NEWARR) {
      operand_t dst = pc->operand0;
      IValue arr(new IArray());

      __set_register(this, dst, std::move(arr));
      VM_NEXT();
    }

    VM_CASE(NEWDICT) {
      operand_t dst = pc->operand0;
      IValue dict(new IDict());

      __set_register(this, dst, std::move(dict));
      VM_NEXT();
    }

    VM_CASE(NEWCLSR) {
      operand_t dst = pc->operand0;
      operand_t len = pc->operand1;

      IFunction* func = new IFunction();

      __closure_bytecode_load(this, func, len);
      __set_register(this, dst, IValue(func));
      // Do not increment program counter, as __closure_bytecode_load automatically positions it to
      // the correct instruction.
      goto dispatch;
    }

    VM_CASE(UPVGET) {
      operand_t dst = pc->operand0;
      operand_t upv_id = pc->operand1;
      IUpValue* upv = __closure_upv_get(frame, upv_id);

      __set_register(this, dst, upv->value->clone());
      VM_NEXT();
    }

    VM_CASE(UPVSET) {
      operand_t src = pc->operand0;
      operand_t upv_id = pc->operand1;
      IValue* val = __get_register(this, src);

      __closure_upv_set(frame, upv_id, *val);
      VM_NEXT();
    }

    VM_CASE(PUSH) {
      operand_t src = pc->operand0;
      IValue* val = __get_register(this, src);

      __push(this, std::move(*val));
      VM_NEXT();
    }

    VM_CASE(PUSHK) {
      operand_t const_idx = pc->operand0;
      IValue constant = __get_constant(this, const_idx);

      __push(this, constant.clone());
      VM_NEXT();
    }

    VM_CASE(PUSHNIL) {
      __push(this, IValue());
      VM_NEXT();
    }

    VM_CASE(PUSHI) {
      int imm = reinterpret_u16_as_u32(pc->operand0, pc->operand1);
      __push(this, IValue(imm));
      VM_NEXT();
    }

    VM_CASE(PUSHF) {
      float imm = reinterpret_u16_as_f32(pc->operand0, pc->operand1);
      __push(this, IValue(imm));
      VM_NEXT();
    }

    VM_CASE(PUSHBT) {
      __push(this, IValue(true));
      VM_NEXT();
    }

    VM_CASE(PUSHBF) {
      __push(this, IValue(false));
      VM_NEXT();
    }

    VM_CASE(POP) {
      operand_t dst = pc->operand0;
      IValue val = __pop(this);

      __set_register(this, dst, std::move(val));
      VM_NEXT();
    }

    VM_CASE(DROP) {
      __pop(this);
      VM_NEXT();
    }

    VM_CASE(STKGET) {
      operand_t dst = pc->operand0;
      operand_t off = pc->operand1;

      const IValue& val = __get_stack(this, off);

      __set_register(this, dst, val.clone());
      VM_NEXT();
    }

    VM_CASE(STKSET) {
      operand_t src = pc->operand0;
      operand_t off = pc->operand1;

      IValue* val = __get_register(this, src);

      __set_stack(this, off, std::move(*val));
      VM_NEXT();
    }

    VM_CASE(ARGGET) {
      operand_t dst = pc->operand0;
      operand_t off = pc->operand1;

      const IValue& val = __get_argument(this, off);

      __set_register(this, dst, val.clone());
      VM_NEXT();
    }

    VM_CASE(GGET) {
      operand_t dst = pc->operand0;
      operand_t key = pc->operand1;

      IValue* key_obj = __get_register(this, key);
      IString* key_str = key_obj->val_string;
      const IValue& global = glb->gtable.get(key_str->data);

      __set_register(this, dst, global.clone());
      VM_NEXT();
    }

    VM_CASE(GSET) {
      operand_t src = pc->operand0;
      operand_t key = pc->operand1;

      IValue* key_obj = __get_register(this, key);
      IString* key_str = key_obj->val_string;
      IValue* global = __get_register(this, src);

      glb->gtable.set(key_str->data, std::move(*global));
      VM_NEXT();
    }

    VM_CASE(EQ) {
      operand_t dst = pc->operand0;
      operand_t lhs = pc->operand1;
      operand_t rhs = pc->operand2;

      if VIA_UNLIKELY (lhs == rhs) {
        __set_register(this, dst, IValue(true));
        VM_NEXT();
      }

      IValue* lhs_val = __get_register(this, lhs);
      IValue* rhs_val = __get_register(this, rhs);

      if VIA_UNLIKELY (lhs_val == rhs_val) {
        __set_register(this, dst, IValue(true));
        VM_NEXT();
      }

      bool result = __compare(*lhs_val, *rhs_val);
      __set_register(this, dst, IValue(result));

      VM_NEXT();
    }

    VM_CASE(NEQ) {
      operand_t dst = pc->operand0;
      operand_t lhs = pc->operand1;
      operand_t rhs = pc->operand2;

      if VIA_LIKELY (lhs != rhs) {
        __set_register(this, dst, IValue(true));
        VM_NEXT();
      }

      IValue* lhs_val = __get_register(this, lhs);
      IValue* rhs_val = __get_register(this, rhs);

      if VIA_LIKELY (lhs_val != rhs_val) {
        __set_register(this, dst, IValue(true));
        VM_NEXT();
      }

      bool result = __compare(*lhs_val, *rhs_val);
      __set_register(this, dst, IValue(result));

      VM_NEXT();
    }

    VM_CASE(AND) {
      operand_t dst = pc->operand0;
      operand_t lhs = pc->operand1;
      operand_t rhs = pc->operand2;

      IValue* lhs_val = __get_register(this, lhs);
      IValue* rhs_val = __get_register(this, rhs);
      bool cond = __to_cxx_bool(*lhs_val) && __to_cxx_bool(*rhs_val);

      __set_register(this, dst, IValue(cond));
      VM_NEXT();
    }

    VM_CASE(OR) {
      operand_t dst = pc->operand0;
      operand_t lhs = pc->operand1;
      operand_t rhs = pc->operand2;

      IValue* lhs_val = __get_register(this, lhs);
      IValue* rhs_val = __get_register(this, rhs);
      bool cond = __to_cxx_bool(*lhs_val) || __to_cxx_bool(*rhs_val);

      __set_register(this, dst, IValue(cond));
      VM_NEXT();
    }

    VM_CASE(NOT) {
      operand_t dst = pc->operand0;
      operand_t lhs = pc->operand1;

      IValue* lhs_val = __get_register(this, lhs);
      bool cond = !__to_cxx_bool(*lhs_val);

      __set_register(this, dst, IValue(cond));
      VM_NEXT();
    }

    VM_CASE(LT) {
      operand_t dst = pc->operand0;
      operand_t lhs = pc->operand1;
      operand_t rhs = pc->operand2;

      IValue* lhs_val = __get_register(this, lhs);
      IValue* rhs_val = __get_register(this, rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          __set_register(this, dst, IValue(lhs_val->val_integer < rhs_val->val_integer));
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          __set_register(
            this,
            dst,
            IValue(static_cast<float>(lhs_val->val_integer) < rhs_val->val_floating_point)
          );
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          __set_register(
            this,
            dst,
            IValue(lhs_val->val_floating_point < static_cast<float>(rhs_val->val_integer))
          );
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          __set_register(
            this, dst, IValue(lhs_val->val_floating_point < rhs_val->val_floating_point)
          );
        }
      }

      VM_NEXT();
    }

    VM_CASE(GT) {
      operand_t dst = pc->operand0;
      operand_t lhs = pc->operand1;
      operand_t rhs = pc->operand2;

      IValue* lhs_val = __get_register(this, lhs);
      IValue* rhs_val = __get_register(this, rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          __set_register(this, dst, IValue(lhs_val->val_integer > rhs_val->val_integer));
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          __set_register(
            this,
            dst,
            IValue(static_cast<float>(lhs_val->val_integer) > rhs_val->val_floating_point)
          );
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          __set_register(
            this,
            dst,
            IValue(lhs_val->val_floating_point > static_cast<float>(rhs_val->val_integer))
          );
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          __set_register(
            this, dst, IValue(lhs_val->val_floating_point > rhs_val->val_floating_point)
          );
        }
      }

      VM_NEXT();
    }

    VM_CASE(LTEQ) {
      operand_t dst = pc->operand0;
      operand_t lhs = pc->operand1;
      operand_t rhs = pc->operand2;

      IValue* lhs_val = __get_register(this, lhs);
      IValue* rhs_val = __get_register(this, rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          __set_register(this, dst, IValue(lhs_val->val_integer <= rhs_val->val_integer));
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          __set_register(
            this,
            dst,
            IValue(static_cast<float>(lhs_val->val_integer) <= rhs_val->val_floating_point)
          );
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          __set_register(
            this,
            dst,
            IValue(lhs_val->val_floating_point <= static_cast<float>(rhs_val->val_integer))
          );
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          __set_register(
            this, dst, IValue(lhs_val->val_floating_point <= rhs_val->val_floating_point)
          );
        }
      }

      VM_NEXT();
    }

    VM_CASE(GTEQ) {
      operand_t dst = pc->operand0;
      operand_t lhs = pc->operand1;
      operand_t rhs = pc->operand2;

      IValue* lhs_val = __get_register(this, lhs);
      IValue* rhs_val = __get_register(this, rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          __set_register(this, dst, IValue(lhs_val->val_integer >= rhs_val->val_integer));
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          __set_register(
            this,
            dst,
            IValue(static_cast<float>(lhs_val->val_integer) >= rhs_val->val_floating_point)
          );
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          __set_register(
            this,
            dst,
            IValue(lhs_val->val_floating_point >= static_cast<float>(rhs_val->val_integer))
          );
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          __set_register(
            this, dst, IValue(lhs_val->val_floating_point >= rhs_val->val_floating_point)
          );
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

      IValue* cond_val = __get_register(this, cond);
      if (__to_cxx_bool(*cond_val)) {
        pc += offset;
        goto dispatch;
      }

      VM_NEXT();
    }

    VM_CASE(JMPIFN) {
      operand_t cond = pc->operand0;
      signed_operand_t offset = pc->operand1;

      IValue* cond_val = __get_register(this, cond);
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
        IValue* lhs_val = __get_register(this, cond_lhs);
        IValue* rhs_val = __get_register(this, cond_rhs);

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
        IValue* lhs_val = __get_register(this, cond_lhs);
        IValue* rhs_val = __get_register(this, cond_rhs);

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

      IValue* lhs_val = __get_register(this, cond_lhs);
      IValue* rhs_val = __get_register(this, cond_rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->val_integer < rhs_val->val_integer) {
            pc += offset;
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (static_cast<float>(lhs_val->val_integer) < rhs_val->val_floating_point) {
            pc += offset;
            goto dispatch;
          }
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->val_floating_point < static_cast<float>(rhs_val->val_integer)) {
            pc += offset;
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (lhs_val->val_floating_point < rhs_val->val_floating_point) {
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

      IValue* lhs_val = __get_register(this, cond_lhs);
      IValue* rhs_val = __get_register(this, cond_rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->val_integer > rhs_val->val_integer) {
            pc += offset;
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (static_cast<float>(lhs_val->val_integer) > rhs_val->val_floating_point) {
            pc += offset;
            goto dispatch;
          }
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->val_floating_point > static_cast<float>(rhs_val->val_integer)) {
            pc += offset;
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (lhs_val->val_floating_point > rhs_val->val_floating_point) {
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

      IValue* lhs_val = __get_register(this, cond_lhs);
      IValue* rhs_val = __get_register(this, cond_rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->val_integer <= rhs_val->val_integer) {
            pc += offset;
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (static_cast<float>(lhs_val->val_integer) <= rhs_val->val_floating_point) {
            pc += offset;
            goto dispatch;
          }
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->val_floating_point <= static_cast<float>(rhs_val->val_integer)) {
            pc += offset;
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (lhs_val->val_floating_point <= rhs_val->val_floating_point) {
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

      IValue* lhs_val = __get_register(this, cond_lhs);
      IValue* rhs_val = __get_register(this, cond_rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->val_integer >= rhs_val->val_integer) {
            pc += offset;
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (static_cast<float>(lhs_val->val_integer) >= rhs_val->val_floating_point) {
            pc += offset;
            goto dispatch;
          }
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->val_floating_point >= static_cast<float>(rhs_val->val_integer)) {
            pc += offset;
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (lhs_val->val_floating_point >= rhs_val->val_floating_point) {
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

      IValue* cond_val = __get_register(this, cond);
      if (__to_cxx_bool(*cond_val)) {
        pc = __label_get(this, label);
        goto dispatch;
      }

      VM_NEXT();
    }

    VM_CASE(LJMPIFN) {
      operand_t cond = pc->operand0;
      operand_t label = pc->operand1;

      IValue* cond_val = __get_register(this, cond);
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
        IValue* lhs_val = __get_register(this, cond_lhs);
        IValue* rhs_val = __get_register(this, cond_rhs);

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
        IValue* lhs_val = __get_register(this, cond_lhs);
        IValue* rhs_val = __get_register(this, cond_rhs);

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

      IValue* lhs_val = __get_register(this, cond_lhs);
      IValue* rhs_val = __get_register(this, cond_rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->val_integer < rhs_val->val_integer) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (static_cast<float>(lhs_val->val_integer) < rhs_val->val_floating_point) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->val_floating_point < static_cast<float>(rhs_val->val_integer)) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (lhs_val->val_floating_point < rhs_val->val_floating_point) {
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

      IValue* lhs_val = __get_register(this, cond_lhs);
      IValue* rhs_val = __get_register(this, cond_rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->val_integer > rhs_val->val_integer) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (static_cast<float>(lhs_val->val_integer) > rhs_val->val_floating_point) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->val_floating_point > static_cast<float>(rhs_val->val_integer)) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (lhs_val->val_floating_point > rhs_val->val_floating_point) {
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

      IValue* lhs_val = __get_register(this, cond_lhs);
      IValue* rhs_val = __get_register(this, cond_rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->val_integer <= rhs_val->val_integer) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (static_cast<float>(lhs_val->val_integer) <= rhs_val->val_floating_point) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->val_floating_point <= static_cast<float>(rhs_val->val_integer)) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (lhs_val->val_floating_point <= rhs_val->val_floating_point) {
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

      IValue* lhs_val = __get_register(this, cond_lhs);
      IValue* rhs_val = __get_register(this, cond_rhs);

      if VIA_LIKELY (lhs_val->is_int()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->val_integer >= rhs_val->val_integer) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (static_cast<float>(lhs_val->val_integer) >= rhs_val->val_floating_point) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
      }
      else if VIA_UNLIKELY (lhs_val->is_float()) {
        if VIA_LIKELY (rhs_val->is_int()) {
          if (lhs_val->val_floating_point >= static_cast<float>(rhs_val->val_integer)) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
        else if VIA_UNLIKELY (rhs_val->is_float()) {
          if (lhs_val->val_floating_point >= rhs_val->val_floating_point) {
            pc = __label_get(this, label);
            goto dispatch;
          }
        }
      }

      VM_NEXT();
    }

    VM_CASE(CALL) {
      operand_t fn = pc->operand0;
      operand_t argc = pc->operand1;
      IValue* fn_val = __get_register(this, fn);

      __call(this, *fn_val, argc);
      goto dispatch;
    }

    VM_CASE(CCALL) {
      operand_t fn = pc->operand0;
      operand_t argc = pc->operand1;
      IValue* cfunc = __get_register(this, fn);

      __extern_call(this, *cfunc, argc);
      VM_NEXT();
    }

    VM_CASE(NTVCALL) {
      operand_t fn = pc->operand0;
      operand_t argc = pc->operand1;
      IValue* func = __get_register(this, fn);

      __native_call(this, func->val_function, argc);
      VM_NEXT();
    }

    VM_CASE(MTDCALL) {
      operand_t obj = pc->operand0;
      operand_t fn = pc->operand1;
      operand_t argc = pc->operand2;

      IValue* func = __get_register(this, fn);
      IValue* object = __get_register(this, obj);

      __push(this, object->clone());
      __native_call(this, func->val_function, argc + 1);
      VM_NEXT();
    }

    VM_CASE(RETNIL) {
      __closure_close_upvalues(frame);
      __native_return(this, IValue());
      VM_NEXT();
    }

    VM_CASE(RET) {
      operand_t src = pc->operand0;
      IValue* val = __get_register(this, src);

      __closure_close_upvalues(frame);
      __native_return(this, std::move(*val));
      VM_NEXT();
    }

    VM_CASE(ARRGET) {
      operand_t dst = pc->operand0;
      operand_t tbl = pc->operand1;
      operand_t key = pc->operand2;

      IValue* value = __get_register(this, tbl);
      IValue* index = __get_register(this, key);
      IValue* result = __array_get(value->val_array, index->val_integer);

      __set_register(this, dst, result->clone());
      VM_NEXT();
    }

    VM_CASE(ARRSET) {
      operand_t src = pc->operand0;
      operand_t tbl = pc->operand1;
      operand_t key = pc->operand2;

      IValue* array = __get_register(this, tbl);
      IValue* index = __get_register(this, key);
      IValue* value = __get_register(this, src);

      __array_set(array->val_array, index->val_integer, std::move(*value));
      VM_NEXT();
    }

    VM_CASE(ARRNEXT) {
      static std::unordered_map<void*, operand_t> next_table;

      operand_t dst = pc->operand0;
      operand_t valr = pc->operand1;

      IValue* val = __get_register(this, valr);
      void* ptr = __to_pointer(*val);
      operand_t key = 0;

      auto it = next_table.find(ptr);
      if (it != next_table.end()) {
        key = ++it->second;
      }
      else {
        next_table[ptr] = 0;
      }

      IValue* field = __array_get(val->val_array, key);
      __set_register(this, dst, field->clone());
      VM_NEXT();
    }

    VM_CASE(ARRLEN) {
      operand_t dst = pc->operand0;
      operand_t tbl = pc->operand1;

      IValue* val = __get_register(this, tbl);
      int size = __array_size(val->val_array);

      __set_register(this, dst, IValue(size));
      VM_NEXT();
    }

    VM_CASE(STRLEN) {
      operand_t rdst = pc->operand0;
      operand_t objr = pc->operand1;

      IValue* val = __get_register(this, objr);
      int len = val->val_string->len;

      __set_register(this, rdst, IValue(len));
      VM_NEXT();
    }

    VM_CASE(STRCONCAT) {
      operand_t left = pc->operand0;
      operand_t right = pc->operand1;

      IValue* left_val = __get_register(this, left);
      IValue* right_val = __get_register(this, right);

      IString* left_str = left_val->val_string;
      IString* right_str = right_val->val_string;

      size_t new_length = left_str->len + right_str->len;
      char* new_string = new char[new_length + 1];

      std::memcpy(new_string, left_str->data, left_str->len);
      std::memcpy(new_string + left_str->len, right_str->data, right_str->len);

      __set_register(this, left, IValue(new_string));

      delete[] new_string;

      VM_NEXT();
    }

    VM_CASE(STRGET) {
      operand_t dst = pc->operand0;
      operand_t str = pc->operand1;
      operand_t idx = pc->operand2;

      IValue* str_val = __get_register(this, str);
      IString* tstr = str_val->val_string;
      char chr = tstr->data[idx];

      __set_register(this, dst, IValue(&chr));
      VM_NEXT();
    }

    VM_CASE(STRSET) {
      operand_t str = pc->operand0;
      operand_t src = pc->operand1;
      operand_t idx = pc->operand2;

      IValue* str_val = __get_register(this, str);
      IString* tstr = str_val->val_string;

      char chr = static_cast<char>(src);
      char* str_cpy = duplicate_string(tstr->data);
      str_cpy[idx] = chr;

      __set_register(this, str, IValue(str_cpy));
      delete[] str_cpy;
      VM_NEXT();
    }

    VM_CASE(CASTI) {
      operand_t dst = pc->operand0;
      operand_t src = pc->operand1;

      IValue* target = __get_register(this, src);
      IValue result = __to_int(this, *target);

      __set_register(this, dst, std::move(result));
      VM_NEXT();
    }

    VM_CASE(CASTF) {
      operand_t dst = pc->operand0;
      operand_t src = pc->operand1;

      IValue* target = __get_register(this, src);
      IValue result = __to_float(this, *target);

      __set_register(this, dst, std::move(result));
      VM_NEXT();
    }

    VM_CASE(CASTSTR) {
      operand_t dst = pc->operand0;
      operand_t src = pc->operand1;

      IValue* target = __get_register(this, src);
      IValue result = __to_string(*target);

      __set_register(this, dst, std::move(result));
      VM_NEXT();
    }

    VM_CASE(CASTB) {
      operand_t dst = pc->operand0;
      operand_t src = pc->operand1;

      IValue* target = __get_register(this, src);
      IValue result = __to_bool(*target);

      __set_register(this, dst, std::move(result));
      VM_NEXT();
    }
  }

exit:;
}

} // namespace via
