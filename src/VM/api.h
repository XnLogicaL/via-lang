/* This file is a part of the via programming language at
 * https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "gc.h"
#include "instruction.h"
#include "opcode.h"
#include "register.h"
#include "stack.h"
#include "state.h"
#include "types.h"

// Fuck.
#include <cmath>

namespace via
{

// Manually sets the VM exit data.
VIA_MAXOPTIMIZE void setexitdata(RTState *VIA_RESTRICT V, ExitCode exitc, const std::string &exitm) noexcept
{
    V->exitc = exitc;
    V->exitm = dupstring(exitm);
}

// Returns whether if <addr> is within the bounds of the instruction pipeline.
VIA_MAXOPTIMIZE bool isvalidjmpaddr(RTState *VIA_RESTRICT V, Instruction *VIA_RESTRICT const addr) noexcept
{
    return (addr >= V->ihp) && (addr <= V->ibp);
}

// Sets register <reg> to the given value <val>.
// Wrapper for `rsetregister`.
VIA_MAXOPTIMIZE void setregister(RTState *VIA_RESTRICT V, RegId reg, TValue val) noexcept
{
    rsetregister(V->ralloc, reg, val);
}

// Returns the value of register <reg>.
// Wrapper for `rgetregister`.
VIA_MAXOPTIMIZE TValue *getregister(RTState *VIA_RESTRICT V, RegId reg) noexcept
{
    return rgetregister(V->ralloc, reg);
}

// Returns the underlying pointer of a data type if present, nullptr if not.
VIA_FORCEINLINE void *topointer(RTState *VIA_RESTRICT, TValue val) noexcept
{
    switch (val.type)
    {
    case ValueType::CFunction:
        return val.val_cfunction;
    case ValueType::Function:
        return val.val_function;
    case ValueType::Table:
        return val.val_table;
    case ValueType::String:
        return val.val_string;
    default:
        return nullptr;
    }
}

// Returns whether if <val> has a heap component.
VIA_FORCEINLINE bool isheap(RTState *VIA_RESTRICT V, TValue val) noexcept
{
    return topointer(V, val) != nullptr;
}

// Compares 2 values and returns whether if they are equal.
// Optimized for maximum performance.
VIA_MAXOPTIMIZE bool compare(RTState *VIA_RESTRICT V, TValue v0, TValue v1) noexcept
{
    // Early return; if types aren't the same they cannot be equivalent
    if (v0.type != v1.type)
        return false;

    // Optimized for most efficient branching by sorting the cases from most common to least
    switch (v0.type)
    {
    case ValueType::Number:
        return v0.val_number == v1.val_number;
    case ValueType::Bool:
        return v0.val_boolean == v1.val_boolean;
    case ValueType::Nil:
        // Nil values are always equal
        return true;
    case ValueType::Pointer:
        return v0.val_pointer == v1.val_pointer;
        // Even though values with a heap component are considered unique,
        // strings are an exception due to well... common sense,
        // eg. you would compare strings with each other but not tables.
    case ValueType::String:
        return !std::strcmp(v0.val_string->ptr, v1.val_string->ptr);
    default:
        return topointer(V, v0) == topointer(V, v1);
    }

    VIA_UNREACHABLE();
    return false;
};

// Compares the values of two registers and returns whether if they are
// equivalent. Optimized for maximum performance.
VIA_MAXOPTIMIZE bool compareregisters(RTState *VIA_RESTRICT V, RegId reg0, RegId reg1) noexcept
{
    // Early return if registers are equivalent
    if (reg0 == reg1)
        return true;

    TValue v0 = *getregister(V, reg0);
    TValue v1 = *getregister(V, reg1);
    // No need to assert &v0 == &v1 because
    // the first equality check is basically the same thing, just at a higher
    // level of abstraction.

    // Early type mismatch check
    if (v0.type != v1.type)
        return false;

    return compare(V, v0, v1);
}

// Pushes a value onto the stack.
// Wrapper for `tspush`. Copies the value.
VIA_MAXOPTIMIZE void pushval(RTState *VIA_RESTRICT V, TValue val)
{
    tspush(V->stack, val);
}

// Pops a value from the stack. Returns it as a TValue *.
// Wrapper for `tspop`. Does not clear pointer tags.
VIA_MAXOPTIMIZE TValue popval(RTState *VIA_RESTRICT V)
{
    return tspop(V->stack);
}

// Returns a value that contains a String that represents the string-ified
// version of <val>. The return value is guaranteed to be a String type.
VIA_INLINE TValue tostring(RTState *VIA_RESTRICT V, TValue val) noexcept
{
    // If the value union is tagged as a String type but an invalid string value,
    // that is classified as undefined behavior and should be explicitly handled
    // by the end user. It is guaranteed to NEVER occur under compiled bytecode
    if (checkstring(V, val))
        return val;

    switch (val.type)
    {
    case ValueType::Number:
    {
        std::string str = std::to_string(val.val_number);
        TString *tstr = new TString(V, str.c_str());
        return TValue(tstr);
    }
    case ValueType::Bool:
    {
        TString *str = new TString(V, val.val_boolean ? "true" : "false");
        return TValue(str);
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

        TString *tstr = new TString(V, str.c_str());
        return TValue(tstr);
    }
    case ValueType::Function:
    {
        const void *faddr = val.val_function;
        std::string str = std::format("<function@{}>", faddr);
        TString *tstr = new TString(V, str.c_str());
        return TValue(tstr);
    }
    case ValueType::CFunction:
    {
        // This has to be explicitly casted because function pointers be weird
        const void *cfaddr = val.val_cfunction;
        std::string str = std::format("<cfunction@{}>", cfaddr);
        TString *tstr = new TString(V, str.c_str());
        return TValue(tstr);
    }
    default:
        TString *tstr = new TString(V, "nil");
        return TValue(tstr);
    }

    VIA_UNREACHABLE();
    return TValue();
}

// Returns the truthiness of value <val>.
// Guaranteed to be a Bool type.
VIA_FORCEINLINE TValue tobool(RTState *VIA_RESTRICT V, TValue val) noexcept
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
        return TValue(false);
    default:
        return TValue(true);
    }

    VIA_UNREACHABLE();
    return TValue();
}

// Returns the number representation of value <val>.
// Returns Nil if impossible, unlike `vtostring` or `vtobool`.
VIA_FORCEINLINE TValue tonumber(RTState *VIA_RESTRICT V, TValue &val) noexcept
{
    if (checknumber(V, val))
        return val;

    switch (val.type)
    {
    case ValueType::String:
        return TValue(std::stod(val.val_string->ptr));
    case ValueType::Bool:
        return TValue(val.val_boolean ? 1.0f : 0.0f);
    default:
        break;
    }

    return TValue();
}

// Utility function for quick table indexing.
// Returns the value of key <key> if present in table <tbl>.
VIA_FORCEINLINE TValue gettable(RTState *VIA_RESTRICT V, TTable *VIA_RESTRICT tbl, TableKey key, bool search_meta) noexcept
{
    auto it = tbl->data.find(key);
    if (it != tbl->data.end())
        return it->second;
    else if (search_meta && tbl->meta)
        // Disable metatable search to prevent chain searching
        // Which can cause infinite loops, crashes, and other bugs
        return gettable(V, tbl->meta, key, false);

    // This has to be a pointer because we don't know if the value is a
    // non-pointer primitive type
    return TValue(V);
}

// Assigns the given value <val> to key <key> in table <tbl>.
VIA_FORCEINLINE void settable(RTState *VIA_RESTRICT V, TTable *VIA_RESTRICT tbl, TableKey key, TValue val) noexcept
{
    if (checknil(V, val))
    {
        TValue tbl_val = gettable(V, tbl, key, false);

        if (!checknil(V, val))
            tbl->data.erase(key);
    }
    else
        tbl->data[key] = val;
}

// Utility function for quickly geting a metamethod associated to <op>.
VIA_FORCEINLINE TValue getmetamethod(RTState *VIA_RESTRICT V, TValue val, OpCode op)
{
    if (!checktable(V, val))
        return TValue(V);

#define GET_METHOD(id) (gettable(V, val.val_table, hashstring(V, id), true))
    switch (op)
    {
    case OpCode::ADD:
        return GET_METHOD("__add");
    case OpCode::SUB:
        return GET_METHOD("__sub");
    case OpCode::MUL:
        return GET_METHOD("__mul");
    case OpCode::DIV:
        return GET_METHOD("__div");
    case OpCode::POW:
        return GET_METHOD("__pow");
    case OpCode::MOD:
        return GET_METHOD("__mod");
    case OpCode::NEG:
        return GET_METHOD("__neg");
    case OpCode::INCREMENT:
        return GET_METHOD("__inc");
    case OpCode::DECREMENT:
        return GET_METHOD("__dec");
    case OpCode::CONCAT:
        return GET_METHOD("__con");
    default:
        VIA_ASSERT(false, "getmetamethod(): Unknown opcode");
        break;
    }

    return TValue(V);
#undef GET_METHOD
}

// Returns a local variable located at <offset>, relative to the stack base.
VIA_FORCEINLINE TValue getlocal(RTState *VIA_RESTRICT V, LocalId offset) noexcept
{
    // Check if LocalId is out of bounds;
    // this is CRUCIAL, and prevents UB upon stack dereferencing
    if (offset > V->stack->sp)
        return TValue(V);

    StkAddr stack_address = V->stack->sbp + offset;
    StkVal val = *stack_address; //! BIG WARNING: This is UB without bound checks!!!

    return val;
}

// Reassigns the stack value at offset <offset> to <val>.
VIA_FORCEINLINE void setlocal(RTState *VIA_RESTRICT V, LocalId offset, TValue val)
{
    // Check if LocalId is out of bounds,
    // this is CRUCIAL, and prevents UB upon stack operations.
    if (offset > V->stack->sp)
    {
        std::string identifier("<unknown-symbol>");
        if (V->G->symtable->size() >= offset)
            identifier = V->G->symtable->at(offset);

        VIA_ASSERT_SILENT(false, std::format("Attempt to assign to undeclared variable '{}'", identifier));
    }

    StkAddr stack_address = V->stack->sbp + offset;
    *stack_address = val; //! BIG WARNING: This is UB without bound checks!!!
}

// Returns the global with id <ident>, nil if it has not been declared.
VIA_FORCEINLINE TValue getglobal(RTState *VIA_RESTRICT V, kGlobId ident) noexcept
{
    auto it = V->G->gtable->find(ident);
    if (it != V->G->gtable->end())
        return it->second;

    return TValue();
}

// Attempts to declare a new global constant.
VIA_FORCEINLINE void setglobal(RTState *VIA_RESTRICT V, kGlobId ident, TValue val)
{
    auto it = V->G->gtable->find(ident);
    // This is not a silent assertion because it is only possible if a global is
    // reassigned, which is not possible under generated bytecode.
    VIA_ASSERT(it == V->G->gtable->end(), "setglobal(): attempt to reassign global constant");
    (*V->G->gtable)[ident] = val;
}

// Returns the nth argument relative to the saved stack pointer of the current
// stack frame.
VIA_FORCEINLINE TValue getargument(RTState *VIA_RESTRICT V, LocalId offset) noexcept
{
    // Check if the argument is out of bounds, return nil if so
    if (offset >= V->argc)
        return TValue(V);

    // Calculate the stack position of the argument
    StkPos stack_offset = V->ssp + V->argc - 1 - offset;
    // Retrieve stack value
    StkVal val = V->stack->sbp[stack_offset];
    return val;
}

// Performs a native return operation, restores the stack and some other state
// information.
VIA_FORCEINLINE void nativeret(RTState *VIA_RESTRICT V, CallArgc retc) noexcept
{
    std::vector<TValue> ret_values;
    // Restore state
    V->ip = V->frame->ret_addr;
    V->frame = V->frame->caller;

    // Save return values
    for (CallArgc i = 0; i < retc; i++)
    {
        TValue ret_val = popval(V);
        ret_values.push_back(ret_val);
    }

    // Restore stack pointer
    V->stack->sp = V->ssp;

    // Clean up arguments
    for (CallArgc i = 0; i < V->argc; i++)
        popval(V);

    // Restore return values
    for (int i = retc - 1; i >= 0; i--) // Reverse order for pushing return values
        pushval(V, ret_values[i]);
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
        15 additional characters:
        +9 for 'cfunction'
        +1 for '@'
        +2 for '0x'
        +2 for '<' and '>'
        +1 for '\0'
    */
    char buf[2 * sizeof(void *) + 15];
    // Implicitly cast into const void * for formatting
    const void *addr = cf;
    std::snprintf(buf, sizeof(buf), "<cfunction@0x%p>", addr);

    TFunction func(0, buf, V->ip, V->frame, {}, cf->error_handler, false);

    nativecall(V, &func, argc);
    // Call function pointer
    cf->ptr(V);
}

// Calls a table method.
VIA_INLINE void methodcall(RTState *VIA_RESTRICT V, TTable *VIA_RESTRICT tbl, const TableKey key, CallArgc argc) noexcept
{
    TValue method = gettable(V, tbl, key, true);
    if (!checkcallable(V, method))
    {
        std::string err = std::format("Attempt to methodcall non-callable type '{}'", ENUM_NAME(method.type));
        setexitdata(V, 1, err);
        V->abrt = true;
        return;
    }

    if (checkfunction(V, method))
        nativecall(V, method.val_function, argc);
    else if (checkcfunction(V, method))
        externcall(V, method.val_cfunction, argc);
    // No else-block because the method object is pre-guaranteed to be callable
}

// Returns the primitive type of value <val>.
VIA_FORCEINLINE TValue type(RTState *VIA_RESTRICT V, TValue v) noexcept
{
    auto enum_name = ENUM_NAME(v.type);
    char *str = dupstring(std::string(enum_name));
    gcaddcallback(V, [str]() { delete str; });

    return TValue(new TString(V, str));
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
        return TValue(static_cast<TNumber>(strlen(val.val_string->ptr)));
    else if (checktable(V, val))
    {
        TableKey mhash = hashstring(V, "__len");
        TValue mmlen = gettable(V, val.val_table, mhash, true);

        if (checknil(V, mmlen))
            return TValue(static_cast<TNumber>(val.val_table->data.size()));

        call(V, mmlen, 1);
        return popval(V);
    }

    return TValue();
}

// Loads the value of key <key> in table <tbl> into register <reg>, if present
// in table.
VIA_FORCEINLINE TValue loadtableindex(RTState *VIA_RESTRICT V, TTable *VIA_RESTRICT tbl, TableKey key, RegId reg) noexcept
{
    TValue val = gettable(V, tbl, key, true);
    setregister(V, reg, val);
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
        TValue ty = gettable(V, tbl, hashstring(V, "__type"), true);
        // Check if the __type property is Nil
        // if so return the primitive type
        if (checknil(V, ty))
            return type(V, val);

        TString *tystr = new TString(V, ty.val_string->ptr);
        return TValue(tystr);
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
//! Potential UB: if <meta> and <tbl> are the same table, the pointer
//! restriction promise will break and cause undefined behavior.
VIA_FORCEINLINE void setmetatable(RTState *VIA_RESTRICT V, TTable *VIA_RESTRICT tbl, TTable *VIA_RESTRICT meta)
{
    // Safeguard for the UB mentioned above, may or may not fail, all on the
    // end-user.
    VIA_ASSERT(tbl != meta, "Cannot set metatable of table to self");

    if (isfrozen(V, tbl))
    {
        setexitdata(V, 1, "Cannot set metatable of frozen table");
        V->abrt = true;
    }
    else
        tbl->meta = meta;
}

// Returns the metatable of <tbl>, nil if impossible.
VIA_FORCEINLINE TValue getmetatable(RTState *VIA_RESTRICT, TTable *VIA_RESTRICT tbl)
{
    if (tbl->meta)
        // We don't need to return a value pointer as the underlying table value is
        // already a pointer
        return TValue(tbl->meta);

    return TValue();
}

// Converts an Operand object into a TValue object.
VIA_FORCEINLINE TValue toviavalue(RTState *VIA_RESTRICT V, const Operand &operand)
{
    switch (operand.type)
    {
    case OperandType::Number:
        return TValue(operand.val_number);
    case OperandType::Bool:
        return TValue(operand.val_boolean);
    case OperandType::String:
        return TValue(new TString(V, operand.val_string));
    case via::OperandType::Nil:
        return TValue();
    default:
    {
        std::string err = std::format("Cannot interpret operand '{}' as a data type", ENUM_NAME(operand.type));
        setexitdata(V, 1, err);
        V->abrt = true;
        break;
    }
    }

    return TValue();
}

// Schedules a yield <ms>.
// Only yields the VM thread.
VIA_FORCEINLINE void yield(RTState *VIA_RESTRICT V, YldTime ms) noexcept
{
    if (V->tstate == ThreadState::RUNNING)
        V->yield = ms;
}

// Saves the state by copying it and storing it.
VIA_FORCEINLINE void savestate(RTState *VIA_RESTRICT V) noexcept
{
    // Cleanup previously saved state, if present
    if (V->sstate)
        delete V->sstate;

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
VIA_FORCEINLINE TValue arith(RTState *VIA_RESTRICT V, TValue lhs, TValue rhs, OpCode op)
{
    VIA_ASSERT(checknumber(V, rhs), "arith(): rhs must be a number");

    if (checknumber(V, lhs))
    {
        switch (op)
        {
        case OpCode::ADD:
            return TValue(lhs.val_number + rhs.val_number);
        case OpCode::SUB:
            return TValue(lhs.val_number - rhs.val_number);
        case OpCode::MUL:
            return TValue(lhs.val_number * rhs.val_number);
        case OpCode::DIV:
            VIA_ASSERT_SILENT(rhs.val_number != 0.0f, "Division by zero");
            return TValue(lhs.val_number / rhs.val_number);
        case OpCode::POW:
            return TValue(std::pow(lhs.val_number, rhs.val_number));
        case OpCode::MOD:
            return TValue(std::fmod(lhs.val_number, rhs.val_number));
        default:
            VIA_ASSERT(false, "arith(): Unknown opcode");
            break;
        }

        return TValue();
    }
    else if (checktable(V, lhs))
    {
        TValue metamethod = getmetamethod(V, lhs, op);
        pushval(V, lhs); // self
        pushval(V, rhs); // other
        call(V, metamethod, 2);
        return popval(V);
    }

    VIA_ASSERT(false, "arith(): Invalid lhs operand");
    return TValue();
}

// Performs inline artihmetic between <lhs*> and <rhs>, operation determined by <op>.
VIA_FORCEINLINE void iarith(RTState *VIA_RESTRICT V, TValue *lhs, TValue rhs, OpCode op)
{
    if (checknumber(V, *lhs))
    {
        switch (op)
        {
        case OpCode::ADD:
            lhs->val_number += rhs.val_number;
            break;
        case OpCode::SUB:
            lhs->val_number -= rhs.val_number;
            break;
        case OpCode::MUL:
            lhs->val_number *= rhs.val_number;
            break;
        case OpCode::DIV:
            VIA_ASSERT_SILENT(rhs.val_number != 0.0f, "Division by zero");
            lhs->val_number /= rhs.val_number;
            break;
        case OpCode::POW:
            lhs->val_number = std::pow(lhs->val_number, rhs.val_number);
            break;
        case OpCode::MOD:
            lhs->val_number = std::fmod(lhs->val_number, rhs.val_number);
            break;
        default:
            VIA_ASSERT(false, "iarith(): Unknown opcode");
            break;
        }
    }
    else if (checktable(V, *lhs))
    {
        TValue metamethod = getmetamethod(V, *lhs, op);
        pushval(V, *lhs); // self
        pushval(V, rhs);  // other
        call(V, metamethod, 2);
        *lhs = popval(V);
    }

    VIA_ASSERT(false, "iarith(): Invalid lhs operand");
}

} // namespace via