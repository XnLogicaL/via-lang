/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "gc.h"
#include "instruction.h"
#include "opcode.h"
#include "register.h"
#include "state.h"
#include "types.h"
#include "vmapi.h"

// Fuck.
#include <cmath>

namespace via
{

static const TValue nil = TValue();

// Returns the underlying pointer of a data type if present, nullptr if not.
VIA_FORCEINLINE void *topointer(const TValue &val) noexcept
{
    return impl::__to_pointer(val);
}

// Returns whether if <val> has a heap component.
VIA_FORCEINLINE bool isheap(const TValue &val) noexcept
{
    return topointer(val) != nullptr;
}

// Compares 2 values and returns whether if they are equal.
// Optimized for maximum performance.
VIA_MAXOPTIMIZE bool compare(const TValue &v0, const TValue &v1) noexcept
{
    // Early return; if types aren't the same they cannot be equivalent
    if (v0.type != v1.type)
        return false;

    // Optimized for most efficient branching by sorting the cases from most common to least
    switch (v0.type)
    {
    case ValueType::number:
        return v0.val_number == v1.val_number;
    case ValueType::boolean:
        return v0.val_boolean == v1.val_boolean;
    case ValueType::nil:
        // Nil values are always equal
        return true;
    case ValueType::string:
        return !std::strcmp(v0.val_string->ptr, v1.val_string->ptr);
    default:
        return topointer(v0) == topointer(v1);
    }

    VIA_UNREACHABLE();
    return false;
};

// Compares the values of two registers and returns whether if they are
// equivalent. Optimized for maximum performance.
VIA_MAXOPTIMIZE bool compareregisters(State *VIA_RESTRICT V, RegId reg0, RegId reg1) noexcept
{
    // Early return if registers are equivalent
    if (reg0 == reg1)
        return true;

    TValue &v0 = *getregister(V, reg0);
    TValue &v1 = *getregister(V, reg1);
    // No need to assert &v0 == &v1 because
    // the first equality check is basically the same thing, just at a higher
    // level of abstraction.
    if (v0.type != v1.type)
        return false;

    return compare(v0, v1);
}

// Pushes a copy of the given value onto the stack.
VIA_MAXOPTIMIZE void push(State *VIA_RESTRICT V, const TValue &val)
{
    VIA_ASSERT(V->sp < VIA_VM_STACK_SIZE / sizeof(TValue), "stack overflow");
    V->sbp[V->sp++] = val.clone();
}

// Pops a value from the stack and returns a copy of it.
VIA_MAXOPTIMIZE TValue pop(State *VIA_RESTRICT V)
{
    VIA_ASSERT(V->sp == 0, "stack underflow");
    return V->sbp[V->sp--].clone();
}

// Returns a copy of the top-most value on the stack.
VIA_MAXOPTIMIZE TValue top(State *VIA_RESTRICT V)
{
    VIA_ASSERT(V->sp == 0, "stack underflow");
    return V->sbp[V->sp--].clone();
}

// Returns a value that contains a String that represents the string-ified
// version of <val>. The return value is guaranteed to be a String type.
VIA_INLINE TValue tostring(State *VIA_RESTRICT V, const TValue &val) noexcept
{
    // If the value union is tagged as a String type but an invalid string value,
    // that is classified as undefined behavior and should be explicitly handled
    // by the end user. It is guaranteed to NEVER occur under compiled bytecode
    if (checkstring(val))
        return val.clone();

    switch (val.type)
    {
    case ValueType::number: {
        std::string str = std::to_string(val.val_number);
        TString *tstr = new TString(V, str.c_str());
        return TValue(tstr);
    }
    case ValueType::boolean: {
        TString *str = new TString(V, val.val_boolean ? "true" : "false");
        return TValue(str);
    }
    case ValueType::table: {
        std::string str = "{";

        for (auto &elem : val.val_table->data)
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
    case ValueType::function: {
        const void *faddr = val.val_function;
        std::string str = std::format("<function@{}>", faddr);
        TString *tstr = new TString(V, str.c_str());
        return TValue(tstr);
    }
    case ValueType::cfunction: {
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
    return nil.clone();
}

VIA_FORCEINLINE std::string tocxxstring(State *VIA_RESTRICT V, const TValue &val) noexcept
{
    TValue str = tostring(V, val);
    return std::string(str.val_string->ptr);
}

// Returns the truthiness of value <val>.
// Guaranteed to be a Bool type.
VIA_FORCEINLINE TValue tobool(const TValue &val) noexcept
{
    // If the TValue union is tagged as Bool type but it
    // doesn't actually contain a boolean value, that is undefined behavior
    // and is the responsibility of the end-user.
    if (checkbool(val))
        return val.clone();

    switch (val.type)
    {
    // Nil and Monostate is the only falsy type
    case ValueType::nil:
    case ValueType::monostate:
        return TValue(false);
    default:
        return TValue(true);
    }

    VIA_UNREACHABLE();
    return nil.clone();
}

VIA_FORCEINLINE bool tocxxbool(const TValue &val) noexcept
{
    TValue bl = tobool(val);
    return bl.val_boolean;
}

// Returns the number representation of value <val>.
// Returns Nil if impossible, unlike `vtostring` or `vtobool`.
VIA_FORCEINLINE TValue tonumber(const TValue &val) noexcept
{
    if (checknumber(val))
        return val.clone();

    switch (val.type)
    {
    case ValueType::string:
        return TValue(std::stod(val.val_string->ptr));
    case ValueType::boolean:
        return TValue(val.val_boolean ? 1.0f : 0.0f);
    default:
        break;
    }

    return nil.clone();
}

template<typename T = double>
    requires std::is_arithmetic_v<T>
VIA_FORCEINLINE T tocxxnumber(const TValue &val) noexcept
{
    TValue dbl = tonumber(val);

    if (checknil(dbl))
        return std::numeric_limits<T>::quiet_NaN();

    return static_cast<T>(dbl.val_number);
}

// Utility function for quick table indexing.
// Returns the value of key <key> if present in table <tbl>.
VIA_FORCEINLINE TValue gettable(TTable *VIA_RESTRICT tbl, TableKey key, bool search_meta) noexcept
{
    auto it = tbl->data.find(key);
    if (it != tbl->data.end())
        return it->second.clone();
    else if (search_meta && tbl->meta)
        // Disable metatable search to prevent chain searching
        // Which can cause infinite loops, crashes, and other bugs
        return gettable(tbl->meta, key, false);

    // This has to be a pointer because we don't know if the value is a
    // non-pointer primitive type
    return nil.clone();
}

// Assigns the given value <val> to key <key> in table <tbl>.
VIA_FORCEINLINE void settable(TTable *VIA_RESTRICT tbl, TableKey key, const TValue &val) noexcept
{
    if (checknil(val))
    {
        const TValue &tbl_val = gettable(tbl, key, false);

        if (!checknil(tbl_val))
            tbl->data.erase(key);
    }
    else
        tbl->data.emplace(key, val.clone());
}

// Utility function for quickly geting a metamethod associated to <op>.
VIA_FORCEINLINE TValue getmetamethod(const TValue &val, OpCode op)
{
    if (!checktable(val))
        return nil.clone();

#define GET_METHOD(id) (gettable(val.val_table, hashstring(id), true))
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
        VIA_ASSERT(false, "non-operator opcode");
        break;
    }

    return nil.clone();
#undef GET_METHOD
}

// Returns a local variable located at <offset>, relative to the stack base.
VIA_FORCEINLINE const TValue &getlocal(State *VIA_RESTRICT V, LocalId offset) noexcept
{
    // Check if LocalId is out of bounds
    if (offset > V->sp)
        return nil;

    TValue *stack_address = V->sbp + offset;
    TValue &val = *stack_address;
    return val;
}

// Reassigns the stack value at offset <offset> to <val>.
VIA_FORCEINLINE void setlocal(State *VIA_RESTRICT V, LocalId offset, const TValue &val)
{
    // Check if LocalId is out of bounds,
    // this is CRUCIAL, and prevents UB upon stack operations.
    if (offset > V->sp)
    {
        std::string identifier("<unknown-symbol>");
        if (V->G->symtable.size() >= offset)
            identifier = V->G->symtable.at(offset);
    }

    TValue *stack_address = V->sbp + offset;
    *stack_address = val.clone();
}

// Returns the global with id <ident>, nil if it has not been declared.
VIA_FORCEINLINE const TValue &getglobal(State *VIA_RESTRICT V, kGlobId ident) noexcept
{
    auto it = V->G->gtable.find(ident);
    if (it != V->G->gtable.end())
        return it->second;

    return nil;
}

// Attempts to declare a new global constant.
VIA_FORCEINLINE void setglobal(State *VIA_RESTRICT V, kGlobId ident, const TValue &val)
{
    auto it = V->G->gtable.find(ident);
    VIA_ASSERT(it == V->G->gtable.end(), "cannot reassign global");
    V->G->gtable.emplace(ident, val.clone());
}

// Returns the nth argument relative to the saved stack pointer of the current
// stack frame.
VIA_FORCEINLINE const TValue &getargument(State *VIA_RESTRICT V, LocalId offset) noexcept
{
    // Check if the argument is out of bounds, return nil if so
    if (offset >= V->argc)
        return nil;

    // Calculate the stack position of the argument
    StkPos stack_offset = V->ssp + V->argc - 1 - offset;
    // Retrieve stack value
    TValue &val = V->sbp[stack_offset];
    return val;
}

// Performs a native return operation, restores the stack and some other state
// information.
VIA_FORCEINLINE void nativeret(State *VIA_RESTRICT V, size_t retc) noexcept
{
    std::vector<TValue> ret_values;
    // Restore state
    V->ip = V->frame->ret_addr;
    V->frame = V->frame->caller;

    // Save return values
    for (size_t i = 0; i < retc; i++)
    {
        TValue ret_val = pop(V);
        ret_values.push_back(std::move(ret_val));
    }

    // Restore stack pointer
    V->sp = V->ssp;

    // Clean up arguments
    for (size_t i = 0; i < V->argc; i++)
        pop(V);

    // Restore return values
    for (int i = retc - 1; i >= 0; i--) // Reverse order for pushing return values
        push(V, ret_values[i]);
}

// Calls a native function.
VIA_FORCEINLINE void nativecall(State *VIA_RESTRICT V, TFunction *VIA_RESTRICT callee, size_t argc) noexcept
{
    // Save state
    callee->caller = V->frame;
    callee->ret_addr = V->ip;

    // Setup call information
    V->frame = callee;
    V->ip = callee->bytecode.data();
    V->argc = argc;
    V->ssp = V->sp;
}

// Calls a C function pointer.
// Mimics stack behavior as it would behave while calling a native function.
VIA_FORCEINLINE void externcall(State *VIA_RESTRICT V, TCFunction *VIA_RESTRICT cf, size_t argc) noexcept
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
VIA_INLINE void methodcall(State *VIA_RESTRICT V, TTable *VIA_RESTRICT tbl, TableKey key, size_t argc) noexcept
{
    const TValue &method = gettable(tbl, key, true);

    if (checkfunction(method))
        nativecall(V, method.val_function, argc);
    else if (checkcfunction(method))
        externcall(V, method.val_cfunction, argc);
    else
        VIA_ASSERT(false, "value is not callable");
}

// Returns the primitive type of value <val>.
VIA_FORCEINLINE TValue type(State *VIA_RESTRICT V, const TValue &val) noexcept
{
    char *str = dupstring(std::string(ENUM_NAME(val.type)));
    gcaddcallback(V, [&str]() { delete str; });

    return TValue(new TString(V, str));
}

// Unified call interface.
// Works on all callable types (TFunction, TCFunction, TTable).
VIA_FORCEINLINE void call(State *VIA_RESTRICT V, const TValue &val, size_t argc) noexcept
{
    V->calltype = CallType::CALL;

    if (checkfunction(val))
        nativecall(V, val.val_function, argc);
    else if (checkcfunction(val))
        externcall(V, val.val_cfunction, argc);
    else if (checktable(val))
        methodcall(V, val.val_table, hashstring("__call"), argc);
    else
        VIA_ASSERT(false, "value is not callable");
}

// Returns the length of value <val>, nil if impossible.
VIA_FORCEINLINE TValue len(State *VIA_RESTRICT V, const TValue &val) noexcept
{
    if (checkstring(val))
        return TValue(static_cast<TNumber>(strlen(val.val_string->ptr)));
    else if (checktable(val))
    {
        TableKey metamethod_key = hashstring("__len");
        const TValue &metamethod = gettable(val.val_table, metamethod_key, true);

        if (checknil(metamethod))
            return TValue(static_cast<TNumber>(val.val_table->data.size()));

        call(V, metamethod, 1);
        return pop(V);
    }

    return nil.clone();
}

// Returns the complex type of value <val>.
// Practically the same as `type()`, but returns
// the `__type` key if the given table has it defined.
VIA_FORCEINLINE TValue typeofv(State *VIA_RESTRICT V, const TValue &val) noexcept
{
    if (checktable(val))
    {
        TTable *tbl = val.val_table;
        const TValue &ty = gettable(tbl, hashstring("__type"), true);
        // Check if the __type property is Nil
        // if so return the primitive type
        if (checknil(ty))
            return type(V, val);

        TString *tystr = new TString(V, ty.val_string->ptr);
        return TValue(tystr);
    }

    return type(V, val);
}

// Returns wether if table <tbl> is frozen.
VIA_FORCEINLINE bool isfrozen(TTable *VIA_RESTRICT tbl) noexcept
{
    return tbl->frozen.get();
}

// Freezes (locks, const-ifies) table <tbl>.
VIA_FORCEINLINE void freeze(TTable *VIA_RESTRICT tbl) noexcept
{
    tbl->frozen.set(true);
}

// Sets the metatable of <tbl> to <meta>.
VIA_FORCEINLINE void setmetatable(TTable *VIA_RESTRICT tbl, TTable *VIA_RESTRICT meta)
{
    VIA_ASSERT(!isfrozen(tbl), "table is frozen");
    tbl->meta = meta;
}

// Returns the metatable of <tbl>, nil if impossible.
VIA_FORCEINLINE TValue getmetatable(State *VIA_RESTRICT, TTable *VIA_RESTRICT tbl)
{
    if (tbl->meta)
        return TValue(tbl->meta);

    return nil.clone();
}

// Performs an arithmetic operation determined by <op> between <lhs> and <rhs>.
VIA_FORCEINLINE TValue arith(State *VIA_RESTRICT V, const TValue &lhs, const TValue &rhs, OpCode op)
{
    VIA_ASSERT(checknumber(rhs), "rhs is not a number");

    if (checknumber(lhs))
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
            VIA_ASSERT(rhs.val_number != 0.0f, "division by zero");
            return TValue(lhs.val_number / rhs.val_number);
        case OpCode::POW:
            return TValue(std::pow(lhs.val_number, rhs.val_number));
        case OpCode::MOD:
            return TValue(std::fmod(lhs.val_number, rhs.val_number));
        default:
            VIA_ASSERT(false, "non-arithmetic opcode");
            break;
        }

        return nil.clone();
    }
    else if (checktable(lhs))
    {
        const TValue &metamethod = getmetamethod(lhs, op);
        push(V, lhs); // self
        push(V, rhs); // other
        call(V, metamethod, 2);
        return pop(V);
    }

    VIA_ASSERT(false, "non-arithmetic lhs value");
    return nil.clone();
}

// Performs inline artihmetic between <lhs*> and <rhs>, operation determined by <op>.
VIA_FORCEINLINE void iarith(State *VIA_RESTRICT V, TValue *lhs, const TValue &rhs, OpCode op)
{
    if (checknumber(*lhs))
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
            VIA_ASSERT(rhs.val_number != 0.0f, "division by zero");

            lhs->val_number /= rhs.val_number;
            break;
        case OpCode::POW:
            lhs->val_number = std::pow(lhs->val_number, rhs.val_number);
            break;
        case OpCode::MOD:
            lhs->val_number = std::fmod(lhs->val_number, rhs.val_number);
            break;
        default:
            VIA_ASSERT(false, "non-artihmetic opcode");
            break;
        }
    }
    else if (checktable(*lhs))
    {
        const TValue &metamethod = getmetamethod(*lhs, op);
        push(V, *lhs); // self
        push(V, rhs);  // other
        call(V, metamethod, 2);
        *lhs = pop(V);
    }

    VIA_ASSERT(false, "non-arithmetic lhs value");
}

} // namespace via
