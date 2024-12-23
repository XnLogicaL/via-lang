/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "bytecode.h"
#include "core.h"
#include "except.h"
#include "gc.h"
#include "instruction.h"
#include "opcode.h"
#include "register.h"
#include "shared.h"
#include "stack.h"
#include "state.h"
#include "types.h"
#include <cmath>

namespace via
{

// Manually sets the VM exit data
// ! Internal usage only, made public for libraries
inline void via_setexitdata(viaState *V, ExitCode exitc, const std::string &exitm) noexcept
{
    V->exitc = exitc;
    // I don't know why but `strdup` is deprecated but `_strdup` isn't
    // This is the completely opposite of the deprecation convention, but sure...
    V->exitm = strdup(exitm.c_str());
}

inline bool via_validjmpaddr(viaState *V, const Instruction *addr) noexcept
{
    return (addr >= V->ihp) && (addr <= V->ibp);
}

// Appends a pointer to the garbage collector free list
// The pointer should be malloc-ed, otherwise this has undefined behavior
// ! The value cannot be removed from the free list!
inline void via_gcadd(viaState *V, viaValue *ptr) noexcept
{
    viaGC_add(V, ptr);
}

// Invokes garbage collection
// Should be called in intervals
inline void via_gccol(viaState *V) noexcept
{
    viaGC_collect(V);
}

// Asserts a condition, terminates the VM if not met
inline void via_assert(viaState *V, bool cond, const std::string &err) noexcept
{
    if (!cond)
    {
        via_setexitdata(V, 1, std::format("VM assertion failed: {}", err));
        V->abrt = true;
    }
}

// Throws an unrecoverable error that terminates the VM
inline void via_fatalerr(viaState *V, const std::string &err)
{
    std::cerr << err << "\n";
    via_setexitdata(V, 1, std::format("User error: {}", err));
    V->abrt = true;
}

//* Register operations

// Sets register <R> to the given value <v>
template<typename T = viaValue>
    requires(!std::is_pointer_v<T>)
inline void via_setregister(viaState *V, Register R, T val) noexcept
{
    viaR_setregister(V->ralloc, R, val);
}

// Returns the value of register <R>
inline viaValue *via_getregister(viaState *V, Register R) noexcept
{
    return viaR_getregister(V->ralloc, R);
}

// Compares the value of two registers and returns wether if they are equivalent
inline constexpr bool via_cmpregister(viaState *V, Register R0, Register R1) noexcept
{
    // Early return if registers are equivalent
    if (via_getregister(V, R0) == via_getregister(V, R1))
        return true;

    viaValue v0 = *via_getregister(V, R0);
    viaValue v1 = *via_getregister(V, R1);

    // Early type mismatch check
    if (v0.type != v1.type)
        return false;

    // Common types first to improve branching efficiency
    switch (v0.type)
    {
    case ValueType::Number:
        return v0.val_number == v1.val_number;
    case ValueType::Bool:
        return v0.val_boolean == v1.val_boolean;
    case ValueType::String:
        if (v0.val_string->ptr && v1.val_string->ptr)
        {
            if (strlen(v0.val_string->ptr) != strlen(v1.val_string->ptr))
                return false;
            return !strcmp(v0.val_string->ptr, v1.val_string->ptr);
        }
        return v0.val_string->ptr == v1.val_string->ptr;
    case ValueType::Nil:
        return true; // Nil values are always equal
    case ValueType::Ptr:
        return v0.val_pointer == v1.val_pointer;
    default:
        return false; // Unique objects (CFunc, Func, viaTable, TableKey) are never equal
    }

    // If the type isn't matched, return false by default
    return false;
}

inline bool via_compare(viaState *, viaValue V0, viaValue V1)
{
    // Early return; if types aren't the same they cannot be equivalent
    if (V0.type != V1.type)
        return false;

    switch (V0.type)
    {
    case ValueType::Bool:
        return V0.val_boolean == V1.val_boolean;
    case ValueType::Number:
        return V0.val_number == V1.val_number;
    case ValueType::Nil:
        // Nil values are always equal
        return true;
    case ValueType::String:
        return !strcmp(V0.val_string->ptr, V1.val_string->ptr);
    case ValueType::Ptr:
        return V0.val_pointer == V1.val_pointer;
    case ValueType::Func:
        return V0.val_function == V1.val_function;
    case ValueType::CFunc:
        return V0.val_cfunction == V1.val_cfunction;
    default:
        // Unique objects (such as table) are never equal
        return false;
    }

    return false;
};

//* Stack/Global operations

// Soft-unwinds the stack to find variable with declared with <id>
// Starts searching from the first-most call frame
inline viaValue via_getvariable(viaState *V, VarId id)
{
    for (viaFunction *frame : *V->stack)
    {
        viaValue var = frame->locals[id];

        // No branch-hints, for a very good reason
        if (viaT_checknil(V, var))
            continue;

        return var;
    }

    return viaT_stackvalue(V);
}

// Loads the value of the variable declared with <id> into register <reg>
inline viaValue via_loadvariable(viaState *V, VarId id, Register reg)
{
    viaValue val = via_getvariable(V, id);
    via_setregister(V, reg, val);

    return val;
}

inline void via_setlocal(viaState *V, VarId id, viaValue val)
{
    viaFunction *frame = viaS_top(V->stack);
    frame->locals[id] = val;
}

inline void via_setvariable(viaState *V, VarId id, viaValue val)
{
    for (viaFunction *frame : *V->stack)
    {
        auto it = frame->locals.find(id);
        if (it == frame->locals.end())
            continue;

        frame->locals[id] = val;
        return; // Stop after setting the variable in the nearest frame
    }

    // If variable doesn't exist, we declare a new local
    via_setlocal(V, id, val);
}

// Similar to `via_getvariable` but explicitly looks for the variable in the global scope, a.k.a the root caller
template<typename T>
    requires std::same_as<T, VarId>
inline viaValue *via_getglobal(viaState *V, T id)
{
    viaFunction *global = *V->stack->sbp;
    auto it = global->locals.find(id);

    if (it == global->locals.end())
        return viaT_newvalue(V);

    return &it->second;
}

// Similar to `via_loadvariable` but explicitly looks for the variable in the global scope
inline viaValue *via_loadglobal(viaState *V, VarId id, Register reg)
{
    viaValue *val = via_getglobal(V, id);
    via_setregister(V, reg, *val);
    return val;
}

// Similar to `via_setvariable` but explicitly sets the variable in the global scope
inline void via_setglobal(viaState *V, VarId id, viaValue val)
{
    viaFunction *global = *V->stack->sbp;
    global->locals[id] = val;
}

//* viaValue operations

// Returns a value that contains a String that represents the stringified version of <v>
// ! The return value is guaranteed to be a String
inline viaValue &via_tostring(viaState *V, viaValue &val)
{
    // If the value has a String type but an invalid string value,
    // That it undefined behavior and should be explicitly handled by the end user.
    // It should NEVER occur under compiled bytecode
    if (viaT_checkstring(V, val))
        return val;

    switch (val.type)
    {
    case ValueType::Number:
    {
        viaString *str = viaT_newstring(V, std::to_string(val.val_number).c_str());
        val.val_string = str;
        break;
    }
    case ValueType::Bool:
    {
        viaString *str = viaT_newstring(V, val.val_boolean ? "true" : "false");
        val.val_string = str;
        break;
    }
    case ValueType::Table:
    {
        std::string str = "{";

        for (auto elem : val.val_table->data)
        {
            str += via_tostring(V, elem.second).val_string->ptr;
            str += ", ";
        }

        if (str.back() == ' ')
            str += "\b\b";

        str += "}";

        val.val_string = viaT_newstring(V, str.c_str());
        break;
    }
    case ValueType::Func:
    {
        const void *faddr = val.val_function;
        viaString *str = viaT_newstring(V, std::format("<function@{}>", faddr).c_str());
        val.val_string = str;
        break;
    }
    case ValueType::CFunc:
    {
        // This has to be explicitly casted because function pointers be weird
        const void *cfaddr = reinterpret_cast<const void *>(val.val_cfunction);
        viaString *str = viaT_newstring(V, std::format("<cfunction@{}>", cfaddr).c_str());
        val.val_string = str;
        break;
    }
    default:
        val.val_string = viaT_newstring(V, "nil");
        break;
    }

    val.type = ValueType::String;
    return val;
}

// Returns the truthiness of value <v>
// ! Guaranteed to be a Bool
inline viaValue &via_tobool(viaState *V, viaValue &val)
{
    if (viaT_checkbool(V, val))
        return val;

    switch (val.type)
    {
    case ValueType::Nil:
        val.val_boolean = false;
        break;
    default:
        val.val_boolean = true;
        break;
    }

    val.type = ValueType::Bool;
    return val;
}

// Returns the number representation of value <v>
// ! Returns Nil if impossible, unlike `vtostring` or `vtobool`
inline viaValue &via_tonumber(viaState *V, viaValue &val)
{
    if (viaT_checknumber(V, val))
        return val;

    switch (val.type)
    {
    case ValueType::String:
        val.val_number = std::stod(val.val_string->ptr);
        break;
    case ValueType::Bool:
        val.val_number = val.val_boolean ? 1.0f : 0.0f;
        break;
    default:
        return val;
    }

    val.type = ValueType::Number;
    return val;
}

// Utility function for quick table indexing
// Returns the value of key <k> if present in table <t>
inline viaValue *via_gettableindex(viaState *V, viaTable *T, TableKey key, bool search_meta)
{
    auto it = T->data.find(key);
    if (it != T->data.end())
        return &it->second;
    else if (search_meta && T->meta)
        // Disable metatable search to prevent chain searching
        // Which can cause infinite loops, crashes, and other bugs
        return via_gettableindex(V, T->meta, key, false);

    // This has to be a pointer because we don't know if the value is a non-pointer primitive type
    return viaT_newvalue(V);
}

// Assigns the given value <v> to key <k> in table <t>
inline void via_settableindex(viaState *V, viaTable *T, TableKey key, viaValue val)
{
    if (viaT_checknil(V, val))
    {
        if (via_gettableindex(V, T, key, false)->type != ValueType::Nil)
            T->data.erase(key);

        return;
    }

    T->data[key] = val;
}

// Pushes an argument
// Does not interact with FASTCALLs
inline void via_pushargument(viaState *V, viaValue arg)
{
    viaS_push(V->arguments, arg);
}

// Pops and returns a call argument
// Does not interact with FASTCALLs
inline viaValue via_popargument(viaState *V)
{
    // Check if the stack is empty
    // If so, return Nil
    if (V->arguments->size == 0)
        return viaT_stackvalue(V);

    // Clone the top value and pop it
    viaValue arg = viaS_top(V->arguments);
    viaS_pop(V->arguments);

    return arg;
}

// Pushes a return value
inline void via_pushreturn(viaState *V, viaValue ret)
{
    viaS_push(V->returns, ret);
}

// Pops a return value
inline viaValue via_popreturn(viaState *V)
{
    // Check if the stack is empty
    // If so, return Nil
    if (V->returns->size == 0)
        return viaT_stackvalue(V);

    // Clone the top value and pop it
    viaValue ret = viaS_top(V->returns);
    viaS_pop(V->returns);

    return ret;
}

// Utility function for quickly geting a metamethod from an opcode
inline viaValue *via_getmetamethod(viaState *V, viaValue val, OpCode op)
{
    if (!viaT_checktable(V, val))
        return viaT_newvalue(V);

    switch (op)
    {
    case OpCode::ADD:
    case OpCode::IADD:
        return via_gettableindex(V, val.val_table, viaT_hashstring(V, "__add"), true);
    case OpCode::SUB:
    case OpCode::ISUB:
        return via_gettableindex(V, val.val_table, viaT_hashstring(V, "__sub"), true);
    case OpCode::MUL:
    case OpCode::IMUL:
        return via_gettableindex(V, val.val_table, viaT_hashstring(V, "__mul"), true);
    case OpCode::DIV:
    case OpCode::IDIV:
        return via_gettableindex(V, val.val_table, viaT_hashstring(V, "__div"), true);
    case OpCode::MOD:
    case OpCode::IMOD:
        return via_gettableindex(V, val.val_table, viaT_hashstring(V, "__mod"), true);
    default:
        break;
    }

    return viaT_newvalue(V);
}

// Calls a Func type, terminates if uncallable
inline void via_callf(viaState *V, viaFunction *f, bool save_state)
{
    if (save_state)
    {
        Instruction *begin = f->bytecode.data();

        // Save VM state to be restored when the function stops executing
        V->sstate = new viaState(*V);
        V->ihp = begin;
        V->ibp = begin + f->bytecode.size() - 1;
        V->ip = V->ihp;
    }

    /*
     * Yes, we can safely do this after saving the VM state
     * This is because the stack is a pointer and not an object
     * Which means that it cannot be mutated with context switching
     */
    viaS_push(V->stack, f);
    viaS_flush(V->returns);
}

// Calls a C function pointer
inline void via_callc(viaState *V, viaCFunction *cf)
{
    // 15 additional characters:
    // - 9 for "cfunction"
    // - 1 for space
    // - 2 for "0x"
    // - 2 for < and >
    // - 1 for null terminator
    char buffer[2 * sizeof(void *) + 15];
    // Yes, this is safe because cf is a const reference
    const void *addr = &cf;

    std::snprintf(buffer, sizeof(buffer), "<cfunction@0x%p>", addr);

    viaFunction freplica = viaFunction{
        0,
        cf->error_handler,
        false,
        buffer,
        *V->stack->begin(),
        {},
        {},
    };

    // We can safely reference the functions address because it will be cleaned up after
    // the C function stops executing, therefore it won't dangle
    viaS_push(V->stack, &freplica);
    viaS_flush(V->returns);

    // Call function pointer
    cf->ptr(V);

    viaS_pop(V->stack);
    viaS_flush(V->arguments);
}

// Returns the primitive type of value <v>
inline viaValue via_type(viaState *V, viaValue v)
{
    auto enum_name = ENUM_NAME(v.type);
    std::string stdstr = std::string(enum_name);
    const char *str = strdup(stdstr.c_str());
    return viaT_stackvalue(V, viaT_newstring(V, str));
}

/*
 * Unified call interface
 * Calls both viaFunction and viaCFunction values
 * Uses a stack-based calling convention
 */
inline void via_call(viaState *V, viaValue val)
{
    V->calltype = viaCallType::CALL;
    // V->argc = static_cast<CallArgc>(V->arguments->size);

    if (viaT_checkfunction(V, val))
        via_callf(V, val.val_function, true);
    else if (viaT_checkcfunction(V, val))
        via_callc(V, val.val_cfunction);
    else if (viaT_checktable(V, val))
    {
        TableKey mhash = viaT_hashstring(V, "__call");
        viaValue *mmcall = via_gettableindex(V, val.val_table, mhash, true);

        // Push self argument
        via_pushargument(V, *mmcall);
        via_call(V, *mmcall);
    }
    else
    {
        viaValue callt = via_type(V, val);
        via_setexitdata(V, 1, std::format("Attempt to call a {} value", callt.val_string->ptr));
        V->abrt = true;
    }
}

inline void via_fastcall1(viaState *V, viaValue, Register)
{
    V->calltype = viaCallType::FASTCALL;
    V->argc = 1;
}

// Returns the length of value <v>, nil if impossible
inline viaValue via_len(viaState *V, viaValue val)
{
    if (viaT_checkstring(V, val))
        return viaT_stackvalue(V, static_cast<viaNumber>(strlen(val.val_string->ptr)));
    else if (viaT_checktable(V, val))
    {
        TableKey mhash = viaT_hashstring(V, "__len");
        viaValue *mmlen = via_gettableindex(V, val.val_table, mhash, true);

        if (viaT_checknil(V, *mmlen))
            return viaT_stackvalue(V, static_cast<viaNumber>(val.val_table->data.size()));

        // Push self argument
        via_pushargument(V, val);
        via_call(V, *mmlen);

        return via_popreturn(V);
    }

    return viaT_stackvalue(V);
}

// Loads the value of key <k> in table <t> into register <R>, if present in table
inline viaValue *via_loadtableindex(viaState *V, viaTable *T, TableKey key, Register R)
{
    viaValue *val = via_gettableindex(V, T, key, true);
    via_setregister(V, R, *val);
    return val;
}

// Returns the complex type of value <v>
// Practically the same as `vtype()`, but returns
// the `__type` value if the given table has one
inline viaValue via_typeof(viaState *V, viaValue val)
{
    if (viaT_checktable(V, val))
    {
        viaTable *tbl = val.val_table;
        viaValue *ty = via_gettableindex(V, tbl, viaT_hashstring(V, "__type"), true);

        if (viaT_checknil(V, *ty))
            return via_type(V, val);

        viaString *tystr = viaT_newstring(V, ty->val_string->ptr);

        return viaT_stackvalue(V, tystr);
    }

    return via_type(V, val);
}

// Calls the value of key <k> in table <t>, if callable
// Uses the `call` method internally
inline void via_callmethod(viaState *V, viaTable *T, TableKey key)
{
    viaValue *at = via_gettableindex(V, T, key, true);
    // ! This doesn't need to be a pointer because the underlying table type is already a pointer
    viaValue self = viaT_stackvalue(V, T);

    via_pushargument(V, self);
    via_call(V, *at);
}

// Returns wether if table <t> is frozen
inline bool via_isfrozen(viaState *, viaTable *T)
{
    return T->frozen.get();
}

// Freezes table <t>
// Terminates the VM if table <t> is already frozen
inline void via_freeze(viaState *V, viaTable *T)
{
    if (via_isfrozen(V, T))
    {
        via_setexitdata(V, 1, "Attempt to freeze already-frozen table");
        V->abrt = true;
        return;
    }

    T->frozen.set(true);
}

// Sets the metatable of <t> to <meta>
inline void via_setmetatable(viaState *V, viaTable *T, viaTable *meta)
{
    if (via_isfrozen(V, T))
    {
        via_setexitdata(V, 1, "Cannot set metatable of frozen table");
        V->abrt = true;
        return;
    }

    T->meta = meta;
}

// Returns the metatable of <t>, nil if it doesn't have one
inline viaValue via_getmetatable(viaState *V, viaTable *T)
{
    if (T->meta)
        // We don't need to return a value pointer as the underlying table value is already a pointer
        return viaT_stackvalue(V, T->meta);

    return viaT_stackvalue(V);
}

//* Utility
inline viaValue via_toviavalue(viaState *V, const Operand &operand)
{
    switch (operand.type)
    {
    case OperandType::Number:
        return viaT_stackvalue(V, operand.val_number);
    case OperandType::Bool:
        return viaT_stackvalue(V, operand.val_boolean);
    case OperandType::String:
        return viaT_stackvalue(V, viaT_newstring(V, operand.val_string));
    case via::OperandType::Nil:
        return viaT_stackvalue(V);
    default:
        via_setexitdata(V, 1, std::format("Cannot interpret operand '{}' as a data type", ENUM_NAME(operand.type)));
        V->abrt = true;
        break;
    }

    return viaT_stackvalue(V);
}

// Loads a static library to the global environment
// If called during runtime, it will terminate the VM
inline void via_loadlib(viaState *V, VarId id, viaValue lib) noexcept
{
    if (V->tstate == viaThreadState::RUNNING)
    {
        via_setexitdata(V, 1, "Attempt to load library during runtime");
        V->abrt = true;
        return;
    }

    if (via_getglobal(V, id)->type != ValueType::Nil)
    {
        via_setexitdata(V, 1, std::format("Attempt to load library '{}' twice", id));
        V->abrt = true;
        return;
    }

    via_setglobal(V, id, lib);
}

inline void via_yield(viaState *V, float ms)
{
    if (V->tstate == viaThreadState::RUNNING)
        V->yield = ms;
}

inline void via_savestate(viaState *V)
{
    V->sstate = new viaState(*V);
}

inline void via_restorestate(viaState *V)
{
    *V = *V->sstate;
    V->sstate = nullptr;
}

inline viaValue via_arith(viaState *V, viaValue lhs, viaValue rhs, OpCode op)
{
#ifdef VIA_DEBUG
    VIA_ASSERT(viaT_checknumber(V, rhs), "via_arith(): Expected Number for rhs");
    VIA_ASSERT(rhs.val_number != 0.0f, "via_arith(): Expected non-zero Number for rhs");
#endif
    if (viaT_checknumber(V, lhs))
    {
        switch (op)
        {
        case OpCode::ADD:
            return viaT_stackvalue(V, lhs.val_number + rhs.val_number);
        case OpCode::SUB:
            return viaT_stackvalue(V, lhs.val_number - rhs.val_number);
        case OpCode::MUL:
            return viaT_stackvalue(V, lhs.val_number * rhs.val_number);
        case OpCode::DIV:
            return viaT_stackvalue(V, lhs.val_number / rhs.val_number);
        case OpCode::POW:
            return viaT_stackvalue(V, std::pow(lhs.val_number, rhs.val_number));
        case OpCode::MOD:
            return viaT_stackvalue(V, std::fmod(lhs.val_number, rhs.val_number));
        default:
            break;
        }

        return viaT_stackvalue(V);
    }
    else if (viaT_checktable(V, lhs))
    {
        viaValue *mm = via_getmetamethod(V, lhs, op);

        via_pushargument(V, lhs);
        via_pushargument(V, rhs);
        via_call(V, *mm);

        return via_popreturn(V);
    }

    via_setexitdata(V, 1, "Invalid arithmetic operator");
    V->abrt = true;
    return viaT_stackvalue(V);
}

#if defined(__GNUC__) || defined(__clang__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wunused-parameter"
#elif defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 4100)
#endif
inline void via_iarith(viaState *V, viaValue *lhs, viaValue rhs, OpCode op)
{
#ifdef VIA_DEBUG
    VIA_ASSERT(viaT_checknumber(V, rhs), "via_arith(): Expected Number for rhs");
    VIA_ASSERT(rhs.val_number != 0.0f, "via_arith(): Expected non-zero Number for rhs");
#endif
    switch (op)
    {
    case OpCode::IADD:
        lhs->val_number += rhs.val_number;
        break;
    case OpCode::ISUB:
        lhs->val_number -= rhs.val_number;
        break;
    case OpCode::IMUL:
        lhs->val_number *= rhs.val_number;
        break;
    case OpCode::IDIV:
        lhs->val_number /= rhs.val_number;
        break;
    case OpCode::IPOW:
        lhs->val_number = std::pow(lhs->val_number, rhs.val_number);
        break;
    case OpCode::IMOD:
        lhs->val_number = std::fmod(lhs->val_number, rhs.val_number);
        break;
    default:
        break;
    }
}
#if defined(__GNUC__) || defined(__clang__)
#    pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#    pragma warning(pop)
#endif

} // namespace via