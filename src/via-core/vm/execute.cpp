/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "machine.h"
#include "module/manager.h"
#include "value.h"
#include "value_ref.h"

#if defined(VIA_COMPILER_GCC) || defined(VIA_COMPILER_CLANG)
    #define HAS_CGOTO
#endif

#ifdef HAS_CGOTO
    #define CASE(OP) OP_##OP:
#else
    #define CASE(OP) case OpCode::OP:
#endif

#define DISPATCH()                                                                       \
    {                                                                                    \
        if constexpr (!OverridePC)                                                       \
            pc++;                                                                        \
        if constexpr (SingleStep)                                                        \
            goto exit;                                                                   \
        else                                                                             \
            goto dispatch;                                                               \
    }

#define CONST_VALUE(ID)                                                                  \
    ({                                                                                   \
        auto cv = consts.at(ID);                                                         \
        auto* val = Value::construct(vm, cv);                                            \
        val;                                                                             \
    })

#define CONST_VALUE_REF(ID)                                                              \
    ({                                                                                   \
        auto cv = consts.at(ID);                                                         \
        auto* val = Value::construct(vm, cv);                                            \
        ValueRef(vm, val);                                                               \
    })

#define GET_LOCAL(ID) reinterpret_cast<Value*>(stack.at(ID))
#define SET_LOCAL(ID, VAL) stack.at(ID) = reinterpret_cast<uptr>(VAL);
#define FREE_LOCAL(ID)                                                                   \
    if (GET_REGISTER(ID) != nullptr) {                                                   \
        reinterpret_cast<Value*>(stack.at(ID))->unref();                                 \
    }

#define GET_REGISTER(ID) regs[ID]
#define SET_REGISTER(ID, VAL) regs[ID] = VAL
#define FREE_REGISTER(ID)                                                                \
    [[likely]] if (GET_REGISTER(ID) != nullptr) {                                        \
        GET_REGISTER(ID)->unref();                                                       \
        SET_REGISTER(ID, nullptr);                                                       \
    }

// TODO: This macro does not account for value tags, it might mess up debug info
#define ASSIGN_REGISTER(ID, FIELD, EXPR)                                                 \
    {                                                                                    \
        FREE_REGISTER(ID);                                                               \
        SET_REGISTER(ID, Value::construct(vm, EXPR));                                    \
    }

#define CSE_OPERANDS_A() const u16 a = pc->a;
#define CSE_OPERANDS_AB() const u16 a = pc->a, b = pc->b;
#define CSE_OPERANDS_ABC() const u16 a = pc->a, b = pc->b, c = pc->b;

template <bool SingleStep, bool OverridePC>
void via::detail::__execute(VirtualMachine* vm)
{
#ifdef HAS_CGOTO
    [[gnu::aligned(64)]] static void* dispatch_table[] = {
    #define DEFINE_DISPATCH_OP(OP) &&OP_##OP,
        FOR_EACH_OPCODE(DEFINE_DISPATCH_OP)
    #undef DEFINE_DISPATCH_OP
    };
#endif

    /* Explicit VM stuff CSE */
    auto& stack = vm->m_stack;
    auto& regs = vm->m_registers;
    auto& consts = vm->m_exe->constants();

    /* Explicit module stuff CSE */
    auto* manager = vm->m_module->get_manager();
    auto& symtab = manager->get_symbol_table();

    const auto*& pc = vm->m_pc;

dispatch:
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
            CSE_OPERANDS_AB();
            FREE_REGISTER(a);
            SET_REGISTER(a, GET_REGISTER(b));
            SET_REGISTER(b, nullptr);
            DISPATCH();
        }
        CASE(FREE1)
        {
            FREE_REGISTER(pc->a);
            DISPATCH();
        }
        CASE(FREE2)
        {
            FREE_REGISTER(pc->a);
            FREE_REGISTER(pc->b);
            DISPATCH();
        }
        CASE(FREE3)
        {
            FREE_REGISTER(pc->a);
            FREE_REGISTER(pc->b);
            FREE_REGISTER(pc->c);
            DISPATCH();
        }
        CASE(XCHG)
        {
            CSE_OPERANDS_AB();
            Value* ra = GET_REGISTER(a);
            SET_REGISTER(a, GET_REGISTER(b));
            SET_REGISTER(b, ra);
            DISPATCH();
        }
        CASE(COPY)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(a, GET_REGISTER(pc->b)->clone());
            DISPATCH();
        }
        CASE(COPYREF)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(a, GET_REGISTER(pc->b));
            DISPATCH();
        }
        CASE(LOADK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(a, CONST_VALUE(pc->b));
            DISPATCH();
        }
        CASE(LOADTRUE)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(a, Value::construct(vm, true));
            DISPATCH();
        }
        CASE(LOADFALSE)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(a, Value::construct(vm, false));
            DISPATCH();
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
            ASSIGN_REGISTER(
                pc->a,
                m_data.integer,
                i64(GET_REGISTER(pc->b)->m_data.integer +
                    GET_REGISTER(pc->c)->m_data.integer)
            );
            DISPATCH();
        }
        CASE(IADDK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.integer,
                i64(GET_REGISTER(pc->b)->m_data.integer +
                    CONST_VALUE(pc->c)->m_data.integer)
            );
            DISPATCH();
        }
        CASE(FADD)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.float_,
                f64(GET_REGISTER(pc->b)->m_data.float_ +
                    GET_REGISTER(pc->c)->m_data.float_)
            );
            DISPATCH();
        }
        CASE(FADDK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.float_,
                f64(GET_REGISTER(pc->b)->m_data.float_ + CONST_VALUE(pc->c)->m_data.float_
                )
            );
            DISPATCH();
        }
        CASE(ISUB)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.integer,
                i64(GET_REGISTER(pc->b)->m_data.integer -
                    GET_REGISTER(pc->c)->m_data.integer)
            )
            DISPATCH();
        }
        CASE(ISUBK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.integer,
                i64(GET_REGISTER(pc->b)->m_data.integer -
                    CONST_VALUE(pc->c)->m_data.integer)
            );
            DISPATCH();
        }
        CASE(FSUB)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.float_,
                f64(GET_REGISTER(pc->b)->m_data.float_ -
                    GET_REGISTER(pc->c)->m_data.float_)
            );
            DISPATCH();
        }
        CASE(FSUBK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.float_,
                f64(GET_REGISTER(pc->b)->m_data.float_ - CONST_VALUE(pc->c)->m_data.float_
                )
            );
            DISPATCH();
        }
        CASE(IMUL)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.integer,
                i64(GET_REGISTER(pc->b)->m_data.integer *
                    GET_REGISTER(pc->c)->m_data.integer)
            );
            DISPATCH();
        }
        CASE(IMULK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.integer,
                i64(GET_REGISTER(pc->b)->m_data.integer *
                    CONST_VALUE(pc->c)->m_data.integer)
            );
            DISPATCH();
        }
        CASE(FMUL)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.float_,
                f64(GET_REGISTER(pc->b)->m_data.float_ *
                    GET_REGISTER(pc->c)->m_data.float_)
            );
            DISPATCH();
        }
        CASE(FMULK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.float_,
                f64(GET_REGISTER(pc->b)->m_data.float_ * CONST_VALUE(pc->c)->m_data.float_
                )
            );
            DISPATCH();
        }
        CASE(IDIV)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.integer,
                i64(GET_REGISTER(pc->b)->m_data.integer /
                    GET_REGISTER(pc->c)->m_data.integer)
            );
            DISPATCH();
        }
        CASE(IDIVK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.integer,
                i64(GET_REGISTER(pc->b)->m_data.integer /
                    CONST_VALUE(pc->c)->m_data.integer)
            );
            DISPATCH();
        }
        CASE(FDIV)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.float_,
                f64(GET_REGISTER(pc->b)->m_data.float_ /
                    GET_REGISTER(pc->c)->m_data.float_)
            );
            DISPATCH();
        }
        CASE(FDIVK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.float_,
                f64(GET_REGISTER(pc->b)->m_data.float_ / CONST_VALUE(pc->c)->m_data.float_
                )
            );
            DISPATCH();
        }
        CASE(INEG)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.integer,
                i64(-GET_REGISTER(pc->b)->m_data.integer)
            );
            DISPATCH();
        }
        CASE(INEGK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.integer,
                i64(-CONST_VALUE(pc->b)->m_data.integer)
            );
            DISPATCH();
        }
        CASE(FNEG)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.float_,
                f64(-GET_REGISTER(pc->b)->m_data.float_)
            );
            DISPATCH();
        }
        CASE(FNEGK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.float_,
                f64(-CONST_VALUE(pc->b)->m_data.float_)
            );
            DISPATCH();
        }
        CASE(BAND)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.integer,
                i64(GET_REGISTER(pc->b)->m_data.integer &
                    GET_REGISTER(pc->c)->m_data.integer)
            )
            DISPATCH();
        }
        CASE(BANDK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.integer,
                i64(GET_REGISTER(pc->b)->m_data.integer &
                    CONST_VALUE(pc->c)->m_data.integer)
            )
            DISPATCH();
        }
        CASE(BOR)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.integer,
                i64(GET_REGISTER(pc->b)->m_data.integer |
                    GET_REGISTER(pc->c)->m_data.integer)
            )
            DISPATCH();
        }
        CASE(BORK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.integer,
                i64(GET_REGISTER(pc->b)->m_data.integer |
                    CONST_VALUE(pc->c)->m_data.integer)
            );
            DISPATCH();
        }
        CASE(BXOR)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.integer,
                i64(GET_REGISTER(pc->b)->m_data.integer ^
                    GET_REGISTER(pc->c)->m_data.integer)
            );
            DISPATCH();
        }
        CASE(BXORK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.integer,
                i64(GET_REGISTER(pc->b)->m_data.integer ^
                    CONST_VALUE(pc->c)->m_data.integer)
            );
            DISPATCH();
        }
        CASE(BSHL)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.integer,
                i64(GET_REGISTER(pc->b)->m_data.integer
                    << GET_REGISTER(pc->c)->m_data.integer)
            );
            DISPATCH();
        }
        CASE(BSHLK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.integer,
                i64(GET_REGISTER(pc->b)->m_data.integer
                    << CONST_VALUE(pc->c)->m_data.integer)
            );
            DISPATCH();
        }
        CASE(BSHR)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.integer,
                i64(GET_REGISTER(pc->b)->m_data.integer >>
                    GET_REGISTER(pc->c)->m_data.integer)
            );
            DISPATCH();
        }
        CASE(BSHRK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.integer,
                i64(GET_REGISTER(pc->b)->m_data.integer >>
                    CONST_VALUE(pc->c)->m_data.integer)
            );
            DISPATCH();
        }
        CASE(BNOT)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.integer,
                i64(~GET_REGISTER(pc->b)->m_data.integer)
            );
            DISPATCH();
        }
        CASE(BNOTK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.integer,
                i64(CONST_VALUE(pc->b)->m_data.integer)
            );
            DISPATCH();
        }
        CASE(AND)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.boolean &&
                    GET_REGISTER(pc->c)->m_data.boolean
                )
            );
            DISPATCH();
        }
        CASE(ANDK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.boolean &&
                    CONST_VALUE(pc->c)->m_data.boolean
                )
            );
            DISPATCH();
        }
        CASE(OR)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.boolean ||
                    GET_REGISTER(pc->c)->m_data.boolean
                )
            );
            DISPATCH();
        }
        CASE(ORK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.boolean ||
                    CONST_VALUE(pc->c)->m_data.boolean
                )
            );
            DISPATCH();
        }
        CASE(IEQ)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.integer ==
                    GET_REGISTER(pc->c)->m_data.integer
                )
            );
            DISPATCH();
        }
        CASE(IEQK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.integer ==
                    CONST_VALUE(pc->c)->m_data.integer
                )
            );
            DISPATCH();
        }
        CASE(FEQ)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.float_ ==
                    GET_REGISTER(pc->c)->m_data.float_
                )
            );
            DISPATCH();
        }
        CASE(FEQK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.float_ ==
                    CONST_VALUE(pc->c)->m_data.float_
                )
            );
            DISPATCH();
        }
        CASE(BEQ)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.boolean ==
                    GET_REGISTER(pc->c)->m_data.boolean
                )
            );
            DISPATCH();
        }
        CASE(BEQK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.boolean ==
                    CONST_VALUE(pc->c)->m_data.boolean
                )
            );
            DISPATCH();
        }
        CASE(SEQ)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    std::strcmp(
                        GET_REGISTER(pc->b)->m_data.string,
                        GET_REGISTER(pc->c)->m_data.string
                    ) == 0
                )
            );
            DISPATCH();
        }
        CASE(SEQK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    std::strcmp(
                        GET_REGISTER(pc->b)->m_data.string,
                        CONST_VALUE(pc->c)->m_data.string
                    ) == 0
                )
            );
            DISPATCH();
        }
        CASE(INEQ)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.integer !=
                    GET_REGISTER(pc->c)->m_data.integer
                )
            );
            DISPATCH();
        }
        CASE(INEQK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.integer !=
                    CONST_VALUE(pc->c)->m_data.integer
                )
            );
            DISPATCH();
        }
        CASE(FNEQ)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.float_ !=
                    GET_REGISTER(pc->c)->m_data.float_
                )
            );
            DISPATCH();
        }
        CASE(FNEQK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.float_ !=
                    CONST_VALUE(pc->c)->m_data.float_
                )
            );
            DISPATCH();
        }
        CASE(BNEQ)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.boolean !=
                    GET_REGISTER(pc->c)->m_data.boolean
                )
            );
            DISPATCH();
        }
        CASE(BNEQK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.boolean !=
                    CONST_VALUE(pc->c)->m_data.boolean
                )
            );
            DISPATCH();
        }
        CASE(SNEQ)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    std::strcmp(
                        GET_REGISTER(pc->b)->m_data.string,
                        GET_REGISTER(pc->c)->m_data.string
                    ) == 1
                )
            );
            DISPATCH();
        }
        CASE(SNEQK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    std::strcmp(
                        GET_REGISTER(pc->b)->m_data.string,
                        CONST_VALUE(pc->c)->m_data.string
                    ) == 1
                )
            );
            DISPATCH();
        }
        CASE(IS)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(GET_REGISTER(pc->b) == GET_REGISTER(pc->c))
            );
            DISPATCH();
        }
        CASE(ILT)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.integer <
                    GET_REGISTER(pc->c)->m_data.integer
                )
            );
            DISPATCH();
        }
        CASE(ILTK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.integer <
                    CONST_VALUE(pc->c)->m_data.integer
                )
            );
            DISPATCH();
        }
        CASE(FLT)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.float_ <
                    GET_REGISTER(pc->c)->m_data.float_
                )
            );
            DISPATCH();
        }
        CASE(FLTK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.float_ < CONST_VALUE(pc->c)->m_data.float_
                )
            );
            DISPATCH();
        }
        CASE(IGT)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.integer >
                    GET_REGISTER(pc->c)->m_data.integer
                )
            );
            DISPATCH();
        }
        CASE(IGTK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.integer >
                    CONST_VALUE(pc->c)->m_data.integer
                )
            );
            DISPATCH();
        }
        CASE(FGT)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.float_ >
                    GET_REGISTER(pc->c)->m_data.float_
                )
            );
            DISPATCH();
        }
        CASE(FGTK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.float_ > CONST_VALUE(pc->c)->m_data.float_
                )
            );
            DISPATCH();
        }
        CASE(ILTEQ)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.integer <=
                    GET_REGISTER(pc->c)->m_data.integer
                )
            );
            DISPATCH();
        }
        CASE(ILTEQK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.integer <=
                    CONST_VALUE(pc->c)->m_data.integer
                )
            );
            DISPATCH();
        }
        CASE(FLTEQ)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.float_ <=
                    GET_REGISTER(pc->c)->m_data.float_
                )
            );
            DISPATCH();
        }
        CASE(FLTEQK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.float_ <=
                    CONST_VALUE(pc->c)->m_data.float_
                )
            );
            DISPATCH();
        }
        CASE(IGTEQ)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.integer >=
                    GET_REGISTER(pc->c)->m_data.integer
                )
            );
            DISPATCH();
        }
        CASE(IGTEQK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.integer >=
                    CONST_VALUE(pc->c)->m_data.integer
                )
            );
            DISPATCH();
        }
        CASE(FGTEQ)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.float_ >=
                    GET_REGISTER(pc->c)->m_data.float_
                )
            );
            DISPATCH();
        }
        CASE(FGTEQK)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(
                    GET_REGISTER(pc->b)->m_data.float_ >=
                    CONST_VALUE(pc->c)->m_data.float_
                )
            );
            DISPATCH();
        }
        CASE(NOT)
        {
            ASSIGN_REGISTER(
                pc->a,
                m_data.boolean,
                bool(!GET_REGISTER(pc->b)->m_data.boolean)
            );
            DISPATCH();
        }
        CASE(JMP)
        {
            pc += pc->a;
            DISPATCH();
        }
        CASE(JMPIF)
        {
            [[likely]] if (GET_REGISTER(pc->b)->as_cbool())
                pc += pc->a;
            DISPATCH();
        }
        CASE(JMPIFX)
        {
            [[unlikely]] if (!GET_REGISTER(pc->b)->as_cbool())
                pc += pc->a;
            DISPATCH();
        }
        CASE(JMPBACK)
        {
            pc -= pc->a;
            DISPATCH();
        }
        CASE(JMPBACKIF)
        {
            [[likely]] if (GET_REGISTER(pc->b)->as_cbool())
                pc -= pc->a;
            DISPATCH();
        }
        CASE(JMPBACKIFX)
        {
            [[unlikely]] if (!GET_REGISTER(pc->b)->as_cbool())
                pc -= pc->a;
            DISPATCH();
        }
        CASE(SAVESP)
        {
            vm->m_sp = &stack.top();
            DISPATCH();
        }
        CASE(RESTSP)
        {
            stack.jump(vm->m_sp);
            DISPATCH();
        }
        CASE(PUSH)
        {
            auto* val = GET_REGISTER(pc->a);
            val->m_rc++;
            vm->push_local(ValueRef(vm, val));
            DISPATCH();
        }
        CASE(PUSHK)
        {
            vm->push_local(CONST_VALUE_REF(pc->a));
            DISPATCH();
        }
        CASE(GETARG)
        CASE(GETARGREF)
        CASE(SETARG)
        {
            debug::todo("implement opcodes");
        }
        CASE(GETLOCAL)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(a, GET_LOCAL(pc->b)->clone());
            DISPATCH();
        }
        CASE(GETLOCALREF)
        {
            CSE_OPERANDS_A();
            auto* local = GET_LOCAL(pc->b);
            local->m_rc++;
            FREE_REGISTER(a);
            SET_REGISTER(a, local);
            DISPATCH();
        }
        CASE(SETLOCAL)
        {
            CSE_OPERANDS_AB()
            FREE_LOCAL(b);
            SET_LOCAL(b, GET_REGISTER(a));
            DISPATCH();
        }
        CASE(CALL)
        {
            vm->call(ValueRef(vm, GET_LOCAL(pc->a)));
            DISPATCH();
        }
        CASE(PCALL)
        {
            vm->call(ValueRef(vm, GET_LOCAL(pc->a)), CF_PROTECT);
            DISPATCH();
        }
        CASE(RET)
        {
            vm->return_(ValueRef(vm, GET_REGISTER(pc->a)));
            DISPATCH();
        }
        CASE(RETNIL)
        {
            vm->return_(ValueRef(vm, Value::construct(vm)));
            DISPATCH();
        }
        CASE(RETTRUE)
        {
            vm->return_(ValueRef(vm, Value::construct(vm, true)));
            DISPATCH();
        }
        CASE(RETFALSE)
        {
            vm->return_(ValueRef(vm, Value::construct(vm, false)));
            DISPATCH();
        }
        CASE(RETK)
        {
            vm->return_(CONST_VALUE_REF(pc->a));
            DISPATCH();
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
        {
            debug::todo("implement opcodes");
        }
        CASE(GETIMPORT)
        {
            DISPATCH();
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

void via::VirtualMachine::execute()
{
    detail::__execute<false, false>(this);
}

void via::VirtualMachine::execute_one()
{
    detail::__execute<true, false>(this);
}
