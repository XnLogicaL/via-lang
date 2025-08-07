// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "interpreter.h"
#include "value.h"

namespace via {

namespace core {

namespace vm {

template <typename T>
  requires std::is_integral_v<T>
static inline T ipow(T base, T exp) {
  int result = 1;
  for (;;) {
    if (exp & 1)
      result *= base;
    exp >>= 1;
    if (!exp)
      break;
    base *= base;
  }

  return result;
}

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
  regs.data[reg] = val.ptr;
}

ValueRef Interpreter::new_local(ValueRef val) {
  stack.push((uptr)val.ptr);
  return get_local(stack.size() - 1);
}

void Interpreter::set_local(usize sp, ValueRef val) {
  *stack.at(sp) = reinterpret_cast<uptr>(val.ptr);
}

ValueRef Interpreter::get_local(usize sp) {
  return reinterpret_cast<Value*>(stack.at(sp))->make_ref();
}

#define R(id) get_register(id)
#define K(id) get_constant(id)

void Interpreter::execute() {
  using enum Opcode;

dispatch:
  switch (pc->op) {
    case NOP:
      goto dispatch;
    case HALT:
      goto exit;
    case EXTRAARG1:
    case EXTRAARG2:
    case EXTRAARG3:
      VIA_BUG("use of reserved opcode");
    case IADD1:
      R(pc->a)->u.i += R(pc->b)->u.i;
      goto dispatch;
    case IADD2:
      R(pc->a)->u.i = R(pc->b)->u.i + R(pc->c)->u.i;
      goto dispatch;
    case IADD1K:
      R(pc->a)->u.i += K(pc->b)->u.i;
      goto dispatch;
    case IADD2K:
      R(pc->a)->u.i = R(pc->b)->u.i + K(pc->c)->u.i;
      goto dispatch;
    case FADD1:
      R(pc->a)->u.fp += R(pc->b)->u.fp;
      goto dispatch;
    case FADD2:
      R(pc->a)->u.fp = R(pc->b)->u.fp + R(pc->c)->u.fp;
      goto dispatch;
    case FADD1K:
      R(pc->a)->u.fp += K(pc->b)->u.fp;
      goto dispatch;
    case FADD2K:
      R(pc->a)->u.fp = R(pc->b)->u.fp + K(pc->c)->u.fp;
      goto dispatch;
    case ISUB1:
      R(pc->a)->u.i -= R(pc->b)->u.i;
      goto dispatch;
    case ISUB2:
      R(pc->a)->u.i = R(pc->b)->u.i - R(pc->c)->u.i;
      goto dispatch;
    case ISUB1K:
      R(pc->a)->u.i -= K(pc->b)->u.i;
      goto dispatch;
    case ISUB2K:
      R(pc->a)->u.i = R(pc->b)->u.i - K(pc->c)->u.i;
      goto dispatch;
    case ISUB2KX:
      R(pc->a)->u.i = K(pc->b)->u.i - R(pc->c)->u.i;
      goto dispatch;
    case FSUB1:
      R(pc->a)->u.fp -= R(pc->b)->u.fp;
      goto dispatch;
    case FSUB2:
      R(pc->a)->u.fp = R(pc->b)->u.fp - R(pc->c)->u.fp;
      goto dispatch;
    case FSUB1K:
      R(pc->a)->u.fp -= K(pc->b)->u.fp;
      goto dispatch;
    case FSUB2K:
      R(pc->a)->u.fp = R(pc->b)->u.fp - K(pc->c)->u.fp;
      goto dispatch;
    case FSUB2KX:
      R(pc->a)->u.fp = K(pc->b)->u.fp - R(pc->c)->u.fp;
      goto dispatch;
    case IMUL1:
      R(pc->a)->u.i *= R(pc->b)->u.i;
      goto dispatch;
    case IMUL2:
      R(pc->a)->u.i = R(pc->b)->u.i * R(pc->c)->u.i;
      goto dispatch;
    case IMUL1K:
      R(pc->a)->u.i *= K(pc->b)->u.i;
      goto dispatch;
    case IMUL2K:
      R(pc->a)->u.i = R(pc->b)->u.i * K(pc->c)->u.i;
      goto dispatch;
    case FMUL1:
      R(pc->a)->u.fp *= R(pc->b)->u.fp;
      goto dispatch;
    case FMUL2:
      R(pc->a)->u.fp = R(pc->b)->u.fp * R(pc->c)->u.fp;
      goto dispatch;
    case FMUL1K:
      R(pc->a)->u.fp *= K(pc->b)->u.fp;
      goto dispatch;
    case FMUL2K:
      R(pc->a)->u.fp = R(pc->b)->u.fp * K(pc->c)->u.fp;
      goto dispatch;
    case IDIV1:
      R(pc->a)->u.i /= R(pc->b)->u.i;
      goto dispatch;
    case IDIV2:
      R(pc->a)->u.i = R(pc->b)->u.i / R(pc->c)->u.i;
      goto dispatch;
    case IDIV1K:
      R(pc->a)->u.i /= K(pc->b)->u.i;
      goto dispatch;
    case IDIV2K:
      R(pc->a)->u.i = R(pc->b)->u.i / K(pc->c)->u.i;
      goto dispatch;
    case IDIV2KX:
      R(pc->a)->u.i = K(pc->b)->u.i / R(pc->c)->u.i;
      goto dispatch;
    case FDIV1:
      R(pc->a)->u.fp /= R(pc->b)->u.fp;
      goto dispatch;
    case FDIV2:
      R(pc->a)->u.fp = R(pc->b)->u.fp / R(pc->c)->u.fp;
      goto dispatch;
    case FDIV1K:
      R(pc->a)->u.fp /= K(pc->b)->u.fp;
      goto dispatch;
    case FDIV2K:
      R(pc->a)->u.fp = R(pc->b)->u.fp / K(pc->c)->u.fp;
      goto dispatch;
    case FDIV2KX:
      R(pc->a)->u.fp = K(pc->b)->u.fp / R(pc->c)->u.fp;
      goto dispatch;
    case IPOW1:
      R(pc->a)->u.i = ipow(R(pc->a)->u.i, R(pc->b)->u.i);
      goto dispatch;
    case IPOW2:
      R(pc->a)->u.i = ipow(R(pc->b)->u.i, R(pc->c)->u.i);
      goto dispatch;
    case IPOW1K:
      R(pc->a)->u.i = ipow(R(pc->a)->u.i, K(pc->b)->u.i);
      goto dispatch;
    case IPOW2K:
      R(pc->a)->u.i = ipow(R(pc->b)->u.i, K(pc->c)->u.i);
      goto dispatch;
    case IPOW2KX:
      R(pc->a)->u.i = ipow(K(pc->b)->u.i, R(pc->c)->u.i);
      goto dispatch;
    case FPOW1:
      R(pc->a)->u.fp = std::pow(R(pc->a)->u.fp, R(pc->b)->u.fp);
      goto dispatch;
    case FPOW2:
      R(pc->a)->u.fp = std::pow(R(pc->b)->u.fp, R(pc->c)->u.fp);
      goto dispatch;
    case FPOW1K:
      R(pc->a)->u.fp = std::pow(R(pc->a)->u.fp, K(pc->b)->u.fp);
      goto dispatch;
    case FPOW2K:
      R(pc->a)->u.fp = std::pow(R(pc->b)->u.fp, K(pc->c)->u.fp);
      goto dispatch;
    case FPOW2KX:
      R(pc->a)->u.fp = std::pow(K(pc->b)->u.fp, R(pc->c)->u.fp);
      goto dispatch;
    case IMOD1:
      R(pc->a)->u.i = R(pc->a)->u.i % R(pc->b)->u.i;
      goto dispatch;
    case IMOD2:
      R(pc->a)->u.i = R(pc->b)->u.i % R(pc->c)->u.i;
      goto dispatch;
    case IMOD1K:
      R(pc->a)->u.i = R(pc->a)->u.i % K(pc->b)->u.i;
      goto dispatch;
    case IMOD2K:
      R(pc->a)->u.i = R(pc->b)->u.i % K(pc->c)->u.i;
      goto dispatch;
    case IMOD2KX:
      R(pc->a)->u.i = K(pc->b)->u.i % R(pc->c)->u.i;
      goto dispatch;
    case FMOD1:
      R(pc->a)->u.fp = std::fmod(R(pc->a)->u.fp, R(pc->b)->u.fp);
      goto dispatch;
    case FMOD2:
      R(pc->a)->u.fp = std::fmod(R(pc->b)->u.fp, R(pc->c)->u.fp);
      goto dispatch;
    case FMOD1K:
      R(pc->a)->u.fp = std::fmod(R(pc->a)->u.fp, K(pc->b)->u.fp);
      goto dispatch;
    case FMOD2K:
      R(pc->a)->u.fp = std::fmod(R(pc->b)->u.fp, K(pc->c)->u.fp);
      goto dispatch;
    case FMOD2KX:
      R(pc->a)->u.fp = std::fmod(K(pc->b)->u.fp, R(pc->c)->u.fp);
      goto dispatch;
    case INEG:
      R(pc->a)->u.i = -R(pc->b)->u.i;
      goto dispatch;
    case INEGK:
      R(pc->a)->u.i = -K(pc->b)->u.i;
      goto dispatch;
    case FNEG:
      R(pc->a)->u.fp = -R(pc->b)->u.fp;
      goto dispatch;
    case FNEGK:
      R(pc->a)->u.fp = -K(pc->b)->u.fp;
      goto dispatch;
    default:
      VIA_BUG("unknown opcode");
      break;
  }
exit:
}

}  // namespace vm

}  // namespace core

}  // namespace via
