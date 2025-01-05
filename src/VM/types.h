/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "state.h"
#include "shared.h"
#include "Utils/modifiable_once.h"

namespace via
{

// Stores the (always) previous dynamically allocated value, for linking dynamic values
static TValue *__value_prev__ = nullptr;

using TNumber = double;
using TBool = bool;
// Basic pointer type, currently only used internally
using TPointer = uintptr_t;
// Type alias for Hash
using TableKey = Hash;

// Forward declarations
struct TValue;
struct TTable;
struct TString;
struct TFunction;
struct TCFunction;

enum class ValueType : uint8_t
{
    // Represents uninitialized values
    Monostate,
    Nil,
    Number,
    Bool,
    String,
    Ptr,
    Func,
    CFunc,
    Table,
};

// Tagged union that holds a primitive via value
struct TValue
{
    TValue *prev = nullptr;
    TValue *next = nullptr;
    ValueType type = ValueType::Monostate;
    union
    {
        TNumber val_number;
        TBool val_boolean;
        TPointer val_pointer;
        // These are pointers because their size is larger than 4 bytes,
        // which is what registers are supposed to hold
        TString *val_string;
        TFunction *val_function;
        TCFunction *val_cfunction;
        TTable *val_table;
    };
};

struct TString
{
    const char *ptr = nullptr;
    uint32_t len = 0;
    Hash hash = 0;
};

struct TFunction
{
    // Line information, determined during compile time,
    // Inherited from the instruction that declares this function
    size_t line = 0;
    // Tells the VM wether if this caller can handle an error
    bool error_handler = false;
    // Tells the VM if the last argument is a variadic argument
    bool is_vararg = false;
    // Function identifier
    const char *id = "<anonymous-function>";
    TFunction *caller = nullptr;
    Instruction *ret_addr = nullptr;
    std::vector<Instruction> bytecode = {};
};

struct TCFunction
{
    // Function pointer
    void (*ptr)(RTState *) = nullptr;
    // Tells the VM if the function can handle errors or not
    // This gets passed down to the replica function that represents this C functions stack frame
    // Pretty much only used for pcall
    bool error_handler = false;
};

struct TTable
{
    // Pointer to metatable
    TTable *meta = nullptr;
    // Tells the VM if the table is modifiable
    utils::modifiable_once<bool> frozen = false;
    HashMap<TableKey, TValue> data = {};
};

// ! Random hashing algo, may need to be replaced later
VIA_FORCEINLINE Hash hashstring(RTState *, const char *str)
{
    Hash hash = 0;
    // Automatically breaks when it hits the nullbyte, quite clever
    while (*str)
        hash = (hash * 31) + *str++;

    return hash;
}

VIA_FORCEINLINE TString *newstring(RTState *V = nullptr, const char *s = "")
{
    // Retrieve the string table
    STable *stable = V->G->stable;

    // Compute the hash first
    Hash hash = hashstring(V, s);

    // Check if the string already exists in the stable
    auto it = stable->find(hash);
    if (it != stable->end())
        // String already exists, return the existing entry
        return it->second;

    // Allocate and initialize a new TString
    TString *nstr = new TString();

    // Store the constant string length for later use
    size_t slen = std::strlen(s);
    // Dynamically allocated copy string
    char *sptr = new char[slen + 1];

    // Copy the constant string into the owned string
    std::strcpy(sptr, s);

    nstr->len = slen;
    nstr->ptr = sptr; // No need for static_cast<const char*>
    nstr->hash = hash;

    // Insert the new string into the stable
    (*stable)[hash] = nstr;

    return nstr;
}

VIA_FORCEINLINE TTable *newtable(RTState *, TTable *meta = nullptr, HashMap<TableKey, TValue> ilist = {}, bool frozen = false)
{
    // Allocate the table
    TTable *tbl = new TTable;
    // Set metadata and frozen state
    tbl->meta = meta;
    tbl->frozen = frozen;
    // Copy contents
    tbl->data = ilist;
    return tbl;
}

VIA_FORCEINLINE TFunction *newfunc(
    RTState *,
    const char *id,
    Instruction *ret_addr,
    std::vector<Instruction> bytecode,
    bool is_error_handler,
    bool is_var_arg
)
{
    TFunction *func = new TFunction;
    func->id = id;
    func->ret_addr = ret_addr;
    func->bytecode = bytecode;
    func->caller = nullptr;
    func->error_handler = is_error_handler;
    func->is_vararg = is_var_arg;
    func->line = std::numeric_limits<size_t>::max();
    return func;
}

VIA_FORCEINLINE TCFunction *newcfunc(RTState *, void (*ptr)(RTState *), bool is_error_handler)
{
    TCFunction *cfunc = new TCFunction;
    cfunc->ptr = ptr;
    cfunc->error_handler = is_error_handler;
    return cfunc;
}

VIA_FORCEINLINE TValue *newvalue(RTState *, ValueType ty)
{
    TValue *val = new TValue;
    val->type = ty;
    // Link with the previous value
    val->prev = __value_prev__;

    // Link the previous value with this value
    if (__value_prev__ != nullptr)
        __value_prev__->next = val;

    // Overwrite the previous value
    __value_prev__ = val;

    return val;
}

VIA_FORCEINLINE TValue *newvalue(RTState *V)
{
    TValue *val = newvalue(V, ValueType::Nil);
    return val;
}

VIA_FORCEINLINE TValue *newvalue(RTState *V, TNumber x)
{
    TValue *val = newvalue(V, ValueType::Number);
    val->val_number = x;
    return val;
}

VIA_FORCEINLINE TValue *newvalue(RTState *V, TBool b)
{
    TValue *val = newvalue(V, ValueType::Bool);
    val->val_boolean = b;

    return val;
}

VIA_FORCEINLINE TValue *newvalue(RTState *V, TPointer p)
{
    TValue *val = newvalue(V, ValueType::Ptr);
    val->val_pointer = p;

    return val;
}

VIA_FORCEINLINE TValue *newvalue(RTState *V, TCFunction *cf)
{
    TValue *val = newvalue(V, ValueType::CFunc);
    val->val_cfunction = cf;

    return val;
}

VIA_FORCEINLINE TValue *newvalue(RTState *V, void (*cf)(RTState *))
{
    TValue *val = newvalue(V, ValueType::CFunc);
    val->val_cfunction = new TCFunction{cf, false};

    return val;
}

template<typename T>
    requires std::same_as<T, const char *>
VIA_FORCEINLINE TValue *newvalue(RTState *V, T s)
{
    TValue *val = newvalue(V, ValueType::String);
    val->val_string = newstring(V, s);

    return val;
}

VIA_FORCEINLINE TValue *newvalue(RTState *V, TString *s)
{
    TValue *val = newvalue(V, ValueType::String);
    val->val_string = s;

    return val;
}

VIA_FORCEINLINE TValue *newvalue(RTState *V, TTable *t)
{
    TValue *val = newvalue(V, ValueType::Table);
    val->val_table = t;

    return val;
}

VIA_FORCEINLINE TValue *newvalue(RTState *V, TFunction *f)
{
    TValue *val = newvalue(V, ValueType::Func);
    val->val_function = f;

    return val;
}

VIA_FORCEINLINE TValue stackvalue(RTState *, ValueType ty)
{
    TValue val;
    val.type = ty;


    return val;
}

VIA_FORCEINLINE TValue stackvalue(RTState *V)
{
    TValue val = stackvalue(V, ValueType::Nil);
    return val;
}

VIA_FORCEINLINE TValue stackvalue(RTState *V, TNumber x)
{
    TValue val = stackvalue(V, ValueType::Number);
    val.val_number = x;

    return val;
}

VIA_FORCEINLINE TValue stackvalue(RTState *V, TBool b)
{
    TValue val = stackvalue(V, ValueType::Bool);
    val.val_boolean = b;

    return val;
}

VIA_FORCEINLINE TValue stackvalue(RTState *V, TPointer p)
{
    TValue val = stackvalue(V, ValueType::Ptr);
    val.val_pointer = p;

    return val;
}

VIA_FORCEINLINE TValue stackvalue(RTState *V, TCFunction *cf)
{
    TValue val = stackvalue(V, ValueType::CFunc);
    val.val_cfunction = cf;

    return val;
}

template<typename T>
    requires std::same_as<T, void (*)(RTState *)>
VIA_FORCEINLINE TValue stackvalue(RTState *V, T cf)
{
    TValue val = stackvalue(V, ValueType::CFunc);
    val.val_cfunction = new TCFunction{cf, false};

    return val;
}

template<typename T>
    requires std::same_as<T, const char *>
VIA_FORCEINLINE TValue stackvalue(RTState *V, T s)
{
    TValue val = stackvalue(V, ValueType::String);
    val.val_string = newstring(V, s);

    return val;
}

VIA_FORCEINLINE TValue stackvalue(RTState *V, TString *s)
{
    TValue val = stackvalue(V, ValueType::String);
    val.val_string = s;

    return val;
}

VIA_FORCEINLINE TValue stackvalue(RTState *V, TTable *t)
{
    TValue val = stackvalue(V, ValueType::Table);
    val.val_table = t;

    return val;
}

VIA_FORCEINLINE TValue stackvalue(RTState *V, TFunction *f)
{
    TValue val = stackvalue(V, ValueType::Func);
    val.val_function = f;

    return val;
}

VIA_FORCEINLINE void cleanupstring(RTState *V, TString *str)
{
    STable *stable = V->G->stable;
    uint32_t hash = str->hash;

    auto it = stable->find(hash);

    // Check if the string is interned
    if (it != stable->end())
        // If so, remove it from the STable
        stable->erase(hash);

    // Cleanup the owned char *
    delete[] str->ptr;
    // Cleanup the object
    delete str;
}

VIA_FORCEINLINE void cleanuptable(RTState *, TTable *tbl)
{
    tbl->data.clear();
    delete tbl;
}

VIA_FORCEINLINE void cleanupfunc(RTState *, TFunction *fn)
{
    // I know a function just for this is unnecessary
    // The idea is, the user may not know how to cleanup an object that is instantiated by a wrapper (newfunc)
    // And this function is just a wrapper for deleting it
    delete fn;
}

// Cleans up a dynamically allocated (specifically heap allocated) TValue object
// ! Passing a stack-allocated TValue object will result in undefined behavior
VIA_FORCEINLINE void cleanupval(RTState *V, TValue *val)
{
    // Cleanup underlying type, if present
    switch (val->type)
    {
    case ValueType::String:
        cleanupstring(V, val->val_string);
        break;
    case ValueType::Table:
        cleanuptable(V, val->val_table);
        break;
    case ValueType::Func:
        cleanupfunc(V, val->val_function);
        break;
    default:
        break;
    }

    delete val;
}

VIA_FORCEINLINE bool checkmonostate(RTState *, TValue val)
{
    return val.type == ValueType::Monostate;
}

VIA_FORCEINLINE bool checknumber(RTState *, TValue val)
{
    return val.type == ValueType::Number;
}

VIA_FORCEINLINE bool checkbool(RTState *, TValue val)
{
    return val.type == ValueType::Bool;
}

VIA_FORCEINLINE bool checknil(RTState *, TValue val)
{
    return val.type == ValueType::Nil;
}

VIA_FORCEINLINE bool checkptr(RTState *, TValue val)
{
    return val.type == ValueType::Ptr;
}

VIA_FORCEINLINE bool checkstring(RTState *, TValue val)
{
    return val.type == ValueType::String;
}

VIA_FORCEINLINE bool checktable(RTState *, TValue val)
{
    return val.type == ValueType::Table;
}

VIA_FORCEINLINE bool checkcfunction(RTState *, TValue val)
{
    return val.type == ValueType::CFunc;
}

VIA_FORCEINLINE bool checkfunction(RTState *, TValue val)
{
    return val.type == ValueType::Func;
}

VIA_FORCEINLINE bool checkempty(RTState *V, TValue val)
{
    return checknil(V, val) || checkmonostate(V, val);
}

VIA_FORCEINLINE bool checkcallable(RTState *V, TValue val)
{
    return checkfunction(V, val) || checkcfunction(V, val);
}

VIA_FORCEINLINE bool checksubscriptable(RTState *V, TValue val)
{
    return checktable(V, val) || checkstring(V, val);
}

} // namespace via
