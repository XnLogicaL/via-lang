/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "debug.hpp"
#include "machine.hpp"
#include "module/manager.hpp"
#include "ref.hpp"
#include "support/bit.hpp"
#include "value.hpp"

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
        auto* val = Value::create(vm, cv);                                               \
        val;                                                                             \
    })

#define CONST_VALUE_REF(ID)                                                              \
    ({                                                                                   \
        auto cv = consts.at(ID);                                                         \
        auto* val = Value::create(vm, cv);                                               \
        ValueRef(vm, val);                                                               \
    })

#define GET_LOCAL(ID) reinterpret_cast<Value*>(stack.at(ID))
#define SET_LOCAL(ID, VAL) stack.at(ID) = reinterpret_cast<uintptr_t>(VAL);
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

// Common Subexpression Elimination utility
#define CSE_OPERANDS_A() const uint16_t a = pc->a;
#define CSE_OPERANDS_AB() const uint16_t a = pc->a, b = pc->b;
#define CSE_OPERANDS_ABC() const uint16_t a = pc->a, b = pc->b, c = pc->b;

#define DEBUG_TRAP(FORMAT, ...) debug::bug(std::format(FORMAT, __VA_ARGS__));

template <bool SingleStep, bool OverridePC>
[[gnu::flatten]] void via::detail::execute_impl(VirtualMachine* vm)
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
    auto& manager = vm->m_module->manager();
    auto& symtab = manager.symbol_table();

    const auto*& pc = vm->m_pc;

[[maybe_unused]] dispatch:

    [[unlikely]] if (vm->has_interrupt()) {
        auto action = vm->handle_interrupt();
        vm->set_interrupt(Interrupt::NONE, nullptr);

        switch (action) {
        case IntAction::EXIT:
            goto exit;
        case IntAction::REINTERP:
            goto dispatch;
        case IntAction::RESUME:
            DISPATCH();
        default:
            break;
        }
    }

#ifdef HAS_CGOTO
    goto* dispatch_table[static_cast<uint16_t>(pc->op)];
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
        CASE(EXTRAARG)
        {
            goto trap__reserved_opcode;
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
        CASE(LOADNIL)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(a, Value::create(vm));
            DISPATCH();
        }
        CASE(LOADTRUE)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(a, Value::create(vm, true));
            DISPATCH();
        }
        CASE(LOADFALSE)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(a, Value::create(vm, false));
            DISPATCH();
        }
        CASE(LOADINT)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    static_cast<int64_t>(pack_halves<uint32_t>(pc->b, pc->c))
                )
            );
            DISPATCH();
        }
        CASE(NEWSTR)
        CASE(NEWARR)
        CASE(NEWDICT)
        CASE(NEWTUPLE)
        CASE(NEWCLOSURE)
        {
            goto trap__unimplemented_opcode;
        }
        CASE(IADD)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    int64_t(
                        GET_REGISTER(pc->b)->m_data.integer +
                        GET_REGISTER(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(IADDK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    int64_t(
                        GET_REGISTER(pc->b)->m_data.integer +
                        CONST_VALUE(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(FADD)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    double_t(
                        GET_REGISTER(pc->b)->m_data.float_ +
                        GET_REGISTER(pc->c)->m_data.float_
                    )
                )
            );
            DISPATCH();
        }
        CASE(FADDK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    double_t(
                        GET_REGISTER(pc->b)->m_data.float_ +
                        CONST_VALUE(pc->c)->m_data.float_
                    )
                )
            );
            DISPATCH();
        }
        CASE(ISUB)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    int64_t(
                        GET_REGISTER(pc->b)->m_data.integer -
                        GET_REGISTER(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(ISUBK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    int64_t(
                        GET_REGISTER(pc->b)->m_data.integer -
                        CONST_VALUE(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(FSUB)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    double_t(
                        GET_REGISTER(pc->b)->m_data.float_ -
                        GET_REGISTER(pc->c)->m_data.float_
                    )
                )
            );
            DISPATCH();
        }
        CASE(FSUBK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    double_t(
                        GET_REGISTER(pc->b)->m_data.float_ -
                        CONST_VALUE(pc->c)->m_data.float_
                    )
                )
            );
            DISPATCH();
        }
        CASE(IMUL)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    int64_t(
                        GET_REGISTER(pc->b)->m_data.integer *
                        GET_REGISTER(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(IMULK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    int64_t(
                        GET_REGISTER(pc->b)->m_data.integer *
                        CONST_VALUE(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(FMUL)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    double_t(
                        GET_REGISTER(pc->b)->m_data.float_ *
                        GET_REGISTER(pc->c)->m_data.float_
                    )
                )
            );
            DISPATCH();
        }
        CASE(FMULK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    double_t(
                        GET_REGISTER(pc->b)->m_data.float_ *
                        CONST_VALUE(pc->c)->m_data.float_
                    )
                )
            );
            DISPATCH();
        }
        CASE(IDIV)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    int64_t(
                        GET_REGISTER(pc->b)->m_data.integer /
                        GET_REGISTER(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(IDIVK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    int64_t(
                        GET_REGISTER(pc->b)->m_data.integer /
                        CONST_VALUE(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(FDIV)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    double_t(
                        GET_REGISTER(pc->b)->m_data.float_ /
                        GET_REGISTER(pc->c)->m_data.float_
                    )
                )
            );
            DISPATCH();
        }
        CASE(FDIVK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    double_t(
                        GET_REGISTER(pc->b)->m_data.float_ /
                        CONST_VALUE(pc->c)->m_data.float_
                    )
                )
            );
            DISPATCH();
        }
        CASE(INEG)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(vm, int64_t(-GET_REGISTER(pc->b)->m_data.integer))
            );
            DISPATCH();
        }
        CASE(INEGK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(vm, int64_t(-CONST_VALUE(pc->b)->m_data.integer))
            );
            DISPATCH();
        }
        CASE(FNEG)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(vm, double_t(-GET_REGISTER(pc->b)->m_data.float_))
            );
            DISPATCH();
        }
        CASE(FNEGK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(vm, double_t(-CONST_VALUE(pc->b)->m_data.float_))
            );
            DISPATCH();
        }
        CASE(BAND)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    int64_t(
                        GET_REGISTER(pc->b)->m_data.integer &
                        GET_REGISTER(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(BANDK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    int64_t(
                        GET_REGISTER(pc->b)->m_data.integer &
                        CONST_VALUE(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(BOR)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    int64_t(
                        GET_REGISTER(pc->b)->m_data.integer |
                        GET_REGISTER(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(BORK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    int64_t(
                        GET_REGISTER(pc->b)->m_data.integer |
                        CONST_VALUE(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(BXOR)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    int64_t(
                        GET_REGISTER(pc->b)->m_data.integer ^
                        GET_REGISTER(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(BXORK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    int64_t(
                        GET_REGISTER(pc->b)->m_data.integer ^
                        CONST_VALUE(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(BSHL)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    int64_t(
                        GET_REGISTER(pc->b)->m_data.integer
                        << GET_REGISTER(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(BSHLK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    int64_t(
                        GET_REGISTER(pc->b)->m_data.integer
                        << CONST_VALUE(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(BSHR)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    int64_t(
                        GET_REGISTER(pc->b)->m_data.integer >>
                        GET_REGISTER(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(BSHRK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    int64_t(
                        GET_REGISTER(pc->b)->m_data.integer >>
                        CONST_VALUE(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(BNOT)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(vm, int64_t(~GET_REGISTER(pc->b)->m_data.integer))
            );
            DISPATCH();
        }
        CASE(BNOTK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(vm, int64_t(CONST_VALUE(pc->b)->m_data.integer))
            );
            DISPATCH();
        }
        CASE(AND)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.boolean &&
                        GET_REGISTER(pc->c)->m_data.boolean
                    )
                )
            );
            DISPATCH();
        }
        CASE(ANDK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.boolean &&
                        CONST_VALUE(pc->c)->m_data.boolean
                    )
                )
            );
            DISPATCH();
        }
        CASE(OR)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.boolean ||
                        GET_REGISTER(pc->c)->m_data.boolean
                    )
                )
            );
            DISPATCH();
        }
        CASE(ORK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.boolean ||
                        CONST_VALUE(pc->c)->m_data.boolean
                    )
                )
            );
            DISPATCH();
        }
        CASE(IEQ)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.integer ==
                        GET_REGISTER(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(IEQK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.integer ==
                        CONST_VALUE(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(FEQ)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.float_ ==
                        GET_REGISTER(pc->c)->m_data.float_
                    )
                )
            );
            DISPATCH();
        }
        CASE(FEQK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.float_ ==
                        CONST_VALUE(pc->c)->m_data.float_
                    )
                )
            );
            DISPATCH();
        }
        CASE(BEQ)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.boolean ==
                        GET_REGISTER(pc->c)->m_data.boolean
                    )
                )
            );
            DISPATCH();
        }
        CASE(BEQK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.boolean ==
                        CONST_VALUE(pc->c)->m_data.boolean
                    )
                )
            );
            DISPATCH();
        }
        CASE(SEQ)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a)
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        std::strcmp(
                            GET_REGISTER(pc->b)->m_data.string,
                            GET_REGISTER(pc->c)->m_data.string
                        ) == 0
                    )
                )
            );
            DISPATCH();
        }
        CASE(SEQK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a)
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        std::strcmp(
                            GET_REGISTER(pc->b)->m_data.string,
                            CONST_VALUE(pc->c)->m_data.string
                        ) == 0
                    )
                )
            );
            DISPATCH();
        }
        CASE(INEQ)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.integer !=
                        GET_REGISTER(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(INEQK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.integer !=
                        CONST_VALUE(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(FNEQ)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.float_ !=
                        GET_REGISTER(pc->c)->m_data.float_
                    )
                )
            );
            DISPATCH();
        }
        CASE(FNEQK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.float_ !=
                        CONST_VALUE(pc->c)->m_data.float_
                    )
                )
            );
            DISPATCH();
        }
        CASE(BNEQ)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.boolean !=
                        GET_REGISTER(pc->c)->m_data.boolean
                    )
                )
            );
            DISPATCH();
        }
        CASE(BNEQK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.boolean !=
                        CONST_VALUE(pc->c)->m_data.boolean
                    )
                )
            );
            DISPATCH();
        }
        CASE(SNEQ)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a)
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        std::strcmp(
                            GET_REGISTER(pc->b)->m_data.string,
                            GET_REGISTER(pc->c)->m_data.string
                        ) != 0
                    )
                )
            );
            DISPATCH();
        }
        CASE(SNEQK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a)
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        std::strcmp(
                            GET_REGISTER(pc->b)->m_data.string,
                            CONST_VALUE(pc->c)->m_data.string
                        ) != 0
                    )
                )
            );
            DISPATCH();
        }
        CASE(IS)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(vm, bool(GET_REGISTER(pc->b) == GET_REGISTER(pc->c)))
            );
            DISPATCH();
        }
        CASE(ILT)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.integer <
                        GET_REGISTER(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(ILTK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.integer <
                        CONST_VALUE(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(FLT)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.float_ <
                        GET_REGISTER(pc->c)->m_data.float_
                    )
                )
            );
            DISPATCH();
        }
        CASE(FLTK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.float_ <
                        CONST_VALUE(pc->c)->m_data.float_
                    )
                )
            );
            DISPATCH();
        }
        CASE(IGT)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.integer >
                        GET_REGISTER(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(IGTK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.integer >
                        CONST_VALUE(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(FGT)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.float_ >
                        GET_REGISTER(pc->c)->m_data.float_
                    )
                )
            );
            DISPATCH();
        }
        CASE(FGTK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.float_ >
                        CONST_VALUE(pc->c)->m_data.float_
                    )
                )
            );
            DISPATCH();
        }
        CASE(ILTEQ)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.integer <=
                        GET_REGISTER(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(ILTEQK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.integer <=
                        CONST_VALUE(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(FLTEQ)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.float_ <=
                        GET_REGISTER(pc->c)->m_data.float_
                    )
                )
            );
            DISPATCH();
        }
        CASE(FLTEQK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.float_ <=
                        CONST_VALUE(pc->c)->m_data.float_
                    )
                )
            );
            DISPATCH();
        }
        CASE(IGTEQ)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.integer >=
                        GET_REGISTER(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(IGTEQK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.integer >=
                        CONST_VALUE(pc->c)->m_data.integer
                    )
                )
            );
            DISPATCH();
        }
        CASE(FGTEQ)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.float_ >=
                        GET_REGISTER(pc->c)->m_data.float_
                    )
                )
            );
            DISPATCH();
        }
        CASE(FGTEQK)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(
                    vm,
                    bool(
                        GET_REGISTER(pc->b)->m_data.float_ >=
                        CONST_VALUE(pc->c)->m_data.float_
                    )
                )
            );
            DISPATCH();
        }
        CASE(NOT)
        {
            CSE_OPERANDS_A();
            FREE_REGISTER(a);
            SET_REGISTER(
                a,
                Value::create(vm, bool(!GET_REGISTER(pc->b)->m_data.boolean))
            );
            DISPATCH();
        }
        CASE(JMP)
        {
            pc += pack_halves<uint32_t>(pc->a, pc->b);
            goto dispatch;
        }
        CASE(JMPIF)
        {
            if (GET_REGISTER(pc->a)->as_cbool()) {
                pc += pack_halves<uint32_t>(pc->b, pc->c);
                goto dispatch;
            }
            DISPATCH();
        }
        CASE(JMPIFX)
        {
            if (!GET_REGISTER(pc->a)->as_cbool()) {
                pc += pack_halves<uint32_t>(pc->b, pc->c);
                goto dispatch;
            }
            DISPATCH();
        }
        CASE(JMPBACK)
        {
            pc -= pack_halves<uint32_t>(pc->a, pc->b);
            goto dispatch;
        }
        CASE(JMPBACKIF)
        {
            if (GET_REGISTER(pc->a)->as_cbool()) {
                pc -= pack_halves<uint32_t>(pc->b, pc->c);
                goto dispatch;
            }
            DISPATCH();
        }
        CASE(JMPBACKIFX)
        {
            if (!GET_REGISTER(pc->a)->as_cbool()) {
                pc -= pack_halves<uint32_t>(pc->b, pc->c);
                goto dispatch;
            }
            DISPATCH();
        }
        CASE(SAVE)
        {
            vm->save_stack();
            DISPATCH();
        }
        CASE(RESTORE)
        {
            vm->restore_stack();
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
        CASE(GETTOP)
        {
            auto* val = reinterpret_cast<Value*>(stack.top());
            val->m_rc++;
            SET_REGISTER(pc->a, val);
            DISPATCH();
        }
        CASE(GETARG)
        CASE(GETARGREF)
        CASE(SETARG)
        {
            goto trap__unimplemented_opcode;
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
            vm->call(ValueRef(vm, GET_REGISTER(pc->a)));
            DISPATCH();
        }
        CASE(PCALL)
        {
            vm->call(ValueRef(vm, GET_REGISTER(pc->a)), CallFlags::PROTECT);
            DISPATCH();
        }
        CASE(RET)
        {
            vm->return_(ValueRef(vm, GET_REGISTER(pc->a)));
            DISPATCH();
        }
        CASE(RETNIL)
        {
            vm->return_(ValueRef(vm, Value::create(vm)));
            DISPATCH();
        }
        CASE(RETTRUE)
        {
            vm->return_(ValueRef(vm, Value::create(vm, true)));
            DISPATCH();
        }
        CASE(RETFALSE)
        {
            vm->return_(ValueRef(vm, Value::create(vm, false)));
            DISPATCH();
        }
        CASE(RETK)
        {
            vm->return_(CONST_VALUE_REF(pc->a));
            DISPATCH();
        }
        CASE(TOINT)
        {
            SET_REGISTER(pc->a, GET_REGISTER(pc->b)->as_int());
            DISPATCH();
        }
        CASE(TOFLOAT)
        {
            SET_REGISTER(pc->a, GET_REGISTER(pc->b)->as_float());
            DISPATCH();
        }
        CASE(TOBOOL)
        {
            SET_REGISTER(pc->a, GET_REGISTER(pc->b)->as_bool());
            DISPATCH();
        }
        CASE(TOSTRING)
        {
            SET_REGISTER(pc->a, GET_REGISTER(pc->b)->as_string());
            DISPATCH();
        }
        CASE(GETIMPORT)
        {
            CSE_OPERANDS_A();
            auto import = vm->get_import(pc->b, pc->c);
            import->m_rc++;
            FREE_REGISTER(a);
            SET_REGISTER(a, import.get());
            DISPATCH();
        }
#ifdef HAS_CGOTO
    }
#else
    default:
        goto trap__unknown_opcode;
    }
#endif

    // clang-format off
[[maybe_unused]] trap__unknown_opcode:
    DEBUG_TRAP("trap: unknown opcode 0x{:x} ({})", (uint16_t) pc->op, to_string(pc->op));
[[maybe_unused]] trap__reserved_opcode:
    DEBUG_TRAP("trap: reserved opcode 0x{:x} ({})", (uint16_t) pc->op, to_string(pc->op));
[[maybe_unused]] trap__unimplemented_opcode:
    DEBUG_TRAP("trap: unimplemented opcode 0x{:x} ({})", (uint16_t) pc->op, to_string(pc->op));
    DISPATCH();
    // clang-format on

exit:;
}

void via::VirtualMachine::execute()
{
    detail::execute_impl<false, false>(this);
}

void via::VirtualMachine::execute_once()
{
    detail::execute_impl<true, false>(this);
}
