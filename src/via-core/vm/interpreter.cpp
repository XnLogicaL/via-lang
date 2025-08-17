// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "interpreter.h"
#include "panic.h"
#include "value.h"

namespace via {

Stack<uptr>& Interpreter::get_stack() {
  return stack;
}

HeapAllocator& Interpreter::get_allocator() {
  return alloc;
}

ValueRef Interpreter::get_register(u16 reg) {
  return regs.data[reg]->make_ref();
}

void Interpreter::set_register(u16 reg, ValueRef val) {
  regs.data[reg] = val.get();
}

ValueRef Interpreter::new_local(ValueRef val) {
  stack.push((uptr)val.get());
  return get_local(stack.size() - 1);
}

void Interpreter::set_local(usize sp, ValueRef val) {
  *stack.at(sp) = reinterpret_cast<uptr>(val.get());
}

ValueRef Interpreter::get_local(usize sp) {
  return reinterpret_cast<Value*>(stack.at(sp))->make_ref();
}

#if defined(VIA_COMPILER_GCC) || defined(VIA_COMPILER_CLANG)
#define HAS_CGOTO
#endif

#ifdef HAS_CGOTO
#define CASE(op) case_##op:
#define DISPATCH_OP(op) &&case_##op
#define DISPATCH_TABLE()                                                      \
  DISPATCH_OP(NOP), DISPATCH_OP(HALT), DISPATCH_OP(EXTRAARG1),                \
      DISPATCH_OP(EXTRAARG2), DISPATCH_OP(EXTRAARG3), DISPATCH_OP(MOVE),      \
      DISPATCH_OP(XCHG), DISPATCH_OP(COPY), DISPATCH_OP(COPYREF),             \
      DISPATCH_OP(LOADTRUE), DISPATCH_OP(LOADFALSE), DISPATCH_OP(NEWSTR),     \
      DISPATCH_OP(NEWSTR2), DISPATCH_OP(NEWARR), DISPATCH_OP(NEWARR2),        \
      DISPATCH_OP(NEWDICT), DISPATCH_OP(NEWTUPLE), DISPATCH_OP(NEWCLOSURE),   \
      DISPATCH_OP(IADD1), DISPATCH_OP(IADD2), DISPATCH_OP(IADD1K),            \
      DISPATCH_OP(IADD2K), DISPATCH_OP(FADD1), DISPATCH_OP(FADD2),            \
      DISPATCH_OP(FADD1K), DISPATCH_OP(FADD2K), DISPATCH_OP(ISUB1),           \
      DISPATCH_OP(ISUB2), DISPATCH_OP(ISUB1K), DISPATCH_OP(ISUB2K),           \
      DISPATCH_OP(ISUB2KX), DISPATCH_OP(FSUB1), DISPATCH_OP(FSUB2),           \
      DISPATCH_OP(FSUB1K), DISPATCH_OP(FSUB2K), DISPATCH_OP(FSUB2KX),         \
      DISPATCH_OP(IMUL1), DISPATCH_OP(IMUL2), DISPATCH_OP(IMUL1K),            \
      DISPATCH_OP(IMUL2K), DISPATCH_OP(FMUL1), DISPATCH_OP(FMUL2),            \
      DISPATCH_OP(FMUL1K), DISPATCH_OP(FMUL2K), DISPATCH_OP(IDIV1),           \
      DISPATCH_OP(IDIV2), DISPATCH_OP(IDIV1K), DISPATCH_OP(IDIV2K),           \
      DISPATCH_OP(IDIV2KX), DISPATCH_OP(FDIV1), DISPATCH_OP(FDIV2),           \
      DISPATCH_OP(FDIV1K), DISPATCH_OP(FDIV2K), DISPATCH_OP(FDIV2KX),         \
      DISPATCH_OP(IPOW1), DISPATCH_OP(IPOW2), DISPATCH_OP(IPOW1K),            \
      DISPATCH_OP(IPOW2K), DISPATCH_OP(IPOW2KX), DISPATCH_OP(FPOW1),          \
      DISPATCH_OP(FPOW2), DISPATCH_OP(FPOW1K), DISPATCH_OP(FPOW2K),           \
      DISPATCH_OP(FPOW2KX), DISPATCH_OP(IMOD1), DISPATCH_OP(IMOD2),           \
      DISPATCH_OP(IMOD1K), DISPATCH_OP(IMOD2K), DISPATCH_OP(IMOD2KX),         \
      DISPATCH_OP(FMOD1), DISPATCH_OP(FMOD2), DISPATCH_OP(FMOD1K),            \
      DISPATCH_OP(FMOD2K), DISPATCH_OP(FMOD2KX), DISPATCH_OP(INEG),           \
      DISPATCH_OP(INEGK), DISPATCH_OP(FNEG), DISPATCH_OP(FNEGK),              \
      DISPATCH_OP(BAND1), DISPATCH_OP(BAND2), DISPATCH_OP(BAND1K),            \
      DISPATCH_OP(BAND2K), DISPATCH_OP(BOR1), DISPATCH_OP(BOR2),              \
      DISPATCH_OP(BOR1K), DISPATCH_OP(BOR2K), DISPATCH_OP(BXOR1),             \
      DISPATCH_OP(BXOR2), DISPATCH_OP(BXOR1K), DISPATCH_OP(BXOR2K),           \
      DISPATCH_OP(BSHL1), DISPATCH_OP(BSHL2), DISPATCH_OP(BSHL1K),            \
      DISPATCH_OP(BSHL2K), DISPATCH_OP(BSHR1), DISPATCH_OP(BSHR2),            \
      DISPATCH_OP(BSHR1K), DISPATCH_OP(BSHR2K), DISPATCH_OP(BNOT),            \
      DISPATCH_OP(BNOTK), DISPATCH_OP(AND), DISPATCH_OP(ANDK),                \
      DISPATCH_OP(OR), DISPATCH_OP(ORK), DISPATCH_OP(IEQ), DISPATCH_OP(IEQK), \
      DISPATCH_OP(FEQ), DISPATCH_OP(FEQK), DISPATCH_OP(BEQ),                  \
      DISPATCH_OP(BEQK), DISPATCH_OP(SEQ), DISPATCH_OP(SEQK),                 \
      DISPATCH_OP(INEQ), DISPATCH_OP(INEQK), DISPATCH_OP(FNEQ),               \
      DISPATCH_OP(FNEQK), DISPATCH_OP(BNEQ), DISPATCH_OP(BNEQK),              \
      DISPATCH_OP(SNEQ), DISPATCH_OP(SNEQK), DISPATCH_OP(IS),                 \
      DISPATCH_OP(ILT), DISPATCH_OP(ILTK), DISPATCH_OP(FLT),                  \
      DISPATCH_OP(FLTK), DISPATCH_OP(IGT), DISPATCH_OP(IGTK),                 \
      DISPATCH_OP(FGT), DISPATCH_OP(FGTK), DISPATCH_OP(ILTEQ),                \
      DISPATCH_OP(ILTEQK), DISPATCH_OP(FLTEQ), DISPATCH_OP(FLTEQK),           \
      DISPATCH_OP(IGTEQ), DISPATCH_OP(IGTEQK), DISPATCH_OP(FGTEQ),            \
      DISPATCH_OP(FGTEQK), DISPATCH_OP(NOT), DISPATCH_OP(JMP),                \
      DISPATCH_OP(JMPIF), DISPATCH_OP(PUSH), DISPATCH_OP(PUSHK),              \
      DISPATCH_OP(GETARG), DISPATCH_OP(GETARGREF), DISPATCH_OP(SETARG),       \
      DISPATCH_OP(GETLOCAL), DISPATCH_OP(GETLOCALREF), DISPATCH_OP(SETLOCAL), \
      DISPATCH_OP(BTOI), DISPATCH_OP(FTOI), DISPATCH_OP(STOI),                \
      DISPATCH_OP(ITOF), DISPATCH_OP(BTOF), DISPATCH_OP(STOF),                \
      DISPATCH_OP(ITOB), DISPATCH_OP(STOB), DISPATCH_OP(ITOS),                \
      DISPATCH_OP(FTOS), DISPATCH_OP(BTOS), DISPATCH_OP(ARTOS),               \
      DISPATCH_OP(DTTOS), DISPATCH_OP(FNTOS), DISPATCH_OP(CAPTURE),           \
      DISPATCH_OP(CALL), DISPATCH_OP(PCALL), DISPATCH_OP(RET),                \
      DISPATCH_OP(RETNIL), DISPATCH_OP(RETTRUE), DISPATCH_OP(RETFALSE),       \
      DISPATCH_OP(RETK), DISPATCH_OP(GETUPV), DISPATCH_OP(GETUPVREF),         \
      DISPATCH_OP(SETUPV), DISPATCH_OP(ARRGET), DISPATCH_OP(ARRSET),          \
      DISPATCH_OP(ARRGETLEN), DISPATCH_OP(DICTGET), DISPATCH_OP(DICTSET),     \
      DISPATCH_OP(DICTGETLEN), DISPATCH_OP(NEWINSTANCE),                      \
      DISPATCH_OP(GETSUPER), DISPATCH_OP(GETSTATIC), DISPATCH_OP(GETDYNAMIC), \
      DISPATCH_OP(SETSTATIC), DISPATCH_OP(SETDYNAMIC),                        \
      DISPATCH_OP(CALLSTATIC), DISPATCH_OP(PCALLSTATIC),                      \
      DISPATCH_OP(CALLDYNAMIC), DISPATCH_OP(PCALLDYNAMIC)
#else
#define CASE(op) case op:
#endif

#define K get_constant

#define L(id) reinterpret_cast<Value*>(stack.at(id))
#define LSET(id, val) *stack.at(id) = reinterpret_cast<uptr>(val);
#define LFREE(id)    \
  if (R(id) != NULL) \
    reinterpret_cast<Value*>(stack.at(id))->rc--;

#define R(id) regs.data[id]
#define RSET(id, val) regs.data[id] = val
#define RFREE(id)               \
  [[likely]] if (R(id) != NULL) \
    R(id)->rc--;

void Interpreter::execute() {
  using enum Opcode;

#ifdef HAS_CGOTO
#ifdef VIA_COMPILER_GCC
  [[gnu::aligned(64)]]
#endif
  static void* dispatch_table[] = {DISPATCH_TABLE()};
#endif

dispatch:
  // Explicit CSE
  const u16 a = pc->a, b = pc->b, c = pc->c;
#ifdef HAS_CGOTO
  goto* dispatch_table[(usize)pc->op];
  {
#else
  switch (pc->op) {
#endif
    CASE(NOP) {
      goto dispatch;
    }
    CASE(HALT) {
      goto exit;
    }
    CASE(EXTRAARG1)
    CASE(EXTRAARG2)
    CASE(EXTRAARG3) {
      VIA_BUG("use of reserved opcode");
    }
    CASE(MOVE) {
      RFREE(a);
      RSET(a, R(b));
      RSET(b, NULL);
      goto dispatch;
    }
    CASE(XCHG) {
      Value* ra = R(a);
      RSET(a, R(b));
      RSET(b, ra);
      goto dispatch;
    }
    CASE(COPY) {
      RFREE(a);
      RSET(a, R(b)->clone());
      goto dispatch;
    }
    CASE(COPYREF) {
      RFREE(a);
      RSET(a, R(b));
      goto dispatch;
    }
    CASE(LOADTRUE) {
      RFREE(a);
      RSET(a, Value::construct(this, true));
      goto dispatch;
    }
    CASE(LOADFALSE) {
      RFREE(a);
      RSET(a, Value::construct(this, false));
      goto dispatch;
    }
    CASE(NEWSTR)
    CASE(NEWSTR2)
    CASE(NEWARR)
    CASE(NEWARR2)
    CASE(NEWDICT)
    CASE(NEWTUPLE)
    CASE(NEWCLOSURE) {
      VIA_TODO();
    }
    CASE(IADD1) {
      R(a)->u.int_ += R(b)->u.int_;
      goto dispatch;
    }
    CASE(IADD2) {
      R(a)->u.int_ = R(b)->u.int_ + R(c)->u.int_;
      goto dispatch;
    }
    CASE(IADD1K) {
      R(a)->u.int_ += K(b)->u.int_;
      goto dispatch;
    }
    CASE(IADD2K) {
      R(a)->u.int_ = R(b)->u.int_ + K(c)->u.int_;
      goto dispatch;
    }
    CASE(FADD1) {
      R(a)->u.float_ += R(b)->u.float_;
      goto dispatch;
    }
    CASE(FADD2) {
      R(a)->u.float_ = R(b)->u.float_ + R(c)->u.float_;
      goto dispatch;
    }
    CASE(FADD1K) {
      R(a)->u.float_ += K(b)->u.float_;
      goto dispatch;
    }
    CASE(FADD2K) {
      R(a)->u.float_ = R(b)->u.float_ + K(c)->u.float_;
      goto dispatch;
    }
    CASE(ISUB1) {
      R(a)->u.int_ -= R(b)->u.int_;
      goto dispatch;
    }
    CASE(ISUB2) {
      R(a)->u.int_ = R(b)->u.int_ - R(c)->u.int_;
      goto dispatch;
    }
    CASE(ISUB1K) {
      R(a)->u.int_ -= K(b)->u.int_;
      goto dispatch;
    }
    CASE(ISUB2K) {
      R(a)->u.int_ = R(b)->u.int_ - K(c)->u.int_;
      goto dispatch;
    }
    CASE(ISUB2KX) {
      R(a)->u.int_ = K(b)->u.int_ - R(c)->u.int_;
      goto dispatch;
    }
    CASE(FSUB1) {
      R(a)->u.float_ -= R(b)->u.float_;
      goto dispatch;
    }
    CASE(FSUB2) {
      R(a)->u.float_ = R(b)->u.float_ - R(c)->u.float_;
      goto dispatch;
    }
    CASE(FSUB1K) {
      R(a)->u.float_ -= K(b)->u.float_;
      goto dispatch;
    }
    CASE(FSUB2K) {
      R(a)->u.float_ = R(b)->u.float_ - K(c)->u.float_;
      goto dispatch;
    }
    CASE(FSUB2KX) {
      R(a)->u.float_ = K(b)->u.float_ - R(c)->u.float_;
      goto dispatch;
    }
    CASE(IMUL1) {
      R(a)->u.int_ *= R(b)->u.int_;
      goto dispatch;
    }
    CASE(IMUL2) {
      R(a)->u.int_ = R(b)->u.int_ * R(c)->u.int_;
      goto dispatch;
    }
    CASE(IMUL1K) {
      R(a)->u.int_ *= K(b)->u.int_;
      goto dispatch;
    }
    CASE(IMUL2K) {
      R(a)->u.int_ = R(b)->u.int_ * K(c)->u.int_;
      goto dispatch;
    }
    CASE(FMUL1) {
      R(a)->u.float_ *= R(b)->u.float_;
      goto dispatch;
    }
    CASE(FMUL2) {
      R(a)->u.float_ = R(b)->u.float_ * R(c)->u.float_;
      goto dispatch;
    }
    CASE(FMUL1K) {
      R(a)->u.float_ *= K(b)->u.float_;
      goto dispatch;
    }
    CASE(FMUL2K) {
      R(a)->u.float_ = R(b)->u.float_ * K(c)->u.float_;
      goto dispatch;
    }
    CASE(IDIV1) {
      R(a)->u.int_ /= R(b)->u.int_;
      goto dispatch;
    }
    CASE(IDIV2) {
      R(a)->u.int_ = R(b)->u.int_ / R(c)->u.int_;
      goto dispatch;
    }
    CASE(IDIV1K) {
      R(a)->u.int_ /= K(b)->u.int_;
      goto dispatch;
    }
    CASE(IDIV2K) {
      R(a)->u.int_ = R(b)->u.int_ / K(c)->u.int_;
      goto dispatch;
    }
    CASE(IDIV2KX) {
      R(a)->u.int_ = K(b)->u.int_ / R(c)->u.int_;
      goto dispatch;
    }
    CASE(FDIV1) {
      R(a)->u.float_ /= R(b)->u.float_;
      goto dispatch;
    }
    CASE(FDIV2) {
      R(a)->u.float_ = R(b)->u.float_ / R(c)->u.float_;
      goto dispatch;
    }
    CASE(FDIV1K) {
      R(a)->u.float_ /= K(b)->u.float_;
      goto dispatch;
    }
    CASE(FDIV2K) {
      R(a)->u.float_ = R(b)->u.float_ / K(c)->u.float_;
      goto dispatch;
    }
    CASE(FDIV2KX) {
      R(a)->u.float_ = K(b)->u.float_ / R(c)->u.float_;
      goto dispatch;
    }
    CASE(IPOW1) {
      R(a)->u.int_ = ipow(R(a)->u.int_, R(b)->u.int_);
      goto dispatch;
    }
    CASE(IPOW2) {
      R(a)->u.int_ = ipow(R(b)->u.int_, R(c)->u.int_);
      goto dispatch;
    }
    CASE(IPOW1K) {
      R(a)->u.int_ = ipow(R(a)->u.int_, K(b)->u.int_);
      goto dispatch;
    }
    CASE(IPOW2K) {
      R(a)->u.int_ = ipow(R(b)->u.int_, K(c)->u.int_);
      goto dispatch;
    }
    CASE(IPOW2KX) {
      R(a)->u.int_ = ipow(K(b)->u.int_, R(c)->u.int_);
      goto dispatch;
    }
    CASE(FPOW1) {
      R(a)->u.float_ = std::pow(R(a)->u.float_, R(b)->u.float_);
      goto dispatch;
    }
    CASE(FPOW2) {
      R(a)->u.float_ = std::pow(R(b)->u.float_, R(c)->u.float_);
      goto dispatch;
    }
    CASE(FPOW1K) {
      R(a)->u.float_ = std::pow(R(a)->u.float_, K(b)->u.float_);
      goto dispatch;
    }
    CASE(FPOW2K) {
      R(a)->u.float_ = std::pow(R(b)->u.float_, K(c)->u.float_);
      goto dispatch;
    }
    CASE(FPOW2KX) {
      R(a)->u.float_ = std::pow(K(b)->u.float_, R(c)->u.float_);
      goto dispatch;
    }
    CASE(IMOD1) {
      R(a)->u.int_ = R(a)->u.int_ % R(b)->u.int_;
      goto dispatch;
    }
    CASE(IMOD2) {
      R(a)->u.int_ = R(b)->u.int_ % R(c)->u.int_;
      goto dispatch;
    }
    CASE(IMOD1K) {
      R(a)->u.int_ = R(a)->u.int_ % K(b)->u.int_;
      goto dispatch;
    }
    CASE(IMOD2K) {
      R(a)->u.int_ = R(b)->u.int_ % K(c)->u.int_;
      goto dispatch;
    }
    CASE(IMOD2KX) {
      R(a)->u.int_ = K(b)->u.int_ % R(c)->u.int_;
      goto dispatch;
    }
    CASE(FMOD1) {
      R(a)->u.float_ = std::fmod(R(a)->u.float_, R(b)->u.float_);
      goto dispatch;
    }
    CASE(FMOD2) {
      R(a)->u.float_ = std::fmod(R(b)->u.float_, R(c)->u.float_);
      goto dispatch;
    }
    CASE(FMOD1K) {
      R(a)->u.float_ = std::fmod(R(a)->u.float_, K(b)->u.float_);
      goto dispatch;
    }
    CASE(FMOD2K) {
      R(a)->u.float_ = std::fmod(R(b)->u.float_, K(c)->u.float_);
      goto dispatch;
    }
    CASE(FMOD2KX) {
      R(a)->u.float_ = std::fmod(K(b)->u.float_, R(c)->u.float_);
      goto dispatch;
    }
    CASE(INEG) {
      R(a)->u.int_ = -R(b)->u.int_;
      goto dispatch;
    }
    CASE(INEGK) {
      R(a)->u.int_ = -K(b)->u.int_;
      goto dispatch;
    }
    CASE(FNEG) {
      R(a)->u.float_ = -R(b)->u.float_;
      goto dispatch;
    }
    CASE(FNEGK) {
      R(a)->u.float_ = -K(b)->u.float_;
      goto dispatch;
    }
    CASE(BAND1) {
      R(a)->u.int_ &= R(b)->u.int_;
      goto dispatch;
    }
    CASE(BAND2) {
      R(a)->u.int_ = R(b)->u.int_ & R(c)->u.int_;
      goto dispatch;
    }
    CASE(BAND1K) {
      R(a)->u.int_ &= K(b)->u.int_;
      goto dispatch;
    }
    CASE(BAND2K) {
      R(a)->u.int_ = R(b)->u.int_ & K(c)->u.int_;
      goto dispatch;
    }
    CASE(BOR1) {
      R(a)->u.int_ |= R(b)->u.int_;
      goto dispatch;
    }
    CASE(BOR2) {
      R(a)->u.int_ = R(b)->u.int_ | R(c)->u.int_;
      goto dispatch;
    }
    CASE(BOR1K) {
      R(a)->u.int_ |= K(b)->u.int_;
      goto dispatch;
    }
    CASE(BOR2K) {
      R(a)->u.int_ = R(b)->u.int_ | K(c)->u.int_;
      goto dispatch;
    }
    CASE(BXOR1) {
      R(a)->u.int_ ^= R(b)->u.int_;
      goto dispatch;
    }
    CASE(BXOR2) {
      R(a)->u.int_ = R(b)->u.int_ ^ R(c)->u.int_;
      goto dispatch;
    }
    CASE(BXOR1K) {
      R(a)->u.int_ ^= K(b)->u.int_;
      goto dispatch;
    }
    CASE(BXOR2K) {
      R(a)->u.int_ = R(b)->u.int_ ^ K(c)->u.int_;
      goto dispatch;
    }
    CASE(BSHL1) {
      R(a)->u.int_ <<= R(b)->u.int_;
      goto dispatch;
    }
    CASE(BSHL2) {
      R(a)->u.int_ = R(b)->u.int_ << R(c)->u.int_;
      goto dispatch;
    }
    CASE(BSHL1K) {
      R(a)->u.int_ <<= K(b)->u.int_;
      goto dispatch;
    }
    CASE(BSHL2K) {
      R(a)->u.int_ = R(b)->u.int_ << K(c)->u.int_;
      goto dispatch;
    }
    CASE(BSHR1) {
      R(a)->u.int_ >>= R(b)->u.int_;
      goto dispatch;
    }
    CASE(BSHR2) {
      R(a)->u.int_ = R(b)->u.int_ >> R(c)->u.int_;
      goto dispatch;
    }
    CASE(BSHR1K) {
      R(a)->u.int_ >>= K(b)->u.int_;
      goto dispatch;
    }
    CASE(BSHR2K) {
      R(a)->u.int_ = R(b)->u.int_ >> K(c)->u.int_;
      goto dispatch;
    }
    CASE(BNOT) {
      R(a)->u.int_ = ~R(b)->u.int_;
      goto dispatch;
    }
    CASE(BNOTK) {
      R(a)->u.int_ = K(b)->u.int_;
      goto dispatch;
    }
    CASE(AND) {
      R(a)->u.boolean = R(b)->u.boolean && R(c)->u.boolean;
      goto dispatch;
    }
    CASE(ANDK) {
      R(a)->u.boolean = R(b)->u.boolean && K(c)->u.boolean;
      goto dispatch;
    }
    CASE(OR) {
      R(a)->u.boolean = R(b)->u.boolean || R(c)->u.boolean;
      goto dispatch;
    }
    CASE(ORK) {
      R(a)->u.boolean = R(b)->u.boolean || K(c)->u.boolean;
      goto dispatch;
    }
    CASE(IEQ) {
      R(a)->u.boolean = R(b)->u.int_ == R(c)->u.int_;
      goto dispatch;
    }
    CASE(IEQK) {
      R(a)->u.boolean = R(b)->u.int_ == K(c)->u.int_;
      goto dispatch;
    }
    CASE(FEQ) {
      R(a)->u.boolean = R(b)->u.float_ == R(c)->u.float_;
      goto dispatch;
    }
    CASE(FEQK) {
      R(a)->u.boolean = R(b)->u.float_ == K(c)->u.float_;
      goto dispatch;
    }
    CASE(BEQ) {
      R(a)->u.boolean = R(b)->u.boolean == R(c)->u.boolean;
      goto dispatch;
    }
    CASE(BEQK) {
      R(a)->u.boolean = R(b)->u.boolean == K(c)->u.boolean;
      goto dispatch;
    }
    CASE(SEQ) {
      R(a)->u.boolean = std::strcmp(R(b)->u.string, R(c)->u.string) == 0;
      goto dispatch;
    }
    CASE(SEQK) {
      R(a)->u.boolean = std::strcmp(R(b)->u.string, K(c)->u.string) == 0;
      goto dispatch;
    }
    CASE(INEQ) {
      R(a)->u.boolean = R(b)->u.int_ != R(c)->u.int_;
      goto dispatch;
    }
    CASE(INEQK) {
      R(a)->u.boolean = R(b)->u.int_ != K(c)->u.int_;
      goto dispatch;
    }
    CASE(FNEQ) {
      R(a)->u.boolean = R(b)->u.float_ != R(c)->u.float_;
      goto dispatch;
    }
    CASE(FNEQK) {
      R(a)->u.boolean = R(b)->u.float_ != K(c)->u.float_;
      goto dispatch;
    }
    CASE(BNEQ) {
      R(a)->u.boolean = R(b)->u.boolean != R(c)->u.boolean;
      goto dispatch;
    }
    CASE(BNEQK) {
      R(a)->u.boolean = R(b)->u.boolean != K(c)->u.boolean;
      goto dispatch;
    }
    CASE(SNEQ) {
      R(a)->u.boolean = std::strcmp(R(b)->u.string, R(c)->u.string) == 1;
      goto dispatch;
    }
    CASE(SNEQK) {
      R(a)->u.boolean = std::strcmp(R(b)->u.string, K(c)->u.string) == 1;
      goto dispatch;
    }
    CASE(IS) {
      R(a)->u.boolean = R(b) == R(c);
      goto dispatch;
    }
    CASE(ILT) {
      R(a)->u.boolean = R(b)->u.int_ < R(c)->u.int_;
      goto dispatch;
    }
    CASE(ILTK) {
      R(a)->u.boolean = R(b)->u.int_ < K(c)->u.int_;
      goto dispatch;
    }
    CASE(FLT) {
      R(a)->u.boolean = R(b)->u.float_ < R(c)->u.float_;
      goto dispatch;
    }
    CASE(FLTK) {
      R(a)->u.boolean = R(b)->u.float_ < K(c)->u.float_;
      goto dispatch;
    }
    CASE(IGT) {
      R(a)->u.boolean = R(b)->u.int_ > R(c)->u.int_;
      goto dispatch;
    }
    CASE(IGTK) {
      R(a)->u.boolean = R(b)->u.int_ > K(c)->u.int_;
      goto dispatch;
    }
    CASE(FGT) {
      R(a)->u.boolean = R(b)->u.float_ > R(c)->u.float_;
      goto dispatch;
    }
    CASE(FGTK) {
      R(a)->u.boolean = R(b)->u.float_ > K(c)->u.float_;
      goto dispatch;
    }
    CASE(ILTEQ) {
      R(a)->u.boolean = R(b)->u.int_ <= R(c)->u.int_;
      goto dispatch;
    }
    CASE(ILTEQK) {
      R(a)->u.boolean = R(b)->u.int_ <= K(c)->u.int_;
      goto dispatch;
    }
    CASE(FLTEQ) {
      R(a)->u.boolean = R(b)->u.float_ <= R(c)->u.float_;
      goto dispatch;
    }
    CASE(FLTEQK) {
      R(a)->u.boolean = R(b)->u.float_ <= K(c)->u.float_;
      goto dispatch;
    }
    CASE(IGTEQ) {
      R(a)->u.boolean = R(b)->u.int_ >= R(c)->u.int_;
      goto dispatch;
    }
    CASE(IGTEQK) {
      R(a)->u.boolean = R(b)->u.int_ >= K(c)->u.int_;
      goto dispatch;
    }
    CASE(FGTEQ) {
      R(a)->u.boolean = R(b)->u.float_ >= R(c)->u.float_;
      goto dispatch;
    }
    CASE(FGTEQK) {
      R(a)->u.boolean = R(b)->u.float_ >= K(c)->u.float_;
      goto dispatch;
    }
    CASE(NOT) {
      R(a)->u.boolean = !R(b)->u.boolean;
      goto dispatch;
    }
    CASE(JMP) {
      pc = lbt.data[a];
      goto dispatch;
    }
    CASE(JMPIF) {
      if (R(a)->as_cbool())
        pc = lbt.data[b];
      goto dispatch;
    }
    CASE(PUSH) {
      new_local(R(a)->make_ref());
      goto dispatch;
    }
    CASE(PUSHK) {
      new_local(K(a));
      goto dispatch;
    }
    CASE(GETARG)
    CASE(GETARGREF)
    CASE(SETARG) {
      VIA_TODO();
    }
    CASE(GETLOCAL) {
      RFREE(a);
      RSET(a, L(b)->clone());
      goto dispatch;
    }
    CASE(GETLOCALREF) {
      RFREE(a);
      RSET(a, L(b));
      goto dispatch;
    }
    CASE(SETLOCAL) {
      LFREE(b);
      LSET(b, R(a));
      goto dispatch;
    }
    CASE(BTOI)
    CASE(FTOI)
    CASE(STOI)
    CASE(ITOF)
    CASE(BTOF)
    CASE(STOF)
    CASE(ITOB)
    CASE(STOB)
    CASE(ITOS)
    CASE(FTOS)
    CASE(BTOS)
    CASE(ARTOS)
    CASE(DTTOS)
    CASE(FNTOS)
    CASE(CAPTURE)
    CASE(CALL)
    CASE(PCALL)
    CASE(RET)
    CASE(RETNIL)
    CASE(RETTRUE)
    CASE(RETFALSE)
    CASE(RETK)
    CASE(GETUPV)
    CASE(GETUPVREF)
    CASE(SETUPV)
    CASE(ARRGET)
    CASE(ARRSET)
    CASE(ARRGETLEN)
    CASE(DICTGET)
    CASE(DICTSET)
    CASE(DICTGETLEN)
    CASE(NEWINSTANCE)
    CASE(GETSUPER)
    CASE(GETSTATIC)
    CASE(GETDYNAMIC)
    CASE(SETSTATIC)
    CASE(SETDYNAMIC)
    CASE(CALLSTATIC)
    CASE(PCALLSTATIC)
    CASE(CALLDYNAMIC)
    CASE(PCALLDYNAMIC) {
      VIA_TODO();
    }
#ifdef HAS_CGOTO
  }
#else
    default:
      VIA_BUG("unknown opcode");
      break;
  }
#endif
exit:
}

}  // namespace via
