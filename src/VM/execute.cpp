// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#include "execute.h"
#include "fileio.h"
#include "vmapi.h"
#include "chunk.h"
#include "common.h"
#include "state.h"
#include "types.h"

// Define the hot path threshold for the instruction dispatch loop
// How many times a chunk needs to be executed before being flagged as
// "hot"
#ifndef VIA_HOTPATH_THRESHOLD
    #define VIA_HOTPATH_THRESHOLD 64
#endif

// Index of the current instruction
#define VM_POS V->ip - V->ihp
#define VM_CHECK_JMP(addr) ((addr >= V->ihp) && (addr <= V->ibp))

#define VM_ERROR(message) \
    do { \
        __set_error_state(V, message); \
        V->sig_error.fire(); \
        goto dispatch; \
    } while (0)

#define VM_FATAL(message) \
    do { \
        std::cerr << "VM terminated with message: " << message << '\n'; \
        V->sig_fatal.fire(); \
        std::abort(); \
    } while (0)

#define VM_ASSERT(cond, message) \
    if (!cond) { \
        VM_ERROR(message); \
    }

// Macro for loading the next instruction
// Has bound checks
#define VM_LOAD() \
    do { \
        if (!VM_CHECK_JMP(V->ip + 1)) { \
            goto exit; \
        } \
        V->ip++; \
    } while (0)

// Macro that "signals" the VM has completed an execution cycle
#define VM_NEXT() \
    do { \
        VM_LOAD(); \
        goto dispatch; \
    } while (0)

namespace via {

void vm_save_snapshot(State *VIA_RESTRICT V)
{
    U64 pos = V->ip - V->ihp;
    std::string file = std::format("vm_snapshot.{}.log", pos);

    std::string headers = std::format("opcode: {}\n", magic_enum::enum_name(V->ip->op));

    std::string registers = "==== registers ====\n";
    std::string stack = "==== stack ====\n";

    // Generate stack map
    for (TValue *p = V->sbp; p < V->sbp + V->sp; p++) {
        U32 pos = p - V->sbp;
        stack += std::format("|{:02}| {}\n", pos, impl::__to_cxx_string(V, *p));
    }

    stack += "==== stack ====\n";

    // Generate register map
    for (U32 reg = 0; reg < VIA_REGISTER_COUNT; reg++) {
        TValue *val = impl::__get_register(V, reg);
        registers += std::format("|R{:02}| {}\n", reg, impl::__to_cxx_string(V, *val));
    }

    registers += "==== registers ====\n";

    utils::write_to_file(
        std::format("./__viacache__/{}", file),
        headers + "\n" + to_string(V) + "\n" + stack + "\n" + registers
    );
}

// Starts VM execution cycle by altering it's state and "iterating" over
// the instruction pipeline.
void execute(State *VIA_RESTRICT V)
{
    using enum ValueType;
    using enum OpCode;
    using namespace impl;

    VIA_ASSERT(V->tstate == ThreadState::PAUSED, "via::execute must be called on inactive thread");
    V->tstate = ThreadState::RUNNING;

    goto dispatch;

dispatch: {

    vm_save_snapshot(V);

    // Check for errors and attempt handling them.
    // The __handle_error function works by unwinding the stack until
    // either hitting a stack frame flagged as error handler, or, the root
    // stack frame, and the root stack frame cannot be an error handler
    // under any circumstances. Therefore the error will act as a fatal
    // error, being automatically thrown by __handle_error, along with a
    // callstack and debug information.
    if (__has_error(V) && !__handle_error(V)) {
        goto exit;
    }

    // Abort is second priority due to verbosity.
    if (V->abort) {
        V->sig_abort.fire();
        goto exit;
    }

    switch (V->ip->op) {
    case NOP: {
        VM_NEXT();
    }

    case ADD: {
        U32 lhs = V->ip->operand0;
        U32 rhs = V->ip->operand1;

        TValue *lhs_val = __get_register(V, lhs);
        TValue *rhs_val = __get_register(V, rhs);

        ValueType lhs_type = lhs_val->type;
        ValueType rhs_type = rhs_val->type;

        if VIA_LIKELY (lhs_type == integer) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_integer += rhs_val->val_integer;
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    static_cast<float>(lhs_val->val_integer) + rhs_val->val_floating_point;
                lhs_val->type = floating_point;
            }
        }
        else if (lhs_type == floating_point) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_floating_point += static_cast<float>(rhs_val->val_integer);
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point += rhs_val->val_floating_point;
            }
        }
        else if (lhs_type == table) {
            const TValue &metamethod = __get_metamethod(*lhs_val, ADD);
            __push(V, *lhs_val);
            __push(V, *rhs_val);
            __call(V, metamethod, 2);
        }
        else {
            VM_ERROR(std::format(
                "attempt to perform arithmetic ({}) on {} and {}",
                magic_enum::enum_name(V->ip->op),
                magic_enum::enum_name(lhs_type),
                magic_enum::enum_name(rhs_type)
            ));
        }

        VM_NEXT();
    }
    case ADDK: {
        U32 lhs = V->ip->operand0;
        U32 idx = V->ip->operand1;

        TValue *lhs_val = __get_register(V, lhs);
        const TValue &rhs_val = __get_constant(V, idx);

        ValueType lhs_type = lhs_val->type;
        ValueType rhs_type = rhs_val.type;

        if VIA_LIKELY (lhs_type == integer) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_integer += rhs_val.val_integer;
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    static_cast<float>(lhs_val->val_integer) + rhs_val.val_floating_point;
                lhs_val->type = floating_point;
            }
        }
        else if (lhs_type == floating_point) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_floating_point += static_cast<float>(rhs_val.val_integer);
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point += rhs_val.val_floating_point;
            }
        }
        else if (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, ADD);
            __push(V, *lhs_val); // Push self
            __push(V, rhs_val);  // Push other
            __call(V, metamethod, 2);
        }
        else {
            VM_ERROR(std::format(
                "attempt to perform arithmetic ({}) on {} and constant",
                magic_enum::enum_name(V->ip->op),
                magic_enum::enum_name(lhs_val->type)
            ));
        }

        VM_NEXT();
    }

    case SUB: {
        U32 lhs = V->ip->operand0;
        U32 rhs = V->ip->operand1;

        TValue *lhs_val = __get_register(V, lhs);
        TValue *rhs_val = __get_register(V, rhs);

        ValueType lhs_type = lhs_val->type;
        ValueType rhs_type = rhs_val->type;

        if VIA_LIKELY (lhs_type == integer) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_integer -= rhs_val->val_integer;
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    static_cast<float>(lhs_val->val_integer) - rhs_val->val_floating_point;
                lhs_val->type = floating_point;
            }
        }
        else if (lhs_type == floating_point) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_floating_point -= static_cast<float>(rhs_val->val_integer);
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point -= rhs_val->val_floating_point;
            }
        }
        else if (lhs_type == table) {
            const TValue &metamethod = __get_metamethod(*lhs_val, V->ip->op);
            __push(V, *lhs_val);
            __push(V, *rhs_val);
            __call(V, metamethod, 2);
        }
        else {
            VM_ERROR(std::format(
                "attempt to perform arithmetic ({}) on {} and {}",
                magic_enum::enum_name(V->ip->op),
                magic_enum::enum_name(lhs_type),
                magic_enum::enum_name(rhs_type)
            ));
        }

        VM_NEXT();
    }
    case SUBK: {
        U32 lhs = V->ip->operand0;
        U32 idx = V->ip->operand1;

        TValue *lhs_val = __get_register(V, lhs);
        const TValue &rhs_val = __get_constant(V, idx);

        ValueType lhs_type = lhs_val->type;
        ValueType rhs_type = rhs_val.type;

        if VIA_LIKELY (lhs_type == integer) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_integer -= rhs_val.val_integer;
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    static_cast<float>(lhs_val->val_integer) - rhs_val.val_floating_point;
                lhs_val->type = floating_point;
            }
        }
        else if (lhs_type == floating_point) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_floating_point -= static_cast<float>(rhs_val.val_integer);
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point -= rhs_val.val_floating_point;
            }
        }
        else if (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, V->ip->op);
            __push(V, *lhs_val); // Push self
            __push(V, rhs_val);  // Push other
            __call(V, metamethod, 2);
        }
        else {
            VM_ERROR(std::format(
                "attempt to perform arithmetic ({}) on {} and constant",
                magic_enum::enum_name(V->ip->op),
                magic_enum::enum_name(lhs_val->type)
            ));
        }

        VM_NEXT();
    }

    case MUL: {
        U32 lhs = V->ip->operand0;
        U32 rhs = V->ip->operand1;

        TValue *lhs_val = __get_register(V, lhs);
        TValue *rhs_val = __get_register(V, rhs);

        ValueType lhs_type = lhs_val->type;
        ValueType rhs_type = rhs_val->type;

        if VIA_LIKELY (lhs_type == integer) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_integer *= rhs_val->val_integer;
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    static_cast<float>(lhs_val->val_integer) * rhs_val->val_floating_point;
                lhs_val->type = floating_point;
            }
        }
        else if (lhs_type == floating_point) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_floating_point *= static_cast<float>(rhs_val->val_integer);
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point *= rhs_val->val_floating_point;
            }
        }
        else if (lhs_type == table) {
            const TValue &metamethod = __get_metamethod(*lhs_val, V->ip->op);
            __push(V, *lhs_val);
            __push(V, *rhs_val);
            __call(V, metamethod, 2);
        }
        else {
            VM_ERROR(std::format(
                "attempt to perform arithmetic ({}) on {} and {}",
                magic_enum::enum_name(V->ip->op),
                magic_enum::enum_name(lhs_type),
                magic_enum::enum_name(rhs_type)
            ));
        }

        VM_NEXT();
    }
    case MULK: {
        U32 lhs = V->ip->operand0;
        U32 idx = V->ip->operand1;

        TValue *lhs_val = __get_register(V, lhs);
        const TValue &rhs_val = __get_constant(V, idx);

        ValueType lhs_type = lhs_val->type;
        ValueType rhs_type = rhs_val.type;

        if VIA_LIKELY (lhs_type == integer) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_integer *= rhs_val.val_integer;
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    static_cast<float>(lhs_val->val_integer) * rhs_val.val_floating_point;
                lhs_val->type = floating_point;
            }
        }
        else if (lhs_type == floating_point) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_floating_point *= static_cast<float>(rhs_val.val_integer);
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point *= rhs_val.val_floating_point;
            }
        }
        else if (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, V->ip->op);
            __push(V, *lhs_val); // Push self
            __push(V, rhs_val);  // Push other
            __call(V, metamethod, 2);
        }
        else {
            VM_ERROR(std::format(
                "attempt to perform arithmetic ({}) on {} and constant",
                magic_enum::enum_name(V->ip->op),
                magic_enum::enum_name(lhs_val->type)
            ));
        }

        VM_NEXT();
    }

    case DIV: {
        U32 lhs = V->ip->operand0;
        U32 rhs = V->ip->operand1;

        TValue *lhs_val = __get_register(V, lhs);
        TValue *rhs_val = __get_register(V, rhs);

        ValueType lhs_type = lhs_val->type;
        ValueType rhs_type = rhs_val->type;

        if VIA_LIKELY (lhs_type == integer) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_integer /= rhs_val->val_integer;
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    static_cast<float>(lhs_val->val_integer) / rhs_val->val_floating_point;
                lhs_val->type = floating_point;
            }
        }
        else if (lhs_type == floating_point) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_floating_point /= static_cast<float>(rhs_val->val_integer);
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point /= rhs_val->val_floating_point;
            }
        }
        else if (lhs_type == table) {
            const TValue &metamethod = __get_metamethod(*lhs_val, V->ip->op);
            __push(V, *lhs_val);
            __push(V, *rhs_val);
            __call(V, metamethod, 2);
        }
        else {
            VM_ERROR(std::format(
                "attempt to perform arithmetic ({}) on {} and {}",
                magic_enum::enum_name(V->ip->op),
                magic_enum::enum_name(lhs_type),
                magic_enum::enum_name(rhs_type)
            ));
        }

        VM_NEXT();
    }
    case DIVK: {
        U32 lhs = V->ip->operand0;
        U32 idx = V->ip->operand1;

        TValue *lhs_val = __get_register(V, lhs);
        const TValue &rhs_val = __get_constant(V, idx);

        ValueType lhs_type = lhs_val->type;
        ValueType rhs_type = rhs_val.type;

        if VIA_LIKELY (lhs_type == integer) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_integer /= rhs_val.val_integer;
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    static_cast<float>(lhs_val->val_integer) / rhs_val.val_floating_point;
                lhs_val->type = floating_point;
            }
        }
        else if (lhs_type == floating_point) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_floating_point /= static_cast<float>(rhs_val.val_integer);
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point /= rhs_val.val_floating_point;
            }
        }
        else if (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, V->ip->op);
            __push(V, *lhs_val); // Push self
            __push(V, rhs_val);  // Push other
            __call(V, metamethod, 2);
        }
        else {
            VM_ERROR(std::format(
                "attempt to perform arithmetic ({}) on {} and constant",
                magic_enum::enum_name(V->ip->op),
                magic_enum::enum_name(lhs_val->type)
            ));
        }

        VM_NEXT();
    }

    case POW: {
        U32 lhs = V->ip->operand0;
        U32 rhs = V->ip->operand1;

        TValue *lhs_val = __get_register(V, lhs);
        TValue *rhs_val = __get_register(V, rhs);

        ValueType lhs_type = lhs_val->type;
        ValueType rhs_type = rhs_val->type;

        if VIA_LIKELY (lhs_type == integer) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_integer = std::pow(lhs_val->val_integer, rhs_val->val_integer);
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point = std::powf(
                    static_cast<float>(lhs_val->val_integer), rhs_val->val_floating_point
                );
                lhs_val->type = floating_point;
            }
        }
        else if (lhs_type == floating_point) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_floating_point = std::powf(
                    lhs_val->val_floating_point, static_cast<float>(rhs_val->val_integer)
                );
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    std::powf(lhs_val->val_floating_point, rhs_val->val_floating_point);
            }
        }
        else if (lhs_type == table) {
            const TValue &metamethod = __get_metamethod(*lhs_val, V->ip->op);
            __push(V, *lhs_val);
            __push(V, *rhs_val);
            __call(V, metamethod, 2);
        }
        else {
            VM_ERROR(std::format(
                "attempt to perform arithmetic ({}) on {} and {}",
                magic_enum::enum_name(V->ip->op),
                magic_enum::enum_name(lhs_type),
                magic_enum::enum_name(rhs_type)
            ));
        }

        VM_NEXT();
    }
    case POWK: {
        U32 lhs = V->ip->operand0;
        U32 idx = V->ip->operand1;

        TValue *lhs_val = __get_register(V, lhs);
        const TValue &rhs_val = __get_constant(V, idx);

        ValueType lhs_type = lhs_val->type;
        ValueType rhs_type = rhs_val.type;

        if VIA_LIKELY (lhs_type == integer) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_integer = std::pow(lhs_val->val_integer, rhs_val.val_integer);
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    std::powf(static_cast<float>(lhs_val->val_integer), rhs_val.val_floating_point);
                lhs_val->type = floating_point;
            }
        }
        else if (lhs_type == floating_point) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_floating_point =
                    std::powf(lhs_val->val_floating_point, static_cast<float>(rhs_val.val_integer));
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    std::powf(lhs_val->val_floating_point, rhs_val.val_floating_point);
            }
        }
        else if (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, V->ip->op);
            __push(V, *lhs_val); // Push self
            __push(V, rhs_val);  // Push other
            __call(V, metamethod, 2);
        }
        else {
            VM_ERROR(std::format(
                "attempt to perform arithmetic ({}) on {} and constant",
                magic_enum::enum_name(V->ip->op),
                magic_enum::enum_name(lhs_val->type)
            ));
        }

        VM_NEXT();
    }

    case MOD: {
        U32 lhs = V->ip->operand0;
        U32 rhs = V->ip->operand1;

        TValue *lhs_val = __get_register(V, lhs);
        TValue *rhs_val = __get_register(V, rhs);

        ValueType lhs_type = lhs_val->type;
        ValueType rhs_type = rhs_val->type;

        if VIA_LIKELY (lhs_type == integer) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_integer %= rhs_val->val_integer;
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point = std::fmod(
                    static_cast<float>(lhs_val->val_integer), rhs_val->val_floating_point
                );
                lhs_val->type = floating_point;
            }
        }
        else if (lhs_type == floating_point) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_floating_point = std::fmod(
                    lhs_val->val_floating_point, static_cast<float>(rhs_val->val_integer)
                );
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    std::fmod(lhs_val->val_floating_point, rhs_val->val_floating_point);
            }
        }
        else if (lhs_type == table) {
            const TValue &metamethod = __get_metamethod(*lhs_val, V->ip->op);
            __push(V, *lhs_val);
            __push(V, *rhs_val);
            __call(V, metamethod, 2);
        }
        else {
            VM_ERROR(std::format(
                "attempt to perform arithmetic ({}) on {} and {}",
                magic_enum::enum_name(V->ip->op),
                magic_enum::enum_name(lhs_type),
                magic_enum::enum_name(rhs_type)
            ));
        }

        VM_NEXT();
    }
    case MODK: {
        U32 lhs = V->ip->operand0;
        U32 idx = V->ip->operand1;

        TValue *lhs_val = __get_register(V, lhs);
        const TValue &rhs_val = __get_constant(V, idx);

        ValueType lhs_type = lhs_val->type;
        ValueType rhs_type = rhs_val.type;

        if VIA_LIKELY (lhs_type == integer) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_integer %= rhs_val.val_integer;
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    std::fmod(static_cast<float>(lhs_val->val_integer), rhs_val.val_floating_point);
                lhs_val->type = floating_point;
            }
        }
        else if (lhs_type == floating_point) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_floating_point =
                    std::fmod(lhs_val->val_floating_point, static_cast<float>(rhs_val.val_integer));
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    std::fmod(lhs_val->val_floating_point, rhs_val.val_floating_point);
            }
        }
        else if (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, V->ip->op);
            __push(V, *lhs_val); // Push self
            __push(V, rhs_val);  // Push other
            __call(V, metamethod, 2);
        }
        else {
            VM_ERROR(std::format(
                "attempt to perform arithmetic ({}) on {} and constant",
                magic_enum::enum_name(V->ip->op),
                magic_enum::enum_name(lhs_val->type)
            ));
        }

        VM_NEXT();
    }

    case MOVE: {
        U32 rdst = V->ip->operand0;
        U32 rsrc = V->ip->operand1;
        TValue *src_val = __get_register(V, rsrc);

        __set_register(V, rdst, *src_val);
        VM_NEXT();
    }

    case LOADK: {
        U32 dst = V->ip->operand0;
        U32 idx = V->ip->operand1;

        // Check if the kId is valid
        if (idx > V->program->constants->size()) {
            VM_FATAL("invalid constant index");
        }

        const TValue &kval = __get_constant(V, idx);

        __set_register(V, dst, kval);
        VM_NEXT();
    }

    case LOADNIL: {
        U32 dst = V->ip->operand0;

        __set_register(V, dst, _Nil);
        VM_NEXT();
    }

    case LOADTABLE: {
        U32 dst = V->ip->operand0;
        TValue ttable(new TTable());

        __set_register(V, dst, ttable);
        VM_NEXT();
    }

    case LOADFUNCTION: {
        U32 dst = V->ip->operand0;
        TFunction *func = new TFunction(V, "<anonymous>", V->ip, V->frame, {}, false, false);
        TValue val(func);

        while (V->ip < V->ibp) {
            if (V->ip->op == RETURN) {
                V->ip++;
                break;
            }
            // Copy the instruction and insert into the function object
            func->bytecode.push_back(*(V->ip++));
        }

        __set_register(V, dst, val);
        // Dispatch instead of invoking VM_NEXT
        goto dispatch;
    }

    case PUSH: {
        U32 src = V->ip->operand0;
        TValue *val = __get_register(V, src);

        __push(V, *val);
        VM_NEXT();
    }

    case PUSHK: {
        U32 const_idx = V->ip->operand0;
        const TValue &constant = __get_constant(V, const_idx);

        __push(V, constant);
        VM_NEXT();
    }

    case PUSHI: {
        U32 immx = V->ip->operand0;
        TValue val(immx);

        __push(V, val);
        VM_NEXT();
    }

    case POP: {
        U32 dst = V->ip->operand0;
        TValue val = __pop(V);

        __set_register(V, dst, val);
        VM_NEXT();
    }

    case GETSTACK: {
        U32 dst = V->ip->operand0;
        U32 off = V->ip->operand1;

        const TValue &val = V->sbp[off];

        __set_register(V, dst, val);
        VM_NEXT();
    }

    case SETSTACK: {
        U32 src = V->ip->operand0;
        U32 off = V->ip->operand1;

        TValue *val = __get_register(V, src);

        V->sbp[off] = std::move(*val);
        VM_NEXT();
    }

    case GETARGUMENT: {
        U32 dst = V->ip->operand0;
        U32 off = V->ip->operand1;

        const TValue &val = __get_argument(V, off);

        __set_register(V, dst, val);
        VM_NEXT();
    }

    case GETGLOBAL: {
        U32 dst = V->ip->operand0;
        U32 glb_idx = V->ip->operand1;

        const TValue &global = __get_global(V, glb_idx);

        __set_register(V, dst, global);
        VM_NEXT();
    }

    case EQUAL: {
        U32 dst = V->ip->operand0;
        U32 lhs = V->ip->operand1;
        U32 rhs = V->ip->operand2;

        if VIA_UNLIKELY (lhs == rhs) {
            __set_register(V, dst, TValue(true));
            VM_NEXT();
        }

        __builtin_prefetch(&V->registers[lhs], 0, 3);
        __builtin_prefetch(&V->registers[rhs], 0, 3);

        TValue *lhs_val = __get_register(V, lhs);
        TValue *rhs_val = __get_register(V, rhs);

        if VIA_UNLIKELY (lhs_val == rhs_val) {
            __set_register(V, dst, TValue(true));
            VM_NEXT();
        }

        bool result = __compare(*lhs_val, *rhs_val);
        __set_register(V, dst, result);

        VM_NEXT();
    }

    case NOTEQUAL: {
        U32 dst = V->ip->operand0;
        U32 lhs = V->ip->operand1;
        U32 rhs = V->ip->operand2;

        if VIA_LIKELY (lhs != rhs) {
            __set_register(V, dst, TValue(true));
            VM_NEXT();
        }

        __builtin_prefetch(&V->registers[lhs], 0, 3);
        __builtin_prefetch(&V->registers[rhs], 0, 3);

        TValue *lhs_val = __get_register(V, lhs);
        TValue *rhs_val = __get_register(V, rhs);

        if VIA_LIKELY (lhs_val != rhs_val) {
            __set_register(V, dst, TValue(true));
            VM_NEXT();
        }

        bool result = __compare(*lhs_val, *rhs_val);
        __set_register(V, dst, result);

        VM_NEXT();
    }

    case LESS: {
        U32 dst = V->ip->operand0;
        U32 lhs = V->ip->operand1;
        U32 rhs = V->ip->operand2;

        TValue *lhs_val = __get_register(V, lhs);
        TValue *rhs_val = __get_register(V, rhs);

        if VIA_LIKELY (check_integer(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                __set_register(V, dst, TValue(lhs_val->val_integer < rhs_val->val_integer));
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                __set_register(
                    V,
                    dst,
                    TValue(static_cast<float>(lhs_val->val_integer) < rhs_val->val_floating_point)
                );
            }
        }
        else if VIA_UNLIKELY (check_floating_point(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                __set_register(
                    V,
                    dst,
                    TValue(lhs_val->val_floating_point < static_cast<float>(rhs_val->val_integer))
                );
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                __set_register(
                    V, dst, TValue(lhs_val->val_floating_point < rhs_val->val_floating_point)
                );
            }
        }
        else if VIA_UNLIKELY (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, V->ip->op);

            __push(V, *lhs_val);
            __push(V, *rhs_val);
            __call(V, metamethod, 2);

            TValue val = __pop(V);

            __set_register(V, dst, val);
            VM_NEXT();
        }
        else {
            VM_ERROR(std::format(
                "attempt to perform comparison ({}) on {} and {}",
                magic_enum::enum_name(V->ip->op),
                magic_enum::enum_name(lhs_val->type),
                magic_enum::enum_name(rhs_val->type)
            ));
        }

        VM_NEXT();
    }

    case GREATER: {
        U32 dst = V->ip->operand0;
        U32 lhs = V->ip->operand1;
        U32 rhs = V->ip->operand2;

        TValue *lhs_val = __get_register(V, lhs);
        TValue *rhs_val = __get_register(V, rhs);

        if VIA_LIKELY (check_integer(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                __set_register(V, dst, TValue(lhs_val->val_integer > rhs_val->val_integer));
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                __set_register(
                    V,
                    dst,
                    TValue(static_cast<float>(lhs_val->val_integer) > rhs_val->val_floating_point)
                );
            }
        }
        else if VIA_UNLIKELY (check_floating_point(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                __set_register(
                    V,
                    dst,
                    TValue(lhs_val->val_floating_point > static_cast<float>(rhs_val->val_integer))
                );
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                __set_register(
                    V, dst, TValue(lhs_val->val_floating_point > rhs_val->val_floating_point)
                );
            }
        }
        else if VIA_UNLIKELY (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, V->ip->op);

            __push(V, *lhs_val);
            __push(V, *rhs_val);
            __call(V, metamethod, 2);

            TValue val = __pop(V);

            __set_register(V, dst, val);
            VM_NEXT();
        }
        else {
            VM_ERROR(std::format(
                "attempt to perform comparison ({}) on {} and {}",
                magic_enum::enum_name(V->ip->op),
                magic_enum::enum_name(lhs_val->type),
                magic_enum::enum_name(rhs_val->type)
            ));
        }

        VM_NEXT();
    }

    case LESSOREQUAL: {
        U32 dst = V->ip->operand0;
        U32 lhs = V->ip->operand1;
        U32 rhs = V->ip->operand2;

        TValue *lhs_val = __get_register(V, lhs);
        TValue *rhs_val = __get_register(V, rhs);

        if VIA_LIKELY (check_integer(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                __set_register(V, dst, TValue(lhs_val->val_integer <= rhs_val->val_integer));
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                __set_register(
                    V,
                    dst,
                    TValue(static_cast<float>(lhs_val->val_integer) <= rhs_val->val_floating_point)
                );
            }
        }
        else if VIA_UNLIKELY (check_floating_point(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                __set_register(
                    V,
                    dst,
                    TValue(lhs_val->val_floating_point <= static_cast<float>(rhs_val->val_integer))
                );
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                __set_register(
                    V, dst, TValue(lhs_val->val_floating_point <= rhs_val->val_floating_point)
                );
            }
        }
        else if VIA_UNLIKELY (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, V->ip->op);

            __push(V, *lhs_val);
            __push(V, *rhs_val);
            __call(V, metamethod, 2);

            TValue val = __pop(V);

            __set_register(V, dst, val);
            VM_NEXT();
        }
        else {
            VM_ERROR(std::format(
                "attempt to perform comparison ({}) on {} and {}",
                magic_enum::enum_name(V->ip->op),
                magic_enum::enum_name(lhs_val->type),
                magic_enum::enum_name(rhs_val->type)
            ));
        }

        VM_NEXT();
    }

    case GREATEROREQUAL: {
        U32 dst = V->ip->operand0;
        U32 lhs = V->ip->operand1;
        U32 rhs = V->ip->operand2;

        TValue *lhs_val = __get_register(V, lhs);
        TValue *rhs_val = __get_register(V, rhs);

        if VIA_LIKELY (check_integer(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                __set_register(V, dst, TValue(lhs_val->val_integer >= rhs_val->val_integer));
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                __set_register(
                    V,
                    dst,
                    TValue(static_cast<float>(lhs_val->val_integer) >= rhs_val->val_floating_point)
                );
            }
        }
        else if VIA_UNLIKELY (check_floating_point(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                __set_register(
                    V,
                    dst,
                    TValue(lhs_val->val_floating_point >= static_cast<float>(rhs_val->val_integer))
                );
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                __set_register(
                    V, dst, TValue(lhs_val->val_floating_point >= rhs_val->val_floating_point)
                );
            }
        }
        else if VIA_UNLIKELY (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, V->ip->op);

            __push(V, *lhs_val);
            __push(V, *rhs_val);
            __call(V, metamethod, 2);

            TValue val = __pop(V);

            __set_register(V, dst, val);
            VM_NEXT();
        }
        else {
            VM_ERROR(std::format(
                "attempt to perform comparison ({}) on {} and {}",
                magic_enum::enum_name(V->ip->op),
                magic_enum::enum_name(lhs_val->type),
                magic_enum::enum_name(rhs_val->type)
            ));
        }

        VM_NEXT();
    }

    case EXIT: {
        goto exit;
    }

    case JUMP: {
        U32 offset = V->ip->operand0;
        V->ip += offset;
        VM_NEXT();
    }

    case JUMPIF: {
        U32 cond = V->ip->operand0;
        U32 offset = V->ip->operand1;

        if VIA_UNLIKELY (!VM_CHECK_JMP(V->ip + offset)) {
            VM_FATAL("invalid jump address");
        }

        TValue *cond_val = __get_register(V, cond);
        if (__to_cxx_bool(*cond_val)) {
            V->ip += offset;
        }

        VM_NEXT();
    }

    case JUMPIFNOT: {
        U32 cond = V->ip->operand0;
        U32 offset = V->ip->operand1;

        if VIA_UNLIKELY (!VM_CHECK_JMP(V->ip + offset)) {
            VM_FATAL("invalid jump address");
        }

        TValue *cond_val = __get_register(V, cond);
        if (!__to_cxx_bool(*cond_val)) {
            V->ip += offset;
        }

        VM_NEXT();
    }

    case JUMPIFEQUAL: {
        U32 cond_lhs = V->ip->operand0;
        U32 cond_rhs = V->ip->operand1;
        U32 offset = V->ip->operand2;

        if VIA_UNLIKELY (!VM_CHECK_JMP(V->ip + offset)) {
            VM_FATAL("invalid jump address");
        }

        if VIA_UNLIKELY (cond_lhs == cond_rhs) {
            V->ip += offset;
        }
        else {
            TValue *lhs_val = __get_register(V, cond_lhs);
            TValue *rhs_val = __get_register(V, cond_rhs);

            if VIA_UNLIKELY (lhs_val == rhs_val || __compare(*lhs_val, *rhs_val)) {
                V->ip += offset;
            }
        }

        VM_NEXT();
    }

    case JUMPIFNOTEQUAL: {
        U32 cond_lhs = V->ip->operand0;
        U32 cond_rhs = V->ip->operand1;
        U32 offset = V->ip->operand2;

        if VIA_UNLIKELY (!VM_CHECK_JMP(V->ip + offset)) {
            VM_FATAL("invalid jump address");
        }

        if VIA_LIKELY (cond_lhs != cond_rhs) {
            V->ip += offset;
        }
        else {
            TValue *lhs_val = __get_register(V, cond_lhs);
            TValue *rhs_val = __get_register(V, cond_rhs);

            if VIA_LIKELY (lhs_val != rhs_val || !__compare(*lhs_val, *rhs_val)) {
                V->ip += offset;
            }
        }

        VM_NEXT();
    }

    case JUMPIFLESS: {
        U32 cond_lhs = V->ip->operand0;
        U32 cond_rhs = V->ip->operand1;
        U32 offset = V->ip->operand2;

        if VIA_UNLIKELY (!VM_CHECK_JMP(V->ip + offset)) {
            VM_FATAL("invalid jump address");
        }

        TValue *lhs_val = __get_register(V, cond_lhs);
        TValue *rhs_val = __get_register(V, cond_rhs);

        if VIA_LIKELY (check_integer(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                if (lhs_val->val_integer < rhs_val->val_integer) {
                    V->ip += offset;
                }
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                if (static_cast<float>(lhs_val->val_integer) < rhs_val->val_floating_point) {
                    V->ip += offset;
                }
            }
        }
        else if VIA_UNLIKELY (check_floating_point(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                if (lhs_val->val_floating_point < static_cast<float>(rhs_val->val_integer)) {
                    V->ip += offset;
                }
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                if (lhs_val->val_floating_point < rhs_val->val_floating_point) {
                    V->ip += offset;
                }
            }
        }
        else if VIA_UNLIKELY (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, V->ip->op);

            __push(V, *lhs_val);
            __push(V, *rhs_val);
            __call(V, metamethod, 2);

            TValue val = __pop(V);
            if VIA_LIKELY (check_bool(val) && val.val_boolean) {
                V->ip += offset;
            }
            else {
                VM_ERROR(
                    std::format("comparison metamethod ({}) did not return a boolean", V->ip->op)
                );
            }

            VM_NEXT();
        }
        else {
            VM_ERROR(std::format(
                "attempt to perform comparison ({}) on {} and {}",
                magic_enum::enum_name(V->ip->op),
                magic_enum::enum_name(lhs_val->type),
                magic_enum::enum_name(rhs_val->type)
            ));
        }

        VM_NEXT();
    }

    case JUMPIFGREATER: {
        U32 cond_lhs = V->ip->operand0;
        U32 cond_rhs = V->ip->operand1;
        U32 offset = V->ip->operand2;

        if VIA_UNLIKELY (!VM_CHECK_JMP(V->ip + offset)) {
            VM_FATAL("invalid jump address");
        }

        TValue *lhs_val = __get_register(V, cond_lhs);
        TValue *rhs_val = __get_register(V, cond_rhs);

        if VIA_LIKELY (check_integer(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                if (lhs_val->val_integer > rhs_val->val_integer) {
                    V->ip += offset;
                }
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                if (static_cast<float>(lhs_val->val_integer) > rhs_val->val_floating_point) {
                    V->ip += offset;
                }
            }
        }
        else if VIA_UNLIKELY (check_floating_point(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                if (lhs_val->val_floating_point > static_cast<float>(rhs_val->val_integer)) {
                    V->ip += offset;
                }
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                if (lhs_val->val_floating_point > rhs_val->val_floating_point) {
                    V->ip += offset;
                }
            }
        }
        else if VIA_UNLIKELY (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, V->ip->op);

            __push(V, *lhs_val);
            __push(V, *rhs_val);
            __call(V, metamethod, 2);

            TValue val = __pop(V);
            if VIA_LIKELY (check_bool(val) && val.val_boolean) {
                V->ip += offset;
            }
            else {
                VM_ERROR(
                    std::format("comparison metamethod ({}) did not return a boolean", V->ip->op)
                );
            }

            VM_NEXT();
        }
        else {
            VM_ERROR(std::format(
                "attempt to perform comparison ({}) on {} and {}",
                magic_enum::enum_name(V->ip->op),
                magic_enum::enum_name(lhs_val->type),
                magic_enum::enum_name(rhs_val->type)
            ));
        }

        VM_NEXT();
    }

    case JUMPIFLESSOREQUAL: {
        U32 cond_lhs = V->ip->operand0;
        U32 cond_rhs = V->ip->operand1;
        U32 offset = V->ip->operand2;

        if VIA_UNLIKELY (!VM_CHECK_JMP(V->ip + offset)) {
            VM_FATAL("invalid jump address");
        }

        TValue *lhs_val = __get_register(V, cond_lhs);
        TValue *rhs_val = __get_register(V, cond_rhs);

        if VIA_LIKELY (check_integer(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                if (lhs_val->val_integer <= rhs_val->val_integer) {
                    V->ip += offset;
                }
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                if (static_cast<float>(lhs_val->val_integer) <= rhs_val->val_floating_point) {
                    V->ip += offset;
                }
            }
        }
        else if VIA_UNLIKELY (check_floating_point(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                if (lhs_val->val_floating_point <= static_cast<float>(rhs_val->val_integer)) {
                    V->ip += offset;
                }
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                if (lhs_val->val_floating_point <= rhs_val->val_floating_point) {
                    V->ip += offset;
                }
            }
        }
        else if VIA_UNLIKELY (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, V->ip->op);

            __push(V, *lhs_val);
            __push(V, *rhs_val);
            __call(V, metamethod, 2);

            TValue val = __pop(V);
            if VIA_LIKELY (check_bool(val) && val.val_boolean) {
                V->ip += offset;
            }
            else {
                VM_ERROR(
                    std::format("comparison metamethod ({}) did not return a boolean", V->ip->op)
                );
            }

            VM_NEXT();
        }
        else {
            VM_ERROR(std::format(
                "attempt to perform comparison ({}) on {} and {}",
                magic_enum::enum_name(V->ip->op),
                magic_enum::enum_name(lhs_val->type),
                magic_enum::enum_name(rhs_val->type)
            ));
        }

        VM_NEXT();
    }

    case JUMPIFGREATEROREQUAL: {
        U32 cond_lhs = V->ip->operand0;
        U32 cond_rhs = V->ip->operand1;
        U32 offset = V->ip->operand2;

        if VIA_UNLIKELY (!VM_CHECK_JMP(V->ip + offset)) {
            VM_FATAL("invalid jump address");
        }

        TValue *lhs_val = __get_register(V, cond_lhs);
        TValue *rhs_val = __get_register(V, cond_rhs);

        if VIA_LIKELY (check_integer(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                if (lhs_val->val_integer >= rhs_val->val_integer) {
                    V->ip += offset;
                }
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                if (static_cast<float>(lhs_val->val_integer) >= rhs_val->val_floating_point) {
                    V->ip += offset;
                }
            }
        }
        else if VIA_UNLIKELY (check_floating_point(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                if (lhs_val->val_floating_point >= static_cast<float>(rhs_val->val_integer)) {
                    V->ip += offset;
                }
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                if (lhs_val->val_floating_point >= rhs_val->val_floating_point) {
                    V->ip += offset;
                }
            }
        }
        else if VIA_UNLIKELY (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, V->ip->op);

            __push(V, *lhs_val);
            __push(V, *rhs_val);
            __call(V, metamethod, 2);

            TValue val = __pop(V);
            if VIA_LIKELY (check_bool(val) && val.val_boolean) {
                V->ip += offset;
            }
            else {
                VM_ERROR(
                    std::format("comparison metamethod ({}) did not return a boolean", V->ip->op)
                );
            }

            VM_NEXT();
        }
        else {
            VM_ERROR(std::format(
                "attempt to perform comparison ({}) on {} and {}",
                magic_enum::enum_name(V->ip->op),
                magic_enum::enum_name(lhs_val->type),
                magic_enum::enum_name(rhs_val->type)
            ));
        }

        VM_NEXT();
    }

    case CALL: {
        U32 fn = V->ip->operand0;
        U32 argc = V->ip->operand1;
        TValue *fn_val = __get_register(V, fn);

        __call(V, *fn_val, argc);
        VM_NEXT();
    }
    case EXTERNCALL: {
        U32 fn = V->ip->operand0;
        U32 argc = V->ip->operand1;
        TValue *cfunc = __get_register(V, fn);

        __extern_call(V, cfunc->cast_ptr<TCFunction>(), argc);
        VM_NEXT();
    }
    case NATIVECALL: {
        U32 fn = V->ip->operand0;
        U32 argc = V->ip->operand1;
        TValue *func = __get_register(V, fn);

        __native_call(V, func->cast_ptr<TFunction>(), argc);
        VM_NEXT();
    }
    case METHODCALL: {
        U32 obj = V->ip->operand0;
        U32 fn = V->ip->operand1;
        U32 argc = V->ip->operand2;

        TValue *func = __get_register(V, fn);
        TValue *object = __get_register(V, obj);

        __push(V, *object);
        __native_call(V, func->cast_ptr<TFunction>(), argc + 1);
        VM_NEXT();
    }

    case RETURN: {
        U32 retc = V->ip->operand0;
        __native_return(V, retc);
        VM_NEXT();
    }

    case GETTABLE: {
        U32 dst = V->ip->operand0;
        U32 tbl = V->ip->operand1;
        U32 idx = V->ip->operand2;

        TValue *tbl_val = __get_register(V, tbl);
        TValue *idx_val = __get_register(V, idx);

        U32 key = check_string(idx) ? idx_val->cast_ptr<TString>()->hash : idx;
        const TValue &index = __get_table(tbl_val->cast_ptr<TTable>(), key, true);

        __set_register(V, dst, index);
        VM_NEXT();
    }

    case SETTABLE: {
        U32 src = V->ip->operand0;
        U32 tbl = V->ip->operand1;
        U32 key = V->ip->operand2;

        TValue *table = __get_register(V, tbl);
        TValue *value = __get_register(V, src);

        __set_table(table->cast_ptr<TTable>(), key, *value);
        VM_NEXT();
    }

    case NEXTTABLE: {
        static std::unordered_map<void *, U32> next_table;

        U32 dst = V->ip->operand0;
        U32 valr = V->ip->operand1;

        TValue *val = __get_register(V, valr);
        void *ptr = __to_pointer(*val);
        U32 key = 0;

        auto it = next_table.find(ptr);
        if (it != next_table.end()) {
            key = ++it->second;
        }
        else {
            next_table[ptr] = 0;
        }

        const auto &table_map = val->cast_ptr<TTable>()->data;
        auto field_it = table_map.find(key);

        if (field_it != table_map.end()) {
            __set_register(V, dst, field_it->second);
        }
        else {
            __set_register(V, dst, _Nil);
        }

        VM_NEXT();
    }

    case LENTABLE: {
        U32 dst = V->ip->operand0;
        U32 tbl = V->ip->operand1;

        TValue *val = __get_register(V, tbl);
        int size = val->cast_ptr<TTable>()->data.size();
        TValue val_size(size);

        __set_register(V, dst, val_size);
        VM_NEXT();
    }

    case LENSTRING: {
        U32 rdst = V->ip->operand0;
        U32 objr = V->ip->operand1;

        TValue *val = __get_register(V, objr);
        int len = val->cast_ptr<TString>()->len;
        TValue val_len(len);

        __set_register(V, rdst, val_len);
        VM_NEXT();
    }

    case GETSTRING: {
        U32 dst = V->ip->operand0;
        U32 str = V->ip->operand1;
        U32 idx = V->ip->operand2;

        TValue *str_val = __get_register(V, str);
        TValue *idx_val = __get_register(V, idx);

        size_t index = idx_val->val_integer;
        if VIA_UNLIKELY (index > str_val->cast_ptr<TString>()->len) {
            __set_register(V, dst, _Nil);
        }

        VM_NEXT();
    }

    case LEN: {
        U32 rdst = V->ip->operand0;
        U32 objr = V->ip->operand1;

        TValue *val = __get_register(V, objr);
        TValue len = __len(V, *val);

        __set_register(V, rdst, len);
        VM_NEXT();
    }

    case TYPE: {
        U32 rdst = V->ip->operand0;
        U32 objr = V->ip->operand1;

        TValue *val = __get_register(V, objr);
        TValue ty = __type(V, *val);

        __set_register(V, rdst, ty);
        VM_NEXT();
    }

    case TYPEOF: {
        U32 rdst = V->ip->operand0;
        U32 objr = V->ip->operand1;

        TValue *val = __get_register(V, objr);
        TValue type = __typeofv(V, *val);

        __set_register(V, rdst, type);
        VM_NEXT();
    }

    default: {
        VM_FATAL(std::format("unknown opcode 0x{:x}", static_cast<int>(V->ip->op)));
    }
    }
}

exit:
    V->sig_exit.fire();
    V->tstate = ThreadState::PAUSED;
}

// Permanently kills the thread. Does not clean up the state object.
void kill_thread(State *VIA_RESTRICT V)
{
    if (V->tstate == ThreadState::RUNNING) {
        V->abort = true;
        V->sig_exit.wait();
    }

    // Mark as dead thread
    V->tstate = ThreadState::DEAD;
    // Decrement the thread_id to make room for more threads (I know you
    // can techni__cally make 2^32 threads ok?)
    V->G->threads.fetch_add(-1);
}

// Temporarily pauses the thread.
void pause_thread(State *VIA_RESTRICT V)
{
    if (V->tstate == ThreadState::RUNNING) {
        V->abort = true;
        V->sig_exit.wait();
    }

    V->tstate = ThreadState::PAUSED;
}

} // namespace via
