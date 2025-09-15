/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "interpreter.h"
#include "value.h"
#include "value_ref.h"

#if defined(VIA_COMPILER_GCC) || defined(VIA_COMPILER_CLANG)
  #define HAS_CGOTO
#endif

#ifdef HAS_CGOTO
  #define CASE(OP) OP_##OP:
  #define DISPATCH_OP(OP) &&OP_##OP
#else
  #define CASE(OP) case OpCode::OP:
#endif

#define DISPATCH()             \
  {                            \
    if constexpr (!OverridePC) \
      pc++;                    \
    if constexpr (SingleStep)  \
      goto exit;               \
    else                       \
      goto dispatch;           \
  }

#define K interp->getConstant

#define L(ID) reinterpret_cast<Value*>(stack.at(ID))
#define LSET(ID, VAL) *stack.at(ID) = reinterpret_cast<uptr>(VAL);
#define LFREE(ID)       \
  if (R(ID) != nullptr) \
    reinterpret_cast<Value*>(stack.at(ID))->mRc--;

#define R(ID) regs[ID]
#define RSET(ID, VAL) regs[ID] = VAL
#define RFREE(ID)                  \
  [[likely]] if (R(ID) != nullptr) \
    R(ID)->mRc--;

// TODO: This macro does not account for value tags, it might mess up debug info
#define RASSIGN(ID, FIELD, EXPR)            \
  {                                         \
    RFREE(ID);                              \
    R(ID) = Value::construct(interp, EXPR); \
  }

namespace via
{

template <bool SingleStep, bool OverridePC>
void __execute(Interpreter* interp)
{
#ifdef HAS_CGOTO
  [[gnu::aligned(64)]] static void* dispatch_table[] = {
  #define X(OP) DISPATCH_OP(OP),
    OPCODE_LIST
  #undef X
  };
#endif

dispatch:
  // Explicit CSE
  auto& stack = interp->mStack;
  auto& regs = interp->mRegisters;

  const auto*& pc = interp->pc;
  const u16 a = pc->a, b = pc->b, c = pc->c;
#ifdef HAS_CGOTO
  goto* dispatch_table[static_cast<u16>(pc->op)];
  {
#else
  switch (pc->op) {
#endif
    CASE(NOP)
    {
      DISPATCH();
    }
    CASE(HALT)
    {
      goto exit;
    }
    CASE(EXTRAARG1)
    CASE(EXTRAARG2)
    CASE(EXTRAARG3)
    {
      debug::bug("use of reserved opcode");
    }
    CASE(MOVE)
    {
      RFREE(a);
      RSET(a, R(b));
      RSET(b, nullptr);
      DISPATCH()
    }
    CASE(XCHG)
    {
      Value* ra = R(a);
      RSET(a, R(b));
      RSET(b, ra);
      DISPATCH()
    }
    CASE(COPY)
    {
      RFREE(a);
      RSET(a, R(b)->clone());
      DISPATCH()
    }
    CASE(COPYREF)
    {
      RFREE(a);
      RSET(a, R(b));
      DISPATCH()
    }
    CASE(LOADK)
    {
      RFREE(a);
      RSET(a, K(b)->clone());
      DISPATCH()
    }
    CASE(LOADTRUE)
    {
      RFREE(a);
      RSET(a, Value::construct(interp, true));
      DISPATCH()
    }
    CASE(LOADFALSE)
    {
      RFREE(a);
      RSET(a, Value::construct(interp, false));
      DISPATCH()
    }
    CASE(NEWSTR)
    CASE(NEWSTR2)
    CASE(NEWARR)
    CASE(NEWARR2)
    CASE(NEWDICT)
    CASE(NEWTUPLE)
    CASE(NEWCLOSURE)
    {
      debug::todo("implement opcodes");
    }
    CASE(ENDCLOSURE)
    {
      debug::bug("direct use of reserved opcode");
    }
    CASE(IADD)
    {
      RASSIGN(a, mData.int_, i64(R(b)->mData.int_ + R(c)->mData.int_));
      DISPATCH();
    }
    CASE(IADDK)
    {
      RASSIGN(a, mData.int_, i64(R(b)->mData.int_ + K(c)->mData.int_));
      DISPATCH();
    }
    CASE(FADD)
    {
      RASSIGN(a, mData.float_, f64(R(b)->mData.float_ + R(c)->mData.float_));
      DISPATCH();
    }
    CASE(FADDK)
    {
      RASSIGN(a, mData.float_, f64(R(b)->mData.float_ + K(c)->mData.float_));
      DISPATCH();
    }
    CASE(ISUB)
    {
      RASSIGN(a, mData.int_, i64(R(b)->mData.int_ - R(c)->mData.int_))
      DISPATCH();
    }
    CASE(ISUBK)
    {
      RASSIGN(a, mData.int_, i64(R(b)->mData.int_ - K(c)->mData.int_));
      DISPATCH();
    }
    CASE(FSUB)
    {
      RASSIGN(a, mData.float_, f64(R(b)->mData.float_ - R(c)->mData.float_));
      DISPATCH();
    }
    CASE(FSUBK)
    {
      RASSIGN(a, mData.float_, f64(R(b)->mData.float_ - K(c)->mData.float_));
      DISPATCH();
    }
    CASE(IMUL)
    {
      RASSIGN(a, mData.int_, i64(R(b)->mData.int_ * R(c)->mData.int_));
      DISPATCH();
    }
    CASE(IMULK)
    {
      RASSIGN(a, mData.int_, i64(R(b)->mData.int_ * K(c)->mData.int_));
      DISPATCH();
    }
    CASE(FMUL)
    {
      RASSIGN(a, mData.float_, f64(R(b)->mData.float_ * R(c)->mData.float_));
      DISPATCH();
    }
    CASE(FMULK)
    {
      RASSIGN(a, mData.float_, f64(R(b)->mData.float_ * K(c)->mData.float_));
      DISPATCH();
    }
    CASE(IDIV)
    {
      RASSIGN(a, mData.int_, i64(R(b)->mData.int_ / R(c)->mData.int_));
      DISPATCH();
    }
    CASE(IDIVK)
    {
      RASSIGN(a, mData.int_, i64(R(b)->mData.int_ / K(c)->mData.int_));
      DISPATCH();
    }
    CASE(FDIV)
    {
      RASSIGN(a, mData.float_, f64(R(b)->mData.float_ / R(c)->mData.float_));
      DISPATCH();
    }
    CASE(FDIVK)
    {
      RASSIGN(a, mData.float_, f64(R(b)->mData.float_ / K(c)->mData.float_));
      DISPATCH();
    }
    CASE(INEG)
    {
      RASSIGN(a, mData.int_, i64(-R(b)->mData.int_));
      DISPATCH();
    }
    CASE(INEGK)
    {
      RASSIGN(a, mData.int_, i64(-K(b)->mData.int_));
      DISPATCH();
    }
    CASE(FNEG)
    {
      RASSIGN(a, mData.float_, f64(-R(b)->mData.float_));
      DISPATCH();
    }
    CASE(FNEGK)
    {
      RASSIGN(a, mData.float_, f64(-K(b)->mData.float_));
      DISPATCH();
    }
    CASE(BAND)
    {
      RASSIGN(a, mData.int_, i64(R(b)->mData.int_ & R(c)->mData.int_))
      DISPATCH();
    }
    CASE(BANDK)
    {
      RASSIGN(a, mData.int_, i64(R(b)->mData.int_ & K(c)->mData.int_))
      DISPATCH();
    }
    CASE(BOR)
    {
      RASSIGN(a, mData.int_, i64(R(b)->mData.int_ | R(c)->mData.int_))
      DISPATCH();
    }
    CASE(BORK)
    {
      RASSIGN(a, mData.int_, i64(R(b)->mData.int_ | K(c)->mData.int_))
      DISPATCH();
    }
    CASE(BXOR)
    {
      RASSIGN(a, mData.int_, i64(R(b)->mData.int_ ^ R(c)->mData.int_))
      DISPATCH();
    }
    CASE(BXORK){RASSIGN(a, mData.int_, i64(R(b)->mData.int_ ^ K(c)->mData.int_))
                  DISPATCH()} CASE(BSHL)
    {
      RASSIGN(a, mData.int_, i64(R(b)->mData.int_ << R(c)->mData.int_));
      DISPATCH()
    }
    CASE(BSHLK)
    {
      RASSIGN(a, mData.int_, i64(R(b)->mData.int_ << K(c)->mData.int_));
      DISPATCH()
    }
    CASE(BSHR)
    {
      RASSIGN(a, mData.int_, i64(R(b)->mData.int_ >> R(c)->mData.int_));
      DISPATCH()
    }
    CASE(BSHRK)
    {
      RASSIGN(a, mData.int_, i64(R(b)->mData.int_ >> K(c)->mData.int_));
      DISPATCH()
    }
    CASE(BNOT)
    {
      RASSIGN(a, mData.int_, i64(~R(b)->mData.int_));
      DISPATCH()
    }
    CASE(BNOTK)
    {
      RASSIGN(a, mData.int_, i64(K(b)->mData.int_));
      DISPATCH()
    }
    CASE(AND)
    {
      RASSIGN(a, mData.boolean,
              bool(R(b)->mData.boolean && R(c)->mData.boolean));
      DISPATCH()
    }
    CASE(ANDK)
    {
      RASSIGN(a, mData.boolean,
              bool(R(b)->mData.boolean && K(c)->mData.boolean));
      DISPATCH()
    }
    CASE(OR)
    {
      RASSIGN(a, mData.boolean,
              bool(R(b)->mData.boolean || R(c)->mData.boolean));
      DISPATCH()
    }
    CASE(ORK)
    {
      RASSIGN(a, mData.boolean,
              bool(R(b)->mData.boolean || K(c)->mData.boolean));
      DISPATCH()
    }
    CASE(IEQ)
    {
      RASSIGN(a, mData.boolean, bool(R(b)->mData.int_ == R(c)->mData.int_));
      DISPATCH()
    }
    CASE(IEQK)
    {
      RASSIGN(a, mData.boolean, bool(R(b)->mData.int_ == K(c)->mData.int_));
      DISPATCH()
    }
    CASE(FEQ)
    {
      RASSIGN(a, mData.boolean, bool(R(b)->mData.float_ == R(c)->mData.float_));
      DISPATCH()
    }
    CASE(FEQK)
    {
      RASSIGN(a, mData.boolean, bool(R(b)->mData.float_ == K(c)->mData.float_));
      DISPATCH()
    }
    CASE(BEQ)
    {
      RASSIGN(a, mData.boolean,
              bool(R(b)->mData.boolean == R(c)->mData.boolean));
      DISPATCH()
    }
    CASE(BEQK)
    {
      RASSIGN(a, mData.boolean,
              bool(R(b)->mData.boolean == K(c)->mData.boolean));
      DISPATCH()
    }
    CASE(SEQ)
    {
      RASSIGN(a, mData.boolean,
              bool(std::strcmp(R(b)->mData.string, R(c)->mData.string) == 0));
      DISPATCH()
    }
    CASE(SEQK)
    {
      RASSIGN(a, mData.boolean,
              bool(std::strcmp(R(b)->mData.string, K(c)->mData.string) == 0));
      DISPATCH()
    }
    CASE(INEQ)
    {
      RASSIGN(a, mData.boolean, bool(R(b)->mData.int_ != R(c)->mData.int_));
      DISPATCH()
    }
    CASE(INEQK)
    {
      RASSIGN(a, mData.boolean, bool(R(b)->mData.int_ != K(c)->mData.int_));
      DISPATCH()
    }
    CASE(FNEQ)
    {
      RASSIGN(a, mData.boolean, bool(R(b)->mData.float_ != R(c)->mData.float_));
      DISPATCH()
    }
    CASE(FNEQK)
    {
      RASSIGN(a, mData.boolean, bool(R(b)->mData.float_ != K(c)->mData.float_));
      DISPATCH()
    }
    CASE(BNEQ)
    {
      RASSIGN(a, mData.boolean,
              bool(R(b)->mData.boolean != R(c)->mData.boolean));
      DISPATCH()
    }
    CASE(BNEQK)
    {
      RASSIGN(a, mData.boolean,
              bool(R(b)->mData.boolean != K(c)->mData.boolean));
      DISPATCH()
    }
    CASE(SNEQ)
    {
      RASSIGN(a, mData.boolean,
              bool(std::strcmp(R(b)->mData.string, R(c)->mData.string) == 1));
      DISPATCH()
    }
    CASE(SNEQK)
    {
      RASSIGN(a, mData.boolean,
              bool(std::strcmp(R(b)->mData.string, K(c)->mData.string) == 1));
      DISPATCH()
    }
    CASE(IS)
    {
      RASSIGN(a, mData.boolean, bool(R(b) == R(c)));
      DISPATCH()
    }
    CASE(ILT)
    {
      RASSIGN(a, mData.boolean, bool(R(b)->mData.int_ < R(c)->mData.int_));
      DISPATCH()
    }
    CASE(ILTK)
    {
      RASSIGN(a, mData.boolean, bool(R(b)->mData.int_ < K(c)->mData.int_));
      DISPATCH()
    }
    CASE(FLT)
    {
      RASSIGN(a, mData.boolean, bool(R(b)->mData.float_ < R(c)->mData.float_));
      DISPATCH()
    }
    CASE(FLTK)
    {
      RASSIGN(a, mData.boolean, bool(R(b)->mData.float_ < K(c)->mData.float_));
      DISPATCH()
    }
    CASE(IGT)
    {
      RASSIGN(a, mData.boolean, bool(R(b)->mData.int_ > R(c)->mData.int_));
      DISPATCH()
    }
    CASE(IGTK)
    {
      RASSIGN(a, mData.boolean, bool(R(b)->mData.int_ > K(c)->mData.int_));
      DISPATCH()
    }
    CASE(FGT)
    {
      RASSIGN(a, mData.boolean, bool(R(b)->mData.float_ > R(c)->mData.float_));
      DISPATCH()
    }
    CASE(FGTK)
    {
      RASSIGN(a, mData.boolean, bool(R(b)->mData.float_ > K(c)->mData.float_));
      DISPATCH()
    }
    CASE(ILTEQ)
    {
      RASSIGN(a, mData.boolean, bool(R(b)->mData.int_ <= R(c)->mData.int_));
      DISPATCH()
    }
    CASE(ILTEQK)
    {
      RASSIGN(a, mData.boolean, bool(R(b)->mData.int_ <= K(c)->mData.int_));
      DISPATCH()
    }
    CASE(FLTEQ)
    {
      RASSIGN(a, mData.boolean, bool(R(b)->mData.float_ <= R(c)->mData.float_));
      DISPATCH()
    }
    CASE(FLTEQK)
    {
      RASSIGN(a, mData.boolean, bool(R(b)->mData.float_ <= K(c)->mData.float_));
      DISPATCH()
    }
    CASE(IGTEQ)
    {
      RASSIGN(a, mData.boolean, bool(R(b)->mData.int_ >= R(c)->mData.int_));
      DISPATCH()
    }
    CASE(IGTEQK)
    {
      RASSIGN(a, mData.boolean, bool(R(b)->mData.int_ >= K(c)->mData.int_));
      DISPATCH()
    }
    CASE(FGTEQ)
    {
      RASSIGN(a, mData.boolean, bool(R(b)->mData.float_ >= R(c)->mData.float_));
      DISPATCH()
    }
    CASE(FGTEQK)
    {
      RASSIGN(a, mData.boolean, bool(R(b)->mData.float_ >= K(c)->mData.float_));
      DISPATCH()
    }
    CASE(NOT)
    {
      RASSIGN(a, mData.boolean, bool(!R(b)->mData.boolean));
      DISPATCH()
    }
    CASE(JMP)
    {
      pc += a;
      DISPATCH()
    }
    CASE(JMPIF)
    {
      [[likely]] if (R(b)->asCBool())
        pc += a;
      DISPATCH()
    }
    CASE(JMPIFX)
    {
      [[unlikely]] if (!R(b)->asCBool())
        pc += a;
      DISPATCH()
    }
    CASE(JMPBACK)
    {
      pc -= a;
      DISPATCH()
    }
    CASE(JMPBACKIF)
    {
      [[likely]] if (R(b)->asCBool())
        pc -= a;
      DISPATCH()
    }
    CASE(JMPBACKIFX)
    {
      [[unlikely]] if (!R(b)->asCBool())
        pc -= a;
      DISPATCH()
    }
    CASE(SAVESP)
    {
      interp->sp = stack.top();
      DISPATCH()
    }
    CASE(RESTSP)
    {
      stack.jump(interp->sp);
      DISPATCH()
    }
    CASE(PUSH)
    {
      interp->pushLocal(ValueRef(interp, R(a)));
      DISPATCH()
    }
    CASE(PUSHK)
    {
      interp->pushLocal(K(a));
      DISPATCH()
    }
    CASE(GETARG)
    CASE(GETARGREF)
    CASE(SETARG)
    {
      debug::todo("implement opcodes");
    }
    CASE(GETLOCAL)
    {
      RFREE(a);
      RSET(a, L(b)->clone());
      DISPATCH()
    }
    CASE(GETLOCALREF)
    {
      RFREE(a);
      RSET(a, L(b));
      DISPATCH()
    }
    CASE(SETLOCAL)
    {
      LFREE(b);
      LSET(b, R(a));
      DISPATCH()
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
    CASE(CALL)
    CASE(PCALL)
    CASE(RET)
    CASE(RETNIL)
    CASE(RETTRUE)
    CASE(RETFALSE)
    CASE(RETK)
    {
      debug::todo("implement opcodes");
    }
#ifdef HAS_CGOTO
  }
#else
    default:
      debug::bug("unknown opcode");
      break;
  }
#endif
exit:
}

}  // namespace via

void via::Interpreter::execute()
{
  __execute<false, false>(this);
}

void via::Interpreter::executeOnce()
{
  __execute<true, false>(this);
}
