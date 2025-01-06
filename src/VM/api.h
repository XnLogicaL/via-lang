/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "bytecode.h"
#include "gc.h"
#include "instruction.h"
#include "opcode.h"
#include "register.h"
#include "shared.h"
#include "stack.h"
#include "state.h"
#include "types.h"

namespace via
{

// Manually sets the VM exit data.
VIA_FORCEINLINE void setexitdata(RTState *VIA_RESTRICT V, ExitCode exitc, const std::string &exitm) noexcept
{
    V->exitc = exitc;
    V->exitm = dupstring(exitm);
}

// Returns whether if <addr> is within the bounds of the instruction pipeline.
VIA_FORCEINLINE bool isvalidjmpaddr(RTState *VIA_RESTRICT V, Instruction *VIA_RESTRICT const addr) noexcept
{
    return (addr >= V->ihp) && (addr <= V->ibp);
}

// Sets register <reg> to the given value <val>.
// Wrapper for `rsetregister`.
template<typename T = TValue>
    requires(!std::is_pointer_v<T> && std::same_as<T, TValue>)
VIA_FORCEINLINE void setregister(RTState *VIA_RESTRICT V, GPRegister reg, T val) noexcept
{
    rsetregister(V->ralloc, reg, val);
}

// Returns the value of register <reg>.
// Wrapper for `rgetregister`.
VIA_FORCEINLINE TValue *getregister(RTState *VIA_RESTRICT V, GPRegister reg) noexcept
{
    return rgetregister(V->ralloc, reg);
}

// Compares the values of two registers and returns whether if they are equivalent.
VIA_FORCEINLINE bool cmpregister(RTState *VIA_RESTRICT V, GPRegister reg0, GPRegister reg1) noexcept
{
    // Early return if registers are equivalent
    if (getregister(V, reg0) == getregister(V, reg1))
        return true;

    TValue v0 = *getregister(V, reg0);
    TValue v1 = *getregister(V, reg1);

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
// Has early sanity-checks to ensure maximum performance.
VIA_FORCEINLINE bool compare(RTState *VIA_RESTRICT, TValue V0, TValue V1) noexcept
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

// Pushes a value onto the stack. Does not tag the pointer.
// Wrapper for tspush. Copies the value.
VIA_FORCEINLINE void pushval(RTState *VIA_RESTRICT V, TValue val)
{
    TValue *cpy = new TValue(val);
    uintptr_t ptr = reinterpret_cast<uintptr_t>(cpy);
    tspush(V->stack, ptr);
}

// Pops a value from the stack. Returns it as a TValue *.
// Wrapper for tspop. Does not clear pointer tags.
VIA_FORCEINLINE TValue *popval(RTState *VIA_RESTRICT V)
{
    uintptr_t ptr = tspop(V->stack);
    return reinterpret_cast<TValue *>(ptr);
}

// Returns a value that contains a String that represents the stringified version of <val>.
// The return value is guaranteed to be a String type.
VIA_FORCEINLINE TValue &tostring(RTState *VIA_RESTRICT V, TValue &val) noexcept
{
    // If the value union is tagged as a String type but an invalid string value,
    // that is classified as undefined behavior and should be explicitly handled by the end user.
    // It is guaranteed to NEVER occur under compiled bytecode
    if (checkstring(V, val))
        return val;

    switch (val.type)
    {
    case ValueType::Number:
    {
        std::string str = std::to_string(val.val_number);
        TString *tstr = newstring(V, str.c_str());
        val.val_string = tstr;
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

// Returns the truthiness of value <val>.
// Guaranteed to be a Bool type.
VIA_FORCEINLINE TValue &tobool(RTState *VIA_RESTRICT V, TValue &val) noexcept
{
    // If the TValue union is tagged as Bool type but it
    // doesn't actually contain a boolean value, that is undefined behavior
    // and is the responsibility of the end-user.
    if (checkbool(V, val))
        return val;

    switch (val.type)
    {
    // Nil and Monostate is the only falsy type
    case ValueType::Nil:
    case ValueType::Monostate:
        val.val_boolean = false;
        break;
    default:
        val.val_boolean = true;
        break;
    }

    val.type = ValueType::Bool;
    return val;
}

// Returns the number representation of value <val>.
// Returns Nil if impossible, unlike `vtostring` or `vtobool`.
VIA_FORCEINLINE TValue &tonumber(RTState *VIA_RESTRICT V, TValue &val) noexcept
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
// Returns the value of key <key> if present in table <tbl>.
VIA_FORCEINLINE TValue *gettableindex(RTState *VIA_RESTRICT V, TTable *VIA_RESTRICT tbl, TableKey key, const bool search_meta)
{
    auto it = tbl->data.find(key);
    if (it != tbl->data.end())
        return &it->second;
    else if (search_meta && tbl->meta)
        // Disable metatable search to prevent chain searching
        // Which can cause infinite loops, crashes, and other bugs
        return gettableindex(V, tbl->meta, key, false);

    // This has to be a pointer because we don't know if the value is a non-pointer primitive type
    return newvalue(V);
}

// Assigns the given value <val> to key <key> in table <tbl>.
VIA_FORCEINLINE void settableindex(RTState *VIA_RESTRICT V, TTable *VIA_RESTRICT tbl, TableKey key, TValue val) noexcept
{
    if (checknil(V, val))
    {
        if (gettableindex(V, tbl, key, false)->type != ValueType::Nil)
            tbl->data.erase(key);

        return;
    }

    tbl->data[key] = val;
}

// Utility function for quickly geting a metamethod associated to <op>.
VIA_FORCEINLINE TValue *getmetamethod(RTState *VIA_RESTRICT V, TValue val, OpCode op) noexcept
{
    if (!checktable(V, val))
        return newvalue(V);

    switch (op)
    {
    case OpCode::ADDRR:
    case OpCode::ADDRN:
    case OpCode::ADDNR:
    case OpCode::ADDNN:
    case OpCode::ADDIR:
    case OpCode::ADDIN:
        return gettableindex(V, val.val_table, hashstring(V, "__add"), true);
    case OpCode::SUBRR:
    case OpCode::SUBRN:
    case OpCode::SUBNR:
    case OpCode::SUBNN:
    case OpCode::SUBIR:
    case OpCode::SUBIN:
        return gettableindex(V, val.val_table, hashstring(V, "__sub"), true);
    case OpCode::MULRR:
    case OpCode::MULRN:
    case OpCode::MULNR:
    case OpCode::MULNN:
    case OpCode::MULIR:
    case OpCode::MULIN:
        return gettableindex(V, val.val_table, hashstring(V, "__mul"), true);
    case OpCode::DIVRR:
    case OpCode::DIVRN:
    case OpCode::DIVNR:
    case OpCode::DIVNN:
    case OpCode::DIVIR:
    case OpCode::DIVIN:
        return gettableindex(V, val.val_table, hashstring(V, "__div"), true);
    case OpCode::POWRR:
    case OpCode::POWRN:
    case OpCode::POWNR:
    case OpCode::POWNN:
    case OpCode::POWIR:
    case OpCode::POWIN:
        return gettableindex(V, val.val_table, hashstring(V, "__pow"), true);
    case OpCode::MODRR:
    case OpCode::MODRN:
    case OpCode::MODNR:
    case OpCode::MODNN:
    case OpCode::MODIR:
    case OpCode::MODIN:
        return gettableindex(V, val.val_table, hashstring(V, "__mod"), true);
    default:
        break;
    }

    return newvalue(V);
}

VIA_FORCEINLINE TValue *getargument(RTState *VIA_RESTRICT V, uint32_t offset)
{
    // Check if the argument is out of bounds, return nil if so
    if (offset >= V->argc)
        return newvalue(V);

    // Calculate the stack position of the argument
    size_t stack_address = V->ssp + V->argc - 1 - offset;
    // Retrieve stack value
    uintptr_t ptr = V->stack->sbp[stack_address];

    return reinterpret_cast<TValue *>(ptr);
}

VIA_FORCEINLINE void nativeret(RTState *VIA_RESTRICT V, CallArgc retc) noexcept
{
    std::vector<TValue *> ret_values;
    // Restore state
    V->ip = V->frame->ret_addr;
    V->frame = V->frame->caller;

    // Save return values
    for (CallArgc i = 0; i < retc; i++)
    {
        TValue *ret_val = popval(V);
        ret_values.push_back(ret_val);
    }

    // Restore stack pointer
    V->stack->sp = V->ssp;

    // Clean up arguments
    for (CallArgc i = 0; i < V->argc; i++)
        popval(V);

    // Restore return values
    for (int i = retc - 1; i >= 0; i--) // Reverse order for pushing return values
        pushval(V, *ret_values[i]);
}

// Calls a native function.
VIA_FORCEINLINE void nativecall(RTState *VIA_RESTRICT V, TFunction *VIA_RESTRICT callee, CallArgc argc) noexcept
{
    // Save state
    callee->caller = V->frame;
    callee->ret_addr = V->ip;

    // Setup call information
    V->frame = callee;
    V->ip = callee->bytecode.data();
    V->argc = argc;
    V->ssp = V->stack->sp;
}

// Calls a C function pointer.
// Mimics stack behavior as it would behave while calling a native function.
VIA_FORCEINLINE void externcall(RTState *VIA_RESTRICT V, TCFunction *VIA_RESTRICT cf, CallArgc argc) noexcept
{
    /* Stack allocate id string
     * 15 additional characters:
     * +9 for 'cfunction'
     * +1 for '@'
     * +2 for '0x'
     * +2 for '<' and '>'
     * +1 for '\0'
     */
    char buf[2 * sizeof(void *) + 15];
    // Implicitly cast into const void * for formatting
    const void *addr = cf;
    std::snprintf(buf, sizeof(buf), "<cfunction@0x%p>", addr);

    TFunction freplica{
        0,
        cf->error_handler,
        false,
        buf,
        V->frame,
        {},
    };

    nativecall(V, &freplica, argc);
    // Call function pointer
    cf->ptr(V);
}

// Calls a table method.
VIA_FORCEINLINE void methodcall(RTState *VIA_RESTRICT V, TTable *VIA_RESTRICT tbl, const TableKey key, CallArgc argc) noexcept
{
    TValue *method = gettableindex(V, tbl, key, true);
    if (!checkcallable(V, *method))
    {
        std::string err_fmt_str = std::format("Attempt to methodcall non-callable type '{}'", ENUM_NAME(method->type));
        setexitdata(V, 1, err_fmt_str);
        V->abrt = true;
        return;
    }

    if (checkfunction(V, *method))
        nativecall(V, method->val_function, argc);
    else if (checkcfunction(V, *method))
        externcall(V, method->val_cfunction, argc);
    else if (checktable(V, *method))
        // Attempt to call table, a.k.a __call metamethod
        methodcall(V, method->val_table, hashstring(V, "__call"), argc);
    // No else-block because the method object is pre-guaranteed to be callable
}

// Returns the primitive type of value <val>.
VIA_FORCEINLINE TValue type(RTState *VIA_RESTRICT V, TValue v)
{
    auto enum_name = ENUM_NAME(v.type);
    const char *str = dupstring(std::string(enum_name));
    return stackvalue(V, newstring(V, str));
}

// Unified call interface.
// Works on all callable types (TFunction, TCFunction, TTable).
VIA_FORCEINLINE void call(RTState *VIA_RESTRICT V, TValue val, CallArgc argc) noexcept
{
    V->calltype = CallType::CALL;

    if (checkfunction(V, val))
        nativecall(V, val.val_function, argc);
    else if (checkcfunction(V, val))
        externcall(V, val.val_cfunction, argc);
    else if (checktable(V, val))
        methodcall(V, val.val_table, hashstring(V, "__call"), argc);
    else
    {
        TValue callt = type(V, val);
        setexitdata(V, 1, std::format("Attempt to call a {} value", callt.val_string->ptr));
        V->abrt = true;
    }
}

// Returns the length of value <val>, nil if impossible.
VIA_FORCEINLINE TValue len(RTState *VIA_RESTRICT V, TValue val) noexcept
{
    if (checkstring(V, val))
        return stackvalue(V, static_cast<TNumber>(strlen(val.val_string->ptr)));
    else if (checktable(V, val))
    {
        TableKey mhash = hashstring(V, "__len");
        TValue *mmlen = gettableindex(V, val.val_table, mhash, true);

        if (checknil(V, *mmlen))
            return stackvalue(V, static_cast<TNumber>(val.val_table->data.size()));

        call(V, *mmlen, 1);
        return *reinterpret_cast<TValue *>(tspop(V->stack));
    }

    return stackvalue(V);
}

// Loads the value of key <key> in table <tbl> into register <reg>, if present in table.
VIA_FORCEINLINE TValue *loadtableindex(RTState *VIA_RESTRICT V, TTable *VIA_RESTRICT tbl, TableKey key, GPRegister reg) noexcept
{
    TValue *val = gettableindex(V, tbl, key, true);
    setregister(V, reg, *val);
    return val;
}

// Returns the complex type of value <val>.
// Practically the same as `type()`, but returns
// the `__type` key if the given table has it defined.
VIA_FORCEINLINE TValue typeofv(RTState *VIA_RESTRICT V, TValue val) noexcept
{
    if (checktable(V, val))
    {
        TTable *tbl = val.val_table;
        TValue *ty = gettableindex(V, tbl, hashstring(V, "__type"), true);
        // Check if the __type property is Nil
        // if so return the primitive type
        if (checknil(V, *ty))
            return type(V, val);

        TString *tystr = newstring(V, ty->val_string->ptr);
        return stackvalue(V, tystr);
    }

    return type(V, val);
}

// Returns wether if table <tbl> is frozen.
VIA_FORCEINLINE bool isfrozen(RTState *VIA_RESTRICT, TTable *VIA_RESTRICT tbl) noexcept
{
    return tbl->frozen.get();
}

// Freezes (locks, const-ifies) table <tbl>.
VIA_FORCEINLINE void freeze(RTState *VIA_RESTRICT, TTable *VIA_RESTRICT tbl) noexcept
{
    tbl->frozen.set(true);
}

// Sets the metatable of <tbl> to <meta>.
// Potential UB: if <meta> and <tbl> are the same table, the pointer restriction promise will break and cause undefined behavior.
VIA_FORCEINLINE void setmetatable(RTState *VIA_RESTRICT V, TTable *VIA_RESTRICT tbl, TTable *VIA_RESTRICT meta) noexcept
{
    if (isfrozen(V, tbl))
    {
        setexitdata(V, 1, "Cannot set metatable of frozen table");
        V->abrt = true;
    }
    else
        tbl->meta = meta;
}

// Returns the metatable of <tbl>, nil if impossible.
VIA_FORCEINLINE TValue getmetatable(RTState *VIA_RESTRICT V, TTable *VIA_RESTRICT tbl)
{
    if (tbl->meta)
        // We don't need to return a value pointer as the underlying table value is already a pointer
        return stackvalue(V, tbl->meta);

    return stackvalue(V);
}

// Converts an Operand objecti into a TValue object.
VIA_FORCEINLINE TValue toviavalue(RTState *VIA_RESTRICT V, const Operand &operand)
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
    {
        std::string err_fmt_str = std::format("Cannot interpret operand '{}' as a data type", ENUM_NAME(operand.type));
        setexitdata(V, 1, err_fmt_str);
        V->abrt = true;
        break;
    }
    }

    return stackvalue(V);
}

// Schedules a yield <ms>.
// Only yields the VM thread.
VIA_FORCEINLINE void yield(RTState *VIA_RESTRICT V, float ms) noexcept
{
    if (V->tstate == ThreadState::RUNNING)
        V->yield = ms;
}

// Saves the state by copying it and storing it.
VIA_FORCEINLINE void savestate(RTState *VIA_RESTRICT V) noexcept
{
    V->sstate = new RTState(*V);
}

// Restores the saved state by overwriting the state pointer.
// Resets the saved state pointer.
VIA_FORCEINLINE void restorestate(RTState *VIA_RESTRICT V) noexcept
{
    *V = *V->sstate;
    V->sstate = nullptr;
}

// Performs an arithmetic operation determined by <op> between <lhs> and <rhs>.
VIA_FORCEINLINE TValue arith(RTState *VIA_RESTRICT V, TValue lhs, TValue rhs, OpCode op) noexcept
{
#ifdef VIA_DEBUG
    VIA_ASSERT(checknumber(V, rhs), "arith(): Expected Number for rhs");
#endif
    if (checknumber(V, lhs))
    {
        switch (op)
        {
        case OpCode::ADDRR:
        case OpCode::ADDRN:
        case OpCode::ADDNR:
        case OpCode::ADDNN:
            return stackvalue(V, lhs.val_number + rhs.val_number);
        case OpCode::SUBRR:
        case OpCode::SUBRN:
        case OpCode::SUBNR:
        case OpCode::SUBNN:
            return stackvalue(V, lhs.val_number - rhs.val_number);
        case OpCode::MULRR:
        case OpCode::MULRN:
        case OpCode::MULNR:
        case OpCode::MULNN:
            return stackvalue(V, lhs.val_number * rhs.val_number);
        case OpCode::DIVRR:
        case OpCode::DIVRN:
        case OpCode::DIVNR:
        case OpCode::DIVNN:
            return stackvalue(V, lhs.val_number / rhs.val_number);
        case OpCode::POWRR:
        case OpCode::POWRN:
        case OpCode::POWNR:
        case OpCode::POWNN:
            return stackvalue(V, std::pow(lhs.val_number, rhs.val_number));
        case OpCode::MODRR:
        case OpCode::MODRN:
        case OpCode::MODNR:
        case OpCode::MODNN:
            return stackvalue(V, std::fmod(lhs.val_number, rhs.val_number));
        default:
            break;
        }

        return stackvalue(V);
    }
    else if (checktable(V, lhs))
    {
        TValue *mm = getmetamethod(V, lhs, op);
        pushval(V, lhs); // self
        pushval(V, rhs); // other
        call(V, *mm, 2);
        return *reinterpret_cast<TValue *>(tspop(V->stack));
    }

    setexitdata(V, 1, "Invalid arithmetic operator");
    V->abrt = true;
    return stackvalue(V);
}

// Performs inline artihmetic between <lhs*> and <rhs>, operation determined by <op>.
VIA_FORCEINLINE void iarith(RTState *VIA_RESTRICT V, TValue *lhs, TValue rhs, OpCode op) noexcept
{
#ifdef VIA_DEBUG
    VIA_ASSERT(checknumber(V, rhs), "arith(): Expected Number for rhs");
    VIA_ASSERT(rhs.val_number != 0.0f, "arith(): Expected non-zero Number for rhs");
#endif
    if (checknumber(V, *lhs))
    {
        switch (op)
        {
        case OpCode::ADDIR:
        case OpCode::ADDIN:
            lhs->val_number += rhs.val_number;
            break;
        case OpCode::SUBIR:
        case OpCode::SUBIN:
            lhs->val_number -= rhs.val_number;
            break;
        case OpCode::MULIR:
        case OpCode::MULIN:
            lhs->val_number *= rhs.val_number;
            break;
        case OpCode::DIVIR:
        case OpCode::DIVIN:
            lhs->val_number /= rhs.val_number;
            break;
        case OpCode::POWIR:
        case OpCode::POWIN:
            lhs->val_number = std::pow(lhs->val_number, rhs.val_number);
            break;
        case OpCode::MODIR:
        case OpCode::MODIN:
            lhs->val_number = std::fmod(lhs->val_number, rhs.val_number);
            break;
        default:
            break;
        }
    }
    else if (checktable(V, *lhs))
    {
        TValue *mm = getmetamethod(V, *lhs, op);
        pushval(V, *lhs); // self
        pushval(V, rhs);  // other
        call(V, *mm, 2);
        *lhs = *reinterpret_cast<TValue *>(tspop(V->stack));
    }

    setexitdata(V, 1, "Invalid arithmetic operator");
    V->abrt = true;
}

} // namespace via