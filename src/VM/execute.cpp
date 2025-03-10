// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "execute.h"
#include "fileio.h"
#include "vmapi.h"
#include "bitutils.h"
#include "chunk.h"
#include "common.h"
#include "state.h"
#include "rttypes.h"
#include "constant.h"
#include "vaux.h"
#include <cmath>

// How many times a chunk needs to be executed to be flagged as "hot"
#define VIA_HOTPATH_THRESHOLD 64

// Index of the current instruction
#define VM_POS                 (V->ip - V->ibp)
#define VM_CHECK_JMP(addr)     ((addr >= V->ibp) && (addr <= V->iep))
#define VM_CHECK_OPERAND(oper) (oper != VIA_OPERAND_INVALID)

#define VM_ERROR(message)                                                                          \
    do {                                                                                           \
        __set_error_state(V, message);                                                             \
        V->sig_error.fire();                                                                       \
        goto dispatch;                                                                             \
    } while (0)

#define VM_FATAL(message)                                                                          \
    do {                                                                                           \
        std::cerr << "VM terminated with message: " << message << '\n';                            \
        V->sig_fatal.fire();                                                                       \
        std::abort();                                                                              \
    } while (0)

// Macro for loading the next instruction
// Has bound checks
#define VM_LOAD()                                                                                  \
    do {                                                                                           \
        if (!VM_CHECK_JMP(V->ip + 1)) {                                                            \
            goto exit;                                                                             \
        }                                                                                          \
        V->ip++;                                                                                   \
    } while (0)

// Macro that "signals" the VM has completed an execution cycle
#define VM_NEXT()                                                                                  \
    do {                                                                                           \
        VM_LOAD();                                                                                 \
        goto dispatch;                                                                             \
    } while (0)

VIA_NAMESPACE_BEGIN

void vm_save_snapshot(State* VIA_RESTRICT V) {
    U64         pos  = V->ip - V->ibp;
    std::string file = std::format("vm_snapshot.{}.log", pos);

    std::ostringstream headers;
    headers << "opcode: " << magic_enum::enum_name(V->ip->op) << "\n";

    std::ostringstream registers;
    registers << "==== registers ====\n";

    std::ostringstream stack;
    stack << "==== stack ====\n";

    // Generate stack map
    for (TValue* ptr = V->sbp; ptr < V->sbp + V->sp; ptr++) {
        U32 pos = ptr - V->sbp;
        stack << "|" << std::setw(2) << std::setfill('0') << pos << "| "
              << impl::__to_cxx_string(V, *ptr) << ' ' << memdump(ptr, sizeof(TValue));
    }
    stack << "==== stack ====\n";

    // Generate register map
    for (Operand reg = 0; reg < VIA_REGISTER_COUNT; reg++) {
        TValue* val = impl::__get_register(V, reg);
        if (!check_nil(*val)) {
            registers << "|R" << std::setw(2) << std::setfill('0') << reg << "| "
                      << impl::__to_cxx_string(V, *val) << ' ' << memdump(val, sizeof(TValue));
        }
    }
    registers << "==== registers ====\n";

    // Combine all parts and write to file
    std::ostringstream output;
    output << headers.str() << "\n"
           << to_string(V) << "\n"
           << stack.str() << "\n"
           << registers.str();

    try {
        utils::write_to_file(std::format("./__viacache__/{}", file), output.str());
    }
    catch (const std::exception&) {
    }
}

// Starts VM execution cycle by altering it's state and "iterating" over
// the instruction pipeline.
void execute(State* VIA_RESTRICT V) {
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
    // Handle special/internal opcodes
    case NOP:
    case LABEL:
    case CAPTURE:
        VM_NEXT();

    case ADD: {
        Operand lhs = V->ip->operand0;
        Operand rhs = V->ip->operand1;

        TValue* lhs_val = __get_register(V, lhs);
        TValue* rhs_val = __get_register(V, rhs);

        ValueType lhs_type = lhs_val->type;
        ValueType rhs_type = rhs_val->type;

        if VIA_LIKELY (lhs_type == integer) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_integer += rhs_val->val_integer;
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    static_cast<TFloat>(lhs_val->val_integer) + rhs_val->val_floating_point;
                lhs_val->type = floating_point;
            }
        }
        else if (lhs_type == floating_point) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_floating_point += static_cast<TFloat>(rhs_val->val_integer);
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point += rhs_val->val_floating_point;
            }
        }

        VM_NEXT();
    }
    case ADDK: {
        Operand lhs = V->ip->operand0;
        Operand idx = V->ip->operand1;

        TValue*       lhs_val = __get_register(V, lhs);
        const TValue& rhs_val = __get_constant(V, idx);

        ValueType lhs_type = lhs_val->type;
        ValueType rhs_type = rhs_val.type;

        if VIA_LIKELY (lhs_type == integer) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_integer += rhs_val.val_integer;
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    static_cast<TFloat>(lhs_val->val_integer) + rhs_val.val_floating_point;
                lhs_val->type = floating_point;
            }
        }
        else if (lhs_type == floating_point) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_floating_point += static_cast<TFloat>(rhs_val.val_integer);
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point += rhs_val.val_floating_point;
            }
        }

        VM_NEXT();
    }

    case SUB: {
        Operand lhs = V->ip->operand0;
        Operand rhs = V->ip->operand1;

        TValue* lhs_val = __get_register(V, lhs);
        TValue* rhs_val = __get_register(V, rhs);

        ValueType lhs_type = lhs_val->type;
        ValueType rhs_type = rhs_val->type;

        if VIA_LIKELY (lhs_type == integer) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_integer -= rhs_val->val_integer;
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    static_cast<TFloat>(lhs_val->val_integer) - rhs_val->val_floating_point;
                lhs_val->type = floating_point;
            }
        }
        else if (lhs_type == floating_point) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_floating_point -= static_cast<TFloat>(rhs_val->val_integer);
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point -= rhs_val->val_floating_point;
            }
        }

        VM_NEXT();
    }
    case SUBK: {
        Operand lhs = V->ip->operand0;
        Operand idx = V->ip->operand1;

        TValue*       lhs_val = __get_register(V, lhs);
        const TValue& rhs_val = __get_constant(V, idx);

        ValueType lhs_type = lhs_val->type;
        ValueType rhs_type = rhs_val.type;

        if VIA_LIKELY (lhs_type == integer) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_integer -= rhs_val.val_integer;
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    static_cast<TFloat>(lhs_val->val_integer) - rhs_val.val_floating_point;
                lhs_val->type = floating_point;
            }
        }
        else if (lhs_type == floating_point) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_floating_point -= static_cast<TFloat>(rhs_val.val_integer);
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point -= rhs_val.val_floating_point;
            }
        }

        VM_NEXT();
    }

    case MUL: {
        Operand lhs = V->ip->operand0;
        Operand rhs = V->ip->operand1;

        TValue* lhs_val = __get_register(V, lhs);
        TValue* rhs_val = __get_register(V, rhs);

        ValueType lhs_type = lhs_val->type;
        ValueType rhs_type = rhs_val->type;

        if VIA_LIKELY (lhs_type == integer) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_integer *= rhs_val->val_integer;
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    static_cast<TFloat>(lhs_val->val_integer) * rhs_val->val_floating_point;
                lhs_val->type = floating_point;
            }
        }
        else if (lhs_type == floating_point) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_floating_point *= static_cast<TFloat>(rhs_val->val_integer);
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point *= rhs_val->val_floating_point;
            }
        }

        VM_NEXT();
    }
    case MULK: {
        Operand lhs = V->ip->operand0;
        Operand idx = V->ip->operand1;

        TValue*       lhs_val = __get_register(V, lhs);
        const TValue& rhs_val = __get_constant(V, idx);

        ValueType lhs_type = lhs_val->type;
        ValueType rhs_type = rhs_val.type;

        if VIA_LIKELY (lhs_type == integer) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_integer *= rhs_val.val_integer;
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    static_cast<TFloat>(lhs_val->val_integer) * rhs_val.val_floating_point;
                lhs_val->type = floating_point;
            }
        }
        else if (lhs_type == floating_point) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_floating_point *= static_cast<TFloat>(rhs_val.val_integer);
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point *= rhs_val.val_floating_point;
            }
        }

        VM_NEXT();
    }

    case DIV: {
        Operand lhs = V->ip->operand0;
        Operand rhs = V->ip->operand1;

        TValue* lhs_val = __get_register(V, lhs);
        TValue* rhs_val = __get_register(V, rhs);

        ValueType lhs_type = lhs_val->type;
        ValueType rhs_type = rhs_val->type;

        if VIA_LIKELY (lhs_type == integer) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_integer /= rhs_val->val_integer;
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    static_cast<TFloat>(lhs_val->val_integer) / rhs_val->val_floating_point;
                lhs_val->type = floating_point;
            }
        }
        else if (lhs_type == floating_point) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_floating_point /= static_cast<TFloat>(rhs_val->val_integer);
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point /= rhs_val->val_floating_point;
            }
        }

        VM_NEXT();
    }
    case DIVK: {
        Operand lhs = V->ip->operand0;
        Operand idx = V->ip->operand1;

        TValue*       lhs_val = __get_register(V, lhs);
        const TValue& rhs_val = __get_constant(V, idx);

        ValueType lhs_type = lhs_val->type;
        ValueType rhs_type = rhs_val.type;

        if VIA_LIKELY (lhs_type == integer) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_integer /= rhs_val.val_integer;
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    static_cast<TFloat>(lhs_val->val_integer) / rhs_val.val_floating_point;
                lhs_val->type = floating_point;
            }
        }
        else if (lhs_type == floating_point) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_floating_point /= static_cast<TFloat>(rhs_val.val_integer);
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point /= rhs_val.val_floating_point;
            }
        }

        VM_NEXT();
    }

    case POW: {
        Operand lhs = V->ip->operand0;
        Operand rhs = V->ip->operand1;

        TValue* lhs_val = __get_register(V, lhs);
        TValue* rhs_val = __get_register(V, rhs);

        ValueType lhs_type = lhs_val->type;
        ValueType rhs_type = rhs_val->type;

        if VIA_LIKELY (lhs_type == integer) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_integer = std::pow(lhs_val->val_integer, rhs_val->val_integer);
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point = std::pow(
                    static_cast<TFloat>(lhs_val->val_integer), rhs_val->val_floating_point
                );
                lhs_val->type = floating_point;
            }
        }
        else if (lhs_type == floating_point) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_floating_point = std::pow(
                    lhs_val->val_floating_point, static_cast<TFloat>(rhs_val->val_integer)
                );
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    std::pow(lhs_val->val_floating_point, rhs_val->val_floating_point);
            }
        }

        VM_NEXT();
    }
    case POWK: {
        Operand lhs = V->ip->operand0;
        Operand idx = V->ip->operand1;

        TValue*       lhs_val = __get_register(V, lhs);
        const TValue& rhs_val = __get_constant(V, idx);

        ValueType lhs_type = lhs_val->type;
        ValueType rhs_type = rhs_val.type;

        if VIA_LIKELY (lhs_type == integer) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_integer = std::pow(lhs_val->val_integer, rhs_val.val_integer);
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    std::pow(static_cast<TFloat>(lhs_val->val_integer), rhs_val.val_floating_point);
                lhs_val->type = floating_point;
            }
        }
        else if (lhs_type == floating_point) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_floating_point =
                    std::pow(lhs_val->val_floating_point, static_cast<TFloat>(rhs_val.val_integer));
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    std::pow(lhs_val->val_floating_point, rhs_val.val_floating_point);
            }
        }

        VM_NEXT();
    }

    case MOD: {
        Operand lhs = V->ip->operand0;
        Operand rhs = V->ip->operand1;

        TValue* lhs_val = __get_register(V, lhs);
        TValue* rhs_val = __get_register(V, rhs);

        ValueType lhs_type = lhs_val->type;
        ValueType rhs_type = rhs_val->type;

        if VIA_LIKELY (lhs_type == integer) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_integer %= rhs_val->val_integer;
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point = std::fmod(
                    static_cast<TFloat>(lhs_val->val_integer), rhs_val->val_floating_point
                );
                lhs_val->type = floating_point;
            }
        }
        else if (lhs_type == floating_point) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_floating_point = std::fmod(
                    lhs_val->val_floating_point, static_cast<TFloat>(rhs_val->val_integer)
                );
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    std::fmod(lhs_val->val_floating_point, rhs_val->val_floating_point);
            }
        }

        VM_NEXT();
    }
    case MODK: {
        Operand lhs = V->ip->operand0;
        Operand idx = V->ip->operand1;

        TValue*       lhs_val = __get_register(V, lhs);
        const TValue& rhs_val = __get_constant(V, idx);

        ValueType lhs_type = lhs_val->type;
        ValueType rhs_type = rhs_val.type;

        if VIA_LIKELY (lhs_type == integer) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_integer %= rhs_val.val_integer;
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point = std::fmod(
                    static_cast<TFloat>(lhs_val->val_integer), rhs_val.val_floating_point
                );
                lhs_val->type = floating_point;
            }
        }
        else if (lhs_type == floating_point) {
            if VIA_LIKELY (rhs_type == integer) {
                lhs_val->val_floating_point = std::fmod(
                    lhs_val->val_floating_point, static_cast<TFloat>(rhs_val.val_integer)
                );
            }
            else if VIA_UNLIKELY (rhs_type == floating_point) {
                lhs_val->val_floating_point =
                    std::fmod(lhs_val->val_floating_point, rhs_val.val_floating_point);
            }
        }

        VM_NEXT();
    }

    case MOVE: {
        Operand rdst    = V->ip->operand0;
        Operand rsrc    = V->ip->operand1;
        TValue* src_val = __get_register(V, rsrc);

        __set_register(V, rdst, *src_val);
        VM_NEXT();
    }

    case LOADK: {
        Operand dst = V->ip->operand0;
        Operand idx = V->ip->operand1;

        const TValue& kval = __get_constant(V, idx);

        __set_register(V, dst, kval);
        VM_NEXT();
    }

    case LOADNIL: {
        Operand dst = V->ip->operand0;

        __set_register(V, dst, _Nil);
        VM_NEXT();
    }

    case LOADINT: {
        Operand  dst = V->ip->operand0;
        TInteger imm = reinterpret_u16_as_u32(V->ip->operand1, V->ip->operand2);

        __set_register(V, dst, TValue(imm));
        VM_NEXT();
    }

    case LOADFLOAT: {
        Operand dst = V->ip->operand0;
        TFloat  imm = reinterpret_u16_as_f32(V->ip->operand1, V->ip->operand2);

        __set_register(V, dst, TValue(imm));
        VM_NEXT();
    }

    case LOADTRUE: {
        Operand dst = V->ip->operand0;
        __set_register(V, dst, TValue(true));
        VM_NEXT();
    }

    case LOADFALSE: {
        Operand dst = V->ip->operand0;
        __set_register(V, dst, TValue(false));
        VM_NEXT();
    }

    case LOADTABLE: {
        Operand dst = V->ip->operand0;
        TValue  ttable(new TTable());

        __set_register(V, dst, ttable);
        VM_NEXT();
    }

    case LOADFUNCTION: {
        Operand    dst  = V->ip->operand0;
        TFunction* func = new TFunction;

        __closure_bytecode_load(V, func);
        __set_register(V, dst, TValue(func));
        VM_NEXT();
    }

    case GETUPVALUE: {
        Operand  dst    = V->ip->operand0;
        Operand  upv_id = V->ip->operand1;
        UpValue* upv    = __closure_upv_get(V->frame, upv_id);

        __set_register(V, dst, *upv->value);
        VM_NEXT();
    }

    case SETUPVALUE: {
        Operand src    = V->ip->operand0;
        Operand upv_id = V->ip->operand1;
        TValue* val    = __get_register(V, src);

        __closure_upv_set(V->frame, upv_id, *val);
        VM_NEXT();
    }

    case PUSH: {
        Operand src = V->ip->operand0;
        TValue* val = __get_register(V, src);

        __push(V, *val);
        VM_NEXT();
    }

    case PUSHK: {
        Operand const_idx = V->ip->operand0;
        TValue  constant  = __get_constant(V, const_idx);

        __push(V, constant);
        VM_NEXT();
    }

    case PUSHNIL: {
        __push(V, TValue());
        VM_NEXT();
    }

    case PUSHINT: {
        TInteger imm = reinterpret_u16_as_u32(V->ip->operand0, V->ip->operand1);
        __push(V, TValue(imm));
        VM_NEXT();
    }

    case PUSHFLOAT: {
        TFloat imm = reinterpret_u16_as_f32(V->ip->operand0, V->ip->operand1);
        __push(V, TValue(imm));
        VM_NEXT();
    }

    case PUSHTRUE: {
        __push(V, TValue(true));
        VM_NEXT();
    }

    case PUSHFALSE: {
        __push(V, TValue(false));
        VM_NEXT();
    }

    case POP: {
        Operand dst = V->ip->operand0;
        TValue  val = __pop(V);

        __set_register(V, dst, val);
        VM_NEXT();
    }

    case DROP: {
        __pop(V);
        VM_NEXT();
    }

    case GETSTACK: {
        Operand dst = V->ip->operand0;
        Operand off = V->ip->operand1;

        const TValue& val = __get_stack(V, off);

        __set_register(V, dst, val);
        VM_NEXT();
    }

    case SETSTACK: {
        Operand src = V->ip->operand0;
        Operand off = V->ip->operand1;

        TValue* val = __get_register(V, src);

        V->sbp[off] = std::move(*val);
        VM_NEXT();
    }

    case GETARGUMENT: {
        Operand dst = V->ip->operand0;
        Operand off = V->ip->operand1;

        const TValue& val = __get_argument(V, off);

        __set_register(V, dst, val);
        VM_NEXT();
    }

    case GETGLOBAL: {
        Operand dst = V->ip->operand0;

        U32           hash   = reinterpret_u16_as_u32(V->ip->operand1, V->ip->operand2);
        const TValue& global = __get_global(V, hash);

        __set_register(V, dst, global);
        VM_NEXT();
    }

    case SETGLOBAL: {
        Operand src = V->ip->operand0;

        U32     hash   = reinterpret_u16_as_u32(V->ip->operand1, V->ip->operand2);
        TValue* global = __get_register(V, src);

        __set_global(V, hash, *global);
        VM_NEXT();
    }

    case EQUAL: {
        Operand dst = V->ip->operand0;
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        if VIA_UNLIKELY (lhs == rhs) {
            __set_register(V, dst, TValue(true));
            VM_NEXT();
        }

        TValue* lhs_val = __get_register(V, lhs);
        TValue* rhs_val = __get_register(V, rhs);

        if VIA_UNLIKELY (lhs_val == rhs_val) {
            __set_register(V, dst, TValue(true));
            VM_NEXT();
        }

        bool result = __compare(*lhs_val, *rhs_val);
        __set_register(V, dst, TValue(result));

        VM_NEXT();
    }

    case NOTEQUAL: {
        Operand dst = V->ip->operand0;
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        if VIA_LIKELY (lhs != rhs) {
            __set_register(V, dst, TValue(true));
            VM_NEXT();
        }

        TValue* lhs_val = __get_register(V, lhs);
        TValue* rhs_val = __get_register(V, rhs);

        if VIA_LIKELY (lhs_val != rhs_val) {
            __set_register(V, dst, TValue(true));
            VM_NEXT();
        }

        bool result = __compare(*lhs_val, *rhs_val);
        __set_register(V, dst, TValue(result));

        VM_NEXT();
    }

    case AND: {
        Operand dst = V->ip->operand0;
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue* lhs_val = __get_register(V, lhs);
        TValue* rhs_val = __get_register(V, rhs);
        bool    cond    = __to_cxx_bool(*lhs_val) && __to_cxx_bool(*rhs_val);

        __set_register(V, dst, TValue(cond));
        VM_NEXT();
    }

    case OR: {
        Operand dst = V->ip->operand0;
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue* lhs_val = __get_register(V, lhs);
        TValue* rhs_val = __get_register(V, rhs);
        bool    cond    = __to_cxx_bool(*lhs_val) || __to_cxx_bool(*rhs_val);

        __set_register(V, dst, TValue(cond));
        VM_NEXT();
    }

    case NOT: {
        Operand dst = V->ip->operand0;
        Operand lhs = V->ip->operand1;

        TValue* lhs_val = __get_register(V, lhs);
        bool    cond    = !__to_cxx_bool(*lhs_val);

        __set_register(V, dst, TValue(cond));
        VM_NEXT();
    }

    case LESS: {
        Operand dst = V->ip->operand0;
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue* lhs_val = __get_register(V, lhs);
        TValue* rhs_val = __get_register(V, rhs);

        if VIA_LIKELY (check_integer(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                __set_register(V, dst, TValue(lhs_val->val_integer < rhs_val->val_integer));
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                __set_register(
                    V,
                    dst,
                    TValue(static_cast<TFloat>(lhs_val->val_integer) < rhs_val->val_floating_point)
                );
            }
        }
        else if VIA_UNLIKELY (check_floating_point(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                __set_register(
                    V,
                    dst,
                    TValue(lhs_val->val_floating_point < static_cast<TFloat>(rhs_val->val_integer))
                );
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                __set_register(
                    V, dst, TValue(lhs_val->val_floating_point < rhs_val->val_floating_point)
                );
            }
        }

        VM_NEXT();
    }

    case GREATER: {
        Operand dst = V->ip->operand0;
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue* lhs_val = __get_register(V, lhs);
        TValue* rhs_val = __get_register(V, rhs);

        if VIA_LIKELY (check_integer(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                __set_register(V, dst, TValue(lhs_val->val_integer > rhs_val->val_integer));
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                __set_register(
                    V,
                    dst,
                    TValue(static_cast<TFloat>(lhs_val->val_integer) > rhs_val->val_floating_point)
                );
            }
        }
        else if VIA_UNLIKELY (check_floating_point(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                __set_register(
                    V,
                    dst,
                    TValue(lhs_val->val_floating_point > static_cast<TFloat>(rhs_val->val_integer))
                );
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                __set_register(
                    V, dst, TValue(lhs_val->val_floating_point > rhs_val->val_floating_point)
                );
            }
        }

        VM_NEXT();
    }

    case LESSOREQUAL: {
        Operand dst = V->ip->operand0;
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue* lhs_val = __get_register(V, lhs);
        TValue* rhs_val = __get_register(V, rhs);

        if VIA_LIKELY (check_integer(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                __set_register(V, dst, TValue(lhs_val->val_integer <= rhs_val->val_integer));
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                __set_register(
                    V,
                    dst,
                    TValue(static_cast<TFloat>(lhs_val->val_integer) <= rhs_val->val_floating_point)
                );
            }
        }
        else if VIA_UNLIKELY (check_floating_point(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                __set_register(
                    V,
                    dst,
                    TValue(lhs_val->val_floating_point <= static_cast<TFloat>(rhs_val->val_integer))
                );
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                __set_register(
                    V, dst, TValue(lhs_val->val_floating_point <= rhs_val->val_floating_point)
                );
            }
        }

        VM_NEXT();
    }

    case GREATEROREQUAL: {
        Operand dst = V->ip->operand0;
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue* lhs_val = __get_register(V, lhs);
        TValue* rhs_val = __get_register(V, rhs);

        if VIA_LIKELY (check_integer(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                __set_register(V, dst, TValue(lhs_val->val_integer >= rhs_val->val_integer));
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                __set_register(
                    V,
                    dst,
                    TValue(static_cast<TFloat>(lhs_val->val_integer) >= rhs_val->val_floating_point)
                );
            }
        }
        else if VIA_UNLIKELY (check_floating_point(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                __set_register(
                    V,
                    dst,
                    TValue(lhs_val->val_floating_point >= static_cast<TFloat>(rhs_val->val_integer))
                );
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                __set_register(
                    V, dst, TValue(lhs_val->val_floating_point >= rhs_val->val_floating_point)
                );
            }
        }

        VM_NEXT();
    }

    case EXIT: {
        goto exit;
    }

    case JUMP: {
        OperandS offset = V->ip->operand0;
        V->ip += offset;
        goto dispatch;
    }

    case JUMPIF: {
        Operand  cond   = V->ip->operand0;
        OperandS offset = V->ip->operand1;

        TValue* cond_val = __get_register(V, cond);
        if (__to_cxx_bool(*cond_val)) {
            V->ip += offset;
        }

        goto dispatch;
    }

    case JUMPIFNOT: {
        Operand  cond   = V->ip->operand0;
        OperandS offset = V->ip->operand1;

        TValue* cond_val = __get_register(V, cond);
        if (!__to_cxx_bool(*cond_val)) {
            V->ip += offset;
        }

        goto dispatch;
    }

    case JUMPIFEQUAL: {
        Operand  cond_lhs = V->ip->operand0;
        Operand  cond_rhs = V->ip->operand1;
        OperandS offset   = V->ip->operand2;

        if VIA_UNLIKELY (cond_lhs == cond_rhs) {
            V->ip += offset;
        }
        else {
            TValue* lhs_val = __get_register(V, cond_lhs);
            TValue* rhs_val = __get_register(V, cond_rhs);

            if VIA_UNLIKELY (lhs_val == rhs_val || __compare(*lhs_val, *rhs_val)) {
                V->ip += offset;
            }
        }

        goto dispatch;
    }

    case JUMPIFNOTEQUAL: {
        Operand  cond_lhs = V->ip->operand0;
        Operand  cond_rhs = V->ip->operand1;
        OperandS offset   = V->ip->operand2;

        if VIA_LIKELY (cond_lhs != cond_rhs) {
            V->ip += offset;
        }
        else {
            TValue* lhs_val = __get_register(V, cond_lhs);
            TValue* rhs_val = __get_register(V, cond_rhs);

            if VIA_LIKELY (lhs_val != rhs_val || !__compare(*lhs_val, *rhs_val)) {
                V->ip += offset;
            }
        }

        goto dispatch;
    }

    case JUMPIFLESS: {
        Operand  cond_lhs = V->ip->operand0;
        Operand  cond_rhs = V->ip->operand1;
        OperandS offset   = V->ip->operand2;

        TValue* lhs_val = __get_register(V, cond_lhs);
        TValue* rhs_val = __get_register(V, cond_rhs);

        if VIA_LIKELY (check_integer(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                if (lhs_val->val_integer < rhs_val->val_integer) {
                    V->ip += offset;
                }
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                if (static_cast<TFloat>(lhs_val->val_integer) < rhs_val->val_floating_point) {
                    V->ip += offset;
                }
            }
        }
        else if VIA_UNLIKELY (check_floating_point(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                if (lhs_val->val_floating_point < static_cast<TFloat>(rhs_val->val_integer)) {
                    V->ip += offset;
                }
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                if (lhs_val->val_floating_point < rhs_val->val_floating_point) {
                    V->ip += offset;
                }
            }
        }

        goto dispatch;
    }

    case JUMPIFGREATER: {
        Operand  cond_lhs = V->ip->operand0;
        Operand  cond_rhs = V->ip->operand1;
        OperandS offset   = V->ip->operand2;

        TValue* lhs_val = __get_register(V, cond_lhs);
        TValue* rhs_val = __get_register(V, cond_rhs);

        if VIA_LIKELY (check_integer(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                if (lhs_val->val_integer > rhs_val->val_integer) {
                    V->ip += offset;
                }
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                if (static_cast<TFloat>(lhs_val->val_integer) > rhs_val->val_floating_point) {
                    V->ip += offset;
                }
            }
        }
        else if VIA_UNLIKELY (check_floating_point(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                if (lhs_val->val_floating_point > static_cast<TFloat>(rhs_val->val_integer)) {
                    V->ip += offset;
                }
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                if (lhs_val->val_floating_point > rhs_val->val_floating_point) {
                    V->ip += offset;
                }
            }
        }

        goto dispatch;
    }

    case JUMPIFLESSOREQUAL: {
        Operand  cond_lhs = V->ip->operand0;
        Operand  cond_rhs = V->ip->operand1;
        OperandS offset   = V->ip->operand2;

        TValue* lhs_val = __get_register(V, cond_lhs);
        TValue* rhs_val = __get_register(V, cond_rhs);

        if VIA_LIKELY (check_integer(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                if (lhs_val->val_integer <= rhs_val->val_integer) {
                    V->ip += offset;
                }
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                if (static_cast<TFloat>(lhs_val->val_integer) <= rhs_val->val_floating_point) {
                    V->ip += offset;
                }
            }
        }
        else if VIA_UNLIKELY (check_floating_point(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                if (lhs_val->val_floating_point <= static_cast<TFloat>(rhs_val->val_integer)) {
                    V->ip += offset;
                }
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                if (lhs_val->val_floating_point <= rhs_val->val_floating_point) {
                    V->ip += offset;
                }
            }
        }

        goto dispatch;
    }

    case JUMPIFGREATEROREQUAL: {
        Operand  cond_lhs = V->ip->operand0;
        Operand  cond_rhs = V->ip->operand1;
        OperandS offset   = V->ip->operand2;

        TValue* lhs_val = __get_register(V, cond_lhs);
        TValue* rhs_val = __get_register(V, cond_rhs);

        if VIA_LIKELY (check_integer(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                if (lhs_val->val_integer >= rhs_val->val_integer) {
                    V->ip += offset;
                }
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                if (static_cast<TFloat>(lhs_val->val_integer) >= rhs_val->val_floating_point) {
                    V->ip += offset;
                }
            }
        }
        else if VIA_UNLIKELY (check_floating_point(*lhs_val)) {
            if VIA_LIKELY (check_integer(*rhs_val)) {
                if (lhs_val->val_floating_point >= static_cast<TFloat>(rhs_val->val_integer)) {
                    V->ip += offset;
                }
            }
            else if VIA_UNLIKELY (check_floating_point(*rhs_val)) {
                if (lhs_val->val_floating_point >= rhs_val->val_floating_point) {
                    V->ip += offset;
                }
            }
        }

        goto dispatch;
    }

    case CALL: {
        Operand fn     = V->ip->operand0;
        Operand argc   = V->ip->operand1;
        TValue* fn_val = __get_register(V, fn);

        __call(V, *fn_val, argc);
        VM_NEXT();
    }
    case EXTERNCALL: {
        Operand fn    = V->ip->operand0;
        Operand argc  = V->ip->operand1;
        TValue* cfunc = __get_register(V, fn);

        __extern_call(V, cfunc->cast_ptr<TCFunction>(), argc);
        VM_NEXT();
    }
    case NATIVECALL: {
        Operand fn   = V->ip->operand0;
        Operand argc = V->ip->operand1;
        TValue* func = __get_register(V, fn);

        __native_call(V, func->cast_ptr<TFunction>(), argc);
        VM_NEXT();
    }
    case METHODCALL: {
        Operand obj  = V->ip->operand0;
        Operand fn   = V->ip->operand1;
        Operand argc = V->ip->operand2;

        TValue* func   = __get_register(V, fn);
        TValue* object = __get_register(V, obj);

        __push(V, object->clone());
        __native_call(V, func->cast_ptr<TFunction>(), argc + 1);
        VM_NEXT();
    }

    case RETURN: {
        Operand src = V->ip->operand0;
        TValue* val = __get_register(V, src);

        __closure_close_upvalues(V->frame);
        __native_return(V, *val);
        VM_NEXT();
    }

    case GETTABLE: {
        Operand dst = V->ip->operand0;
        Operand tbl = V->ip->operand1;
        Operand key = V->ip->operand2;

        TValue* tbl_val = __get_register(V, tbl);
        TValue* key_val = __get_register(V, key);

        const TValue& index = __table_get(tbl_val->cast_ptr<TTable>(), *key_val);

        __set_register(V, dst, index);
        VM_NEXT();
    }

    case SETTABLE: {
        Operand src = V->ip->operand0;
        Operand tbl = V->ip->operand1;
        Operand ky  = V->ip->operand2;

        TValue* table = __get_register(V, tbl);
        TValue* value = __get_register(V, src);
        TValue* key   = __get_register(V, ky);

        __table_set(table->cast_ptr<TTable>(), TValue(key), *value);
        VM_NEXT();
    }

    case NEXTTABLE: {
        static std::unordered_map<void*, Operand> next_table;

        Operand dst  = V->ip->operand0;
        Operand valr = V->ip->operand1;

        TValue* val = __get_register(V, valr);
        void*   ptr = __to_pointer(*val);
        Operand key = 0;

        auto it = next_table.find(ptr);
        if (it != next_table.end()) {
            key = ++it->second;
        }
        else {
            next_table[ptr] = 0;
        }

        const TValue& field = __table_get(val->cast_ptr<TTable>(), TValue(key));
        __set_register(V, dst, field);
        VM_NEXT();
    }

    case LENTABLE: {
        Operand dst = V->ip->operand0;
        Operand tbl = V->ip->operand1;

        TValue* val  = __get_register(V, tbl);
        int     size = __table_size(val->cast_ptr<TTable>());
        TValue  val_size(size);

        __set_register(V, dst, val_size);
        VM_NEXT();
    }

    case LENSTRING: {
        Operand rdst = V->ip->operand0;
        Operand objr = V->ip->operand1;

        TValue* val = __get_register(V, objr);
        int     len = val->cast_ptr<TString>()->len;
        TValue  val_len(len);

        __set_register(V, rdst, val_len);
        VM_NEXT();
    }

    case GETSTRING: {
        Operand dst = V->ip->operand0;
        Operand str = V->ip->operand1;
        Operand idx = V->ip->operand2;

        TValue* str_val = __get_register(V, str);
        TValue* idx_val = __get_register(V, idx);

        SIZE index = idx_val->val_integer;
        if VIA_UNLIKELY (index > str_val->cast_ptr<TString>()->len) {
            __set_register(V, dst, _Nil);
        }

        VM_NEXT();
    }

    case LEN: {
        Operand rdst = V->ip->operand0;
        Operand objr = V->ip->operand1;

        TValue* val = __get_register(V, objr);
        TValue  len = __len(*val);

        __set_register(V, rdst, len);
        VM_NEXT();
    }

    case TYPE: {
        Operand rdst = V->ip->operand0;
        Operand objr = V->ip->operand1;

        TValue* val = __get_register(V, objr);
        TValue  ty  = __type(V, *val);

        __set_register(V, rdst, ty);
        VM_NEXT();
    }

    case TYPEOF: {
        Operand rdst = V->ip->operand0;
        Operand objr = V->ip->operand1;

        TValue* val  = __get_register(V, objr);
        TValue  type = __typeofv(V, *val);

        __set_register(V, rdst, type);
        VM_NEXT();
    }

    case GET: {
        Operand dst = V->ip->operand0;
        Operand obj = V->ip->operand1;
        Operand key = V->ip->operand2;

        TValue* obj_val = __get_register(V, obj);
        TValue* key_val = __get_register(V, key);

        if VIA_LIKELY (check_table(*obj_val)) {
            const TValue& val = __table_get(obj_val->cast_ptr<TTable>(), *key_val);
            __set_register(V, dst, val);
            VM_NEXT();
        }
        else if (check_string(*obj_val)) {
        }

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
void kill_thread(State* VIA_RESTRICT V) {
    if (V->tstate == ThreadState::RUNNING) {
        V->abort = true;
        V->sig_exit.wait();
    }

    // Mark as dead thread
    V->tstate = ThreadState::DEAD;
    V->G->threads.fetch_add(-1);
}

// Temporarily pauses the thread.
void pause_thread(State* VIA_RESTRICT V) {
    if (V->tstate == ThreadState::RUNNING) {
        V->abort = true;
        V->sig_exit.wait();
    }

    V->tstate = ThreadState::PAUSED;
}

VIA_NAMESPACE_END
