/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "machine.h"
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

#define DISPATCH()                 \
    {                              \
        if constexpr (!OverridePC) \
            pc++;                  \
        if constexpr (SingleStep)  \
            goto exit;             \
        else                       \
            goto dispatch;         \
    }

#define K vm->get_constant

#define L(ID) reinterpret_cast<Value*>(stack.at(ID))
#define LSET(ID, VAL) *stack.at(ID) = reinterpret_cast<uptr>(VAL);
#define LFREE(ID)         \
    if (R(ID) != nullptr) \
        reinterpret_cast<Value*>(stack.at(ID))->m_rc--;

#define R(ID) regs[ID]
#define RSET(ID, VAL) regs[ID] = VAL
#define RFREE(ID)                    \
    [[likely]] if (R(ID) != nullptr) \
        R(ID)->m_rc--;

// TODO: This macro does not account for value tags, it might mess up debug info
#define RASSIGN(ID, FIELD, EXPR)            \
    {                                       \
        RFREE(ID);                          \
        R(ID) = Value::construct(vm, EXPR); \
    }

namespace via {

template <bool SingleStep, bool OverridePC>
void execute_impl(VirtualMachine* vm)
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
    auto& stack = vm->m_stack;
    auto& regs = vm->m_registers;

    const auto*& pc = vm->m_pc;
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
            RSET(a, Value::construct(vm, true));
            DISPATCH()
        }
        CASE(LOADFALSE)
        {
            RFREE(a);
            RSET(a, Value::construct(vm, false));
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
            RASSIGN(a, m_data.integer, i64(R(b)->m_data.integer + R(c)->m_data.integer));
            DISPATCH();
        }
        CASE(IADDK)
        {
            RASSIGN(a, m_data.integer, i64(R(b)->m_data.integer + K(c)->m_data.integer));
            DISPATCH();
        }
        CASE(FADD)
        {
            RASSIGN(a, m_data.float_, f64(R(b)->m_data.float_ + R(c)->m_data.float_));
            DISPATCH();
        }
        CASE(FADDK)
        {
            RASSIGN(a, m_data.float_, f64(R(b)->m_data.float_ + K(c)->m_data.float_));
            DISPATCH();
        }
        CASE(ISUB)
        {
            RASSIGN(a, m_data.integer, i64(R(b)->m_data.integer - R(c)->m_data.integer))
            DISPATCH();
        }
        CASE(ISUBK)
        {
            RASSIGN(a, m_data.integer, i64(R(b)->m_data.integer - K(c)->m_data.integer));
            DISPATCH();
        }
        CASE(FSUB)
        {
            RASSIGN(a, m_data.float_, f64(R(b)->m_data.float_ - R(c)->m_data.float_));
            DISPATCH();
        }
        CASE(FSUBK)
        {
            RASSIGN(a, m_data.float_, f64(R(b)->m_data.float_ - K(c)->m_data.float_));
            DISPATCH();
        }
        CASE(IMUL)
        {
            RASSIGN(a, m_data.integer, i64(R(b)->m_data.integer * R(c)->m_data.integer));
            DISPATCH();
        }
        CASE(IMULK)
        {
            RASSIGN(a, m_data.integer, i64(R(b)->m_data.integer * K(c)->m_data.integer));
            DISPATCH();
        }
        CASE(FMUL)
        {
            RASSIGN(a, m_data.float_, f64(R(b)->m_data.float_ * R(c)->m_data.float_));
            DISPATCH();
        }
        CASE(FMULK)
        {
            RASSIGN(a, m_data.float_, f64(R(b)->m_data.float_ * K(c)->m_data.float_));
            DISPATCH();
        }
        CASE(IDIV)
        {
            RASSIGN(a, m_data.integer, i64(R(b)->m_data.integer / R(c)->m_data.integer));
            DISPATCH();
        }
        CASE(IDIVK)
        {
            RASSIGN(a, m_data.integer, i64(R(b)->m_data.integer / K(c)->m_data.integer));
            DISPATCH();
        }
        CASE(FDIV)
        {
            RASSIGN(a, m_data.float_, f64(R(b)->m_data.float_ / R(c)->m_data.float_));
            DISPATCH();
        }
        CASE(FDIVK)
        {
            RASSIGN(a, m_data.float_, f64(R(b)->m_data.float_ / K(c)->m_data.float_));
            DISPATCH();
        }
        CASE(INEG)
        {
            RASSIGN(a, m_data.integer, i64(-R(b)->m_data.integer));
            DISPATCH();
        }
        CASE(INEGK)
        {
            RASSIGN(a, m_data.integer, i64(-K(b)->m_data.integer));
            DISPATCH();
        }
        CASE(FNEG)
        {
            RASSIGN(a, m_data.float_, f64(-R(b)->m_data.float_));
            DISPATCH();
        }
        CASE(FNEGK)
        {
            RASSIGN(a, m_data.float_, f64(-K(b)->m_data.float_));
            DISPATCH();
        }
        CASE(BAND)
        {
            RASSIGN(a, m_data.integer, i64(R(b)->m_data.integer & R(c)->m_data.integer))
            DISPATCH();
        }
        CASE(BANDK)
        {
            RASSIGN(a, m_data.integer, i64(R(b)->m_data.integer & K(c)->m_data.integer))
            DISPATCH();
        }
        CASE(BOR)
        {
            RASSIGN(a, m_data.integer, i64(R(b)->m_data.integer | R(c)->m_data.integer))
            DISPATCH();
        }
        CASE(BORK)
        {
            RASSIGN(a, m_data.integer, i64(R(b)->m_data.integer | K(c)->m_data.integer))
            DISPATCH();
        }
        CASE(BXOR)
        {
            RASSIGN(a, m_data.integer, i64(R(b)->m_data.integer ^ R(c)->m_data.integer))
            DISPATCH();
        }
        CASE(BXORK){RASSIGN(a, m_data.integer, i64(R(b)->m_data.integer ^ K(c)->m_data.integer)) DISPATCH()} CASE(BSHL)
        {
            RASSIGN(a, m_data.integer, i64(R(b)->m_data.integer << R(c)->m_data.integer));
            DISPATCH()
        }
        CASE(BSHLK)
        {
            RASSIGN(a, m_data.integer, i64(R(b)->m_data.integer << K(c)->m_data.integer));
            DISPATCH()
        }
        CASE(BSHR)
        {
            RASSIGN(a, m_data.integer, i64(R(b)->m_data.integer >> R(c)->m_data.integer));
            DISPATCH()
        }
        CASE(BSHRK)
        {
            RASSIGN(a, m_data.integer, i64(R(b)->m_data.integer >> K(c)->m_data.integer));
            DISPATCH()
        }
        CASE(BNOT)
        {
            RASSIGN(a, m_data.integer, i64(~R(b)->m_data.integer));
            DISPATCH()
        }
        CASE(BNOTK)
        {
            RASSIGN(a, m_data.integer, i64(K(b)->m_data.integer));
            DISPATCH()
        }
        CASE(AND)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.boolean && R(c)->m_data.boolean));
            DISPATCH()
        }
        CASE(ANDK)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.boolean && K(c)->m_data.boolean));
            DISPATCH()
        }
        CASE(OR)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.boolean || R(c)->m_data.boolean));
            DISPATCH()
        }
        CASE(ORK)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.boolean || K(c)->m_data.boolean));
            DISPATCH()
        }
        CASE(IEQ)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.integer == R(c)->m_data.integer));
            DISPATCH()
        }
        CASE(IEQK)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.integer == K(c)->m_data.integer));
            DISPATCH()
        }
        CASE(FEQ)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.float_ == R(c)->m_data.float_));
            DISPATCH()
        }
        CASE(FEQK)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.float_ == K(c)->m_data.float_));
            DISPATCH()
        }
        CASE(BEQ)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.boolean == R(c)->m_data.boolean));
            DISPATCH()
        }
        CASE(BEQK)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.boolean == K(c)->m_data.boolean));
            DISPATCH()
        }
        CASE(SEQ)
        {
            RASSIGN(a, m_data.boolean, bool(std::strcmp(R(b)->m_data.string, R(c)->m_data.string) == 0));
            DISPATCH()
        }
        CASE(SEQK)
        {
            RASSIGN(a, m_data.boolean, bool(std::strcmp(R(b)->m_data.string, K(c)->m_data.string) == 0));
            DISPATCH()
        }
        CASE(INEQ)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.integer != R(c)->m_data.integer));
            DISPATCH()
        }
        CASE(INEQK)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.integer != K(c)->m_data.integer));
            DISPATCH()
        }
        CASE(FNEQ)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.float_ != R(c)->m_data.float_));
            DISPATCH()
        }
        CASE(FNEQK)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.float_ != K(c)->m_data.float_));
            DISPATCH()
        }
        CASE(BNEQ)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.boolean != R(c)->m_data.boolean));
            DISPATCH()
        }
        CASE(BNEQK)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.boolean != K(c)->m_data.boolean));
            DISPATCH()
        }
        CASE(SNEQ)
        {
            RASSIGN(a, m_data.boolean, bool(std::strcmp(R(b)->m_data.string, R(c)->m_data.string) == 1));
            DISPATCH()
        }
        CASE(SNEQK)
        {
            RASSIGN(a, m_data.boolean, bool(std::strcmp(R(b)->m_data.string, K(c)->m_data.string) == 1));
            DISPATCH()
        }
        CASE(IS)
        {
            RASSIGN(a, m_data.boolean, bool(R(b) == R(c)));
            DISPATCH()
        }
        CASE(ILT)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.integer < R(c)->m_data.integer));
            DISPATCH()
        }
        CASE(ILTK)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.integer < K(c)->m_data.integer));
            DISPATCH()
        }
        CASE(FLT)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.float_ < R(c)->m_data.float_));
            DISPATCH()
        }
        CASE(FLTK)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.float_ < K(c)->m_data.float_));
            DISPATCH()
        }
        CASE(IGT)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.integer > R(c)->m_data.integer));
            DISPATCH()
        }
        CASE(IGTK)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.integer > K(c)->m_data.integer));
            DISPATCH()
        }
        CASE(FGT)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.float_ > R(c)->m_data.float_));
            DISPATCH()
        }
        CASE(FGTK)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.float_ > K(c)->m_data.float_));
            DISPATCH()
        }
        CASE(ILTEQ)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.integer <= R(c)->m_data.integer));
            DISPATCH()
        }
        CASE(ILTEQK)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.integer <= K(c)->m_data.integer));
            DISPATCH()
        }
        CASE(FLTEQ)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.float_ <= R(c)->m_data.float_));
            DISPATCH()
        }
        CASE(FLTEQK)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.float_ <= K(c)->m_data.float_));
            DISPATCH()
        }
        CASE(IGTEQ)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.integer >= R(c)->m_data.integer));
            DISPATCH()
        }
        CASE(IGTEQK)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.integer >= K(c)->m_data.integer));
            DISPATCH()
        }
        CASE(FGTEQ)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.float_ >= R(c)->m_data.float_));
            DISPATCH()
        }
        CASE(FGTEQK)
        {
            RASSIGN(a, m_data.boolean, bool(R(b)->m_data.float_ >= K(c)->m_data.float_));
            DISPATCH()
        }
        CASE(NOT)
        {
            RASSIGN(a, m_data.boolean, bool(!R(b)->m_data.boolean));
            DISPATCH()
        }
        CASE(JMP)
        {
            pc += a;
            DISPATCH()
        }
        CASE(JMPIF)
        {
            [[likely]] if (R(b)->as_cbool())
                pc += a;
            DISPATCH()
        }
        CASE(JMPIFX)
        {
            [[unlikely]] if (!R(b)->as_cbool())
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
            [[likely]] if (R(b)->as_cbool())
                pc -= a;
            DISPATCH()
        }
        CASE(JMPBACKIFX)
        {
            [[unlikely]] if (!R(b)->as_cbool())
                pc -= a;
            DISPATCH()
        }
        CASE(SAVESP)
        {
            vm->m_sp = stack.top();
            DISPATCH()
        }
        CASE(RESTSP)
        {
            stack.jump(vm->m_sp);
            DISPATCH()
        }
        CASE(PUSH)
        {
            vm->push_local(ValueRef(vm, R(a)));
            DISPATCH()
        }
        CASE(PUSHK)
        {
            vm->push_local(K(a));
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

} // namespace via

void via::VirtualMachine::execute()
{
    execute_impl<false, false>(this);
}

void via::VirtualMachine::execute_one()
{
    execute_impl<true, false>(this);
}
