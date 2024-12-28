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

// Manually sets the VM exit data.
inline void setexitdata(RTState *V, ExitCode exitc, const std::string &exitm) noexcept
{
    V->exitc = exitc;
    V->exitm = dupstring(exitm);
}

// Returns whether if <addr> is within the bounds of the instruction pipeline.
inline bool isvalidjmpaddr(RTState *V, const Instruction *addr) noexcept
{
    return (addr >= V->ihp) && (addr <= V->ibp);
}

// Sets register <R> to the given value <v>.
// Wrapper for `rsetregister`.
template<typename T = TValue>
    requires(!std::is_pointer_v<T>)
inline void setregister(RTState *V, GPRegister R, T val) noexcept
{
    rsetregister(V->ralloc, R, val);
}

// Returns the value of register <R>.
// Wrapper for `rgetregister`.
inline TValue *getregister(RTState *V, GPRegister R) noexcept
{
    return rgetregister(V->ralloc, R);
}

// Compares the values of two registers and returns whether if they are equivalent.
inline bool cmpregister(RTState *V, GPRegister R0, GPRegister R1) noexcept
{
    // Early return if registers are equivalent
    if (getregister(V, R0) == getregister(V, R1))
        return true;

    TValue v0 = *getregister(V, R0);
    TValue v1 = *getregister(V, R1);

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
            if (v0.val_string->len != v1.val_string->len)
                return false;
            return !std::strcmp(v0.val_string->ptr, v1.val_string->ptr);
        }
        return v0.val_string->ptr == v1.val_string->ptr;
    case ValueType::Nil:
        return true; // Nil values are always equal
    case ValueType::Ptr:
        return v0.val_pointer == v1.val_pointer;
    default:
        return false; // Unique objects (CFunc, Func, Table, etc.) are never equal
    }

    // If the type isn't matched, return false by default
    return false;
}

// Compares 2 values and returns whether if they are equal.
inline bool compare(RTState *, TValue V0, TValue V1)
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
        return !std::strcmp(V0.val_string->ptr, V1.val_string->ptr);
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

// Soft-unwinds the stack to find variable with declared with <id>.
// Starts searching from the top-most caller stack-frame.
inline TValue getvariable(RTState *V, VarId id)
{
    for (TFunction *frame : *V->stack)
    {
        TValue var = frame->locals[id];

        // No branching-hints, for a very good reason
        if (checknil(V, var))
            continue;

        return var;
    }

    return stackvalue(V);
}

// Loads the value of the variable declared with <id> into register <reg>.
inline TValue loadvariable(RTState *V, VarId id, GPRegister reg)
{
    TValue val = getvariable(V, id);
    setregister(V, reg, val);
    return val;
}

// Declares (or reassigns) a local variable with identifier <id> and value <val>.
inline void setlocal(RTState *V, VarId id, TValue val)
{
    TFunction *frame = tstop(V->stack);
    frame->locals[id] = val;
}

// Declares (or reassigns) a variable with identifier <id> and value <val>.
// Soft unwinds the stack to find the variable, if not found,
// declares a new local variable with the same parameters.
inline void setvariable(RTState *V, VarId id, TValue val)
{
    for (TFunction *frame : *V->stack)
    {
        auto it = frame->locals.find(id);
        if (it == frame->locals.end())
            continue;

        frame->locals[id] = val;
        return; // Stop after setting the variable in the nearest frame
    }

    // If variable doesn't exist, we declare a new local
    setlocal(V, id, val);
}

// Similar to `getvariable` but explicitly looks for the variable in the global scope, a.k.a the root caller stack-frame.
template<typename T>
    requires std::same_as<T, VarId>
inline TValue *getglobal(RTState *V, T id)
{
    TFunction *global = *V->stack->sbp;

    auto it = global->locals.find(id);
    if (it == global->locals.end())
        return newvalue(V);

    return &it->second;
}

// Similar to `loadvariable` but explicitly looks for the variable in the global scope.
inline TValue *loadglobal(RTState *V, VarId id, GPRegister reg)
{
    TValue *val = getglobal(V, id);
    setregister(V, reg, *val);
    return val;
}

// Similar to `setvariable` but explicitly sets the variable in the global scope.
inline void setglobal(RTState *V, VarId id, TValue val)
{
    // Retrieve stack-base-pointer
    TFunction *global = *V->stack->sbp;
    global->locals[id] = val;
}

//* TValue operations

// Returns a value that contains a String that represents the stringified version of <v>.
// The return value is guaranteed to be a String type.
inline TValue &tostring(RTState *V, TValue &val)
{
    // If the value has a String type but an invalid string value,
    // that is classified as undefined behavior and should be explicitly handled by the end user.
    // It is guaranteed to NEVER occur under compiled bytecode
    if (checkstring(V, val))
        return val;

    switch (val.type)
    {
    case ValueType::Number:
    {
        TString *str = newstring(V, std::to_string(val.val_number).c_str());
        val.val_string = str;
        break;
    }
    case ValueType::Bool:
    {
        TString *str = newstring(V, val.val_boolean ? "true" : "false");
        val.val_string = str;
        break;
    }
    case ValueType::Table:
    {
        std::string str = "{";

        for (auto elem : val.val_table->data)
        {
            str += tostring(V, elem.second).val_string->ptr;
            str += ", ";
        }

        if (str.back() == ' ')
            str += "\b\b";

        str += "}";

        val.val_string = newstring(V, str.c_str());
        break;
    }
    case ValueType::Func:
    {
        const void *faddr = val.val_function;
        TString *str = newstring(V, std::format("<function@{}>", faddr).c_str());
        val.val_string = str;
        break;
    }
    case ValueType::CFunc:
    {
        // This has to be explicitly casted because function pointers be weird
        const void *cfaddr = reinterpret_cast<const void *>(val.val_cfunction);
        TString *str = newstring(V, std::format("<cfunction@{}>", cfaddr).c_str());
        val.val_string = str;
        break;
    }
    default:
        val.val_string = newstring(V, "nil");
        break;
    }

    val.type = ValueType::String;
    return val;
}

// Returns the truthiness of value <v>.
// Guaranteed to be a Bool type.
inline TValue &tobool(RTState *V, TValue &val)
{
    if (checkbool(V, val))
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

// Returns the number representation of value <v>.
// Returns Nil if impossible, unlike `vtostring` or `vtobool`.
inline TValue &tonumber(RTState *V, TValue &val)
{
    if (checknumber(V, val))
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

// Utility function for quick table indexing.
// Returns the value of key <k> if present in table <t>.
inline TValue *gettableindex(RTState *V, TTable *T, TableKey key, bool search_meta)
{
    auto it = T->data.find(key);
    if (it != T->data.end())
        return &it->second;
    else if (search_meta && T->meta)
        // Disable metatable search to prevent chain searching
        // Which can cause infinite loops, crashes, and other bugs
        return gettableindex(V, T->meta, key, false);

    // This has to be a pointer because we don't know if the value is a non-pointer primitive type
    return newvalue(V);
}

// Assigns the given value <v> to key <k> in table <t>.
inline void settableindex(RTState *V, TTable *T, TableKey key, TValue val)
{
    if (checknil(V, val))
    {
        if (gettableindex(V, T, key, false)->type != ValueType::Nil)
            T->data.erase(key);

        return;
    }

    T->data[key] = val;
}

// Pushes an argument.
inline void pushargument(RTState *V, TValue arg)
{
    tspush(V->arguments, arg);
}

// Pops and returns a call argument.
inline TValue popargument(RTState *V)
{
    // Check if the stack is empty
    // If so, return Nil
    if (V->arguments->size == 0)
        return stackvalue(V);

    // Clone the top value and pop it
    TValue arg = tstop(V->arguments);
    tspop(V->arguments);

    return arg;
}

// Pushes a return value.
inline void pushreturn(RTState *V, TValue ret)
{
    tspush(V->returns, ret);
}

// Pops a return value.
inline TValue popreturn(RTState *V)
{
    // Check if the stack is empty
    // If so, return Nil
    if (V->returns->size == 0)
        return stackvalue(V);

    // Clone the top value and pop it
    TValue ret = tstop(V->returns);
    tspop(V->returns);

    return ret;
}

// Utility function for quickly geting a metamethod associated to <op>.
inline TValue *getmetamethod(RTState *V, TValue val, OpCode op)
{
    if (!checktable(V, val))
        return newvalue(V);

    switch (op)
    {
    case OpCode::ADD:
    case OpCode::IADD:
        return gettableindex(V, val.val_table, hashstring(V, "__add"), true);
    case OpCode::SUB:
    case OpCode::ISUB:
        return gettableindex(V, val.val_table, hashstring(V, "__sub"), true);
    case OpCode::MUL:
    case OpCode::IMUL:
        return gettableindex(V, val.val_table, hashstring(V, "__mul"), true);
    case OpCode::DIV:
    case OpCode::IDIV:
        return gettableindex(V, val.val_table, hashstring(V, "__div"), true);
    case OpCode::MOD:
    case OpCode::IMOD:
        return gettableindex(V, val.val_table, hashstring(V, "__mod"), true);
    default:
        break;
    }

    return newvalue(V);
}

// Calls a native function.
inline void callf(RTState *V, TFunction *f, bool save_state)
{
    if (save_state)
    {
        Instruction *begin = f->bytecode.data();

        // Save VM state to be restored when the function stops executing
        V->sstate = new RTState(*V);
        V->ihp = begin;
        V->ibp = begin + f->bytecode.size() - 1;
        V->ip = V->ihp;
    }

    /*
     * Yes, we can safely do this after saving the VM state
     * This is because the stack is a pointer and not an object
     * Which means that it cannot be mutated with context switching
     */
    tspush(V->stack, f);
    tsflush(V->returns);
}

// Calls a C function pointer.
// Mimics stack behavior as it would behave while calling a native function.
inline void callc(RTState *V, TCFunction *cf)
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

    TFunction freplica{
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
    tspush(V->stack, &freplica);
    tsflush(V->returns);

    // Call function pointer
    cf->ptr(V);

    tspop(V->stack);
    tsflush(V->arguments);
}

// Returns the primitive type of value <v>.
inline TValue type(RTState *V, TValue v)
{
    auto enum_name = ENUM_NAME(v.type);
    std::string stdstr = std::string(enum_name);
    const char *str = dupstring(stdstr);
    return stackvalue(V, newstring(V, str));
}

// Unified call interface.
// Works on all callable types (TFunction, TCFunction, TTable).
inline void call(RTState *V, TValue val)
{
    V->calltype = CallType::CALL;
    // V->argc = static_cast<CallArgc>(V->arguments->size);

    if (checkfunction(V, val))
        callf(V, val.val_function, true);
    else if (checkcfunction(V, val))
        callc(V, val.val_cfunction);
    else if (checktable(V, val))
    {
        TableKey mhash = hashstring(V, "__call");
        TValue *mmcall = gettableindex(V, val.val_table, mhash, true);

        // Push self argument
        pushargument(V, *mmcall);
        call(V, *mmcall);
    }
    else
    {
        TValue callt = type(V, val);
        setexitdata(V, 1, std::format("Attempt to call a {} value", callt.val_string->ptr));
        V->abrt = true;
    }
}

// Calls <val> with the FASTCALL1 (argument count is 1, argument 1 located in <arg0>) calling convention.
inline void fastcall1(RTState *V, TValue, GPRegister)
{
    V->calltype = CallType::FASTCALL;
    V->argc = 1;
}

// Returns the length of value <v>, nil if impossible.
inline TValue len(RTState *V, TValue val)
{
    if (checkstring(V, val))
        return stackvalue(V, static_cast<TNumber>(strlen(val.val_string->ptr)));
    else if (checktable(V, val))
    {
        TableKey mhash = hashstring(V, "__len");
        TValue *mmlen = gettableindex(V, val.val_table, mhash, true);

        if (checknil(V, *mmlen))
            return stackvalue(V, static_cast<TNumber>(val.val_table->data.size()));

        // Push self argument
        pushargument(V, val);
        call(V, *mmlen);

        return popreturn(V);
    }

    return stackvalue(V);
}

// Loads the value of key <k> in table <t> into register <R>, if present in table.
inline TValue *loadtableindex(RTState *V, TTable *T, TableKey key, GPRegister R)
{
    TValue *val = gettableindex(V, T, key, true);
    setregister(V, R, *val);
    return val;
}

// Returns the complex type of value <v>.
// Practically the same as `type()`, but returns
// the `__type` key if the given table has it defined.
inline TValue typeof(RTState * V, TValue val)
{
    if (checktable(V, val))
    {
        TTable *tbl = val.val_table;
        TValue *ty = gettableindex(V, tbl, hashstring(V, "__type"), true);

        if (checknil(V, *ty))
            return type(V, val);

        TString *tystr = newstring(V, ty->val_string->ptr);

        return stackvalue(V, tystr);
    }

    return type(V, val);
}

// Returns wether if table <t> is frozen.
inline bool isfrozen(RTState *, TTable *T)
{
    return T->frozen.get();
}

// Freezes (locks, const-ifies) table <t>.
inline void freeze(RTState *, TTable *T)
{
    T->frozen.set(true);
}

// Sets the metatable of <t> to <meta>.
inline void setmetatable(RTState *V, TTable *T, TTable *meta)
{
    if (isfrozen(V, T))
    {
        setexitdata(V, 1, "Cannot set metatable of frozen table");
        V->abrt = true;
        return;
    }

    T->meta = meta;
}

// Returns the metatable of <t>, nil if impossible.
inline TValue getmetatable(RTState *V, TTable *T)
{
    if (T->meta)
        // We don't need to return a value pointer as the underlying table value is already a pointer
        return stackvalue(V, T->meta);

    return stackvalue(V);
}

// Converts an Operand objecti into a TValue object.
inline TValue toviavalue(RTState *V, const Operand &operand)
{
    switch (operand.type)
    {
    case OperandType::Number:
        return stackvalue(V, operand.val_number);
    case OperandType::Bool:
        return stackvalue(V, operand.val_boolean);
    case OperandType::String:
        return stackvalue(V, newstring(V, operand.val_string));
    case via::OperandType::Nil:
        return stackvalue(V);
    default:
        setexitdata(V, 1, std::format("Cannot interpret operand '{}' as a data type", ENUM_NAME(operand.type)));
        V->abrt = true;
        break;
    }

    return stackvalue(V);
}

// Loads a static library to the global environment.
// Cannot be called during runtime.
inline void loadlib(RTState *V, VarId id, TValue lib) noexcept
{
    if (V->tstate == ThreadState::RUNNING)
    {
        setexitdata(V, 1, "Attempt to load library during runtime");
        V->abrt = true;
        return;
    }

    if (getglobal(V, id)->type != ValueType::Nil)
    {
        setexitdata(V, 1, std::format("Attempt to load library '{}' twice", id));
        V->abrt = true;
        return;
    }

    setglobal(V, id, lib);
}

// Schedules a yield <ms>.
// Only yields the VM thread.
inline void yield(RTState *V, float ms)
{
    if (V->tstate == ThreadState::RUNNING)
        V->yield = ms;
}

// Saves the state by copying it and storing it.
inline void savestate(RTState *V)
{
    V->sstate = new RTState(*V);
}

// Restores the saved state by overwriting the state pointer.
// Resets the saved state pointer.
inline void restorestate(RTState *V)
{
    *V = *V->sstate;
    V->sstate = nullptr;
}

// Performs an arithmetic operation determined by <op> between <lhs> and <rhs>.
inline TValue arith(RTState *V, TValue lhs, TValue rhs, OpCode op)
{
#ifdef VIA_DEBUG
    VIA_ASSERT(checknumber(V, rhs), "arith(): Expected Number for rhs");
#endif
    if (checknumber(V, lhs))
    {
        switch (op)
        {
        case OpCode::ADD:
            return stackvalue(V, lhs.val_number + rhs.val_number);
        case OpCode::SUB:
            return stackvalue(V, lhs.val_number - rhs.val_number);
        case OpCode::MUL:
            return stackvalue(V, lhs.val_number * rhs.val_number);
        case OpCode::DIV:
            return stackvalue(V, lhs.val_number / rhs.val_number);
        case OpCode::POW:
            return stackvalue(V, std::pow(lhs.val_number, rhs.val_number));
        case OpCode::MOD:
            return stackvalue(V, std::fmod(lhs.val_number, rhs.val_number));
        default:
            break;
        }

        return stackvalue(V);
    }
    else if (checktable(V, lhs))
    {
        TValue *mm = getmetamethod(V, lhs, op);
        pushargument(V, lhs);
        pushargument(V, rhs);
        call(V, *mm);
        return popreturn(V);
    }

    setexitdata(V, 1, "Invalid arithmetic operator");
    V->abrt = true;
    return stackvalue(V);
}

// Work around for the state object being unused
// when `VIA_DEBUG` is not defined
#if defined(__GNUC__) || defined(__clang__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wunused-parameter"
#elif defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 4100)
#endif
// Performs inline artihmetic between <lhs*> and <rhs>, operation determined by <op>.
inline void iarith(RTState *V, TValue *lhs, TValue rhs, OpCode op)
{
#ifdef VIA_DEBUG
    VIA_ASSERT(checknumber(V, rhs), "arith(): Expected Number for rhs");
    VIA_ASSERT(rhs.val_number != 0.0f, "arith(): Expected non-zero Number for rhs");
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