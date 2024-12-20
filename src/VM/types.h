/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "Utils/modifiable_once.h"
#include "common.h"
#include "state.h"
#include "shared.h"

#define VIA_MAX_CONSTS (256)

namespace via
{

// Stores the (always) previous dynamically allocated value, for linking dynamic values
static viaValue *__value_prev__ = nullptr;

using viaNumber = double;
using viaBool = bool;
// Basic pointer type, currently only used internally
using viaPointer = uintptr_t;
// Type alias for Hash
using TableKey = Hash;

// Forward declarations
struct viaValue;
struct viaTable;
struct viaString;
struct viaFunction;
struct viaCFunction;

enum class ValueType : uint8_t
{
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
struct viaValue
{
    viaValue *prev = nullptr;
    viaValue *next = nullptr;
    ValueType type = ValueType::Monostate;
    union
    {
        viaNumber val_number;
        viaBool val_boolean;
        viaPointer val_pointer;
        // These are pointers because their size is larger than 4 bytes,
        // which is what registers are supposed to hold
        viaString *val_string;
        viaFunction *val_function;
        viaCFunction *val_cfunction;
        viaTable *val_table;
    };
};

struct viaString
{
    const char *ptr = nullptr;
    uint32_t len = 0;
    Hash hash = 0;
};

struct viaFunction
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
    viaFunction *caller = nullptr;
    std::unordered_map<VarId, viaValue> locals = {};
    std::vector<Instruction> bytecode = {};
};

struct viaCFunction
{
    // Function pointer
    void (*ptr)(viaState *) = nullptr;
    // Tells the VM if the function can handle errors or not
    // This gets passed down to the replica function that represents this C functions stack frame
    // Pretty much only used for pcall
    bool error_handler = false;
};

struct viaTable
{
    // Pointer to metatable
    viaTable *meta = nullptr;
    // Tells the VM if the table is modifiable
    util::modifiable_once<bool> frozen = false;
    HashMap<TableKey, viaValue> data = {};
};

// ! Random hashing algo, may need to be replaced later
inline Hash viaT_hashstring(viaState *, const char *str)
{
    Hash hash = 0;

    // Automatically breaks when it hits the nullbyte, quite clever
    while (*str)
        hash = (hash * 31) + *str++;

    return hash;
}

inline viaString *viaT_newstring(viaState *V = nullptr, const char *s = "")
{
    // Retrieve the string table
    STable *stable = V->G->stable;

    // Compute the hash first
    Hash hash = viaT_hashstring(V, s);

    // Check if the string already exists in the stable
    auto it = stable->find(hash);
    if (it != stable->end())
        // String already exists, return the existing entry
        return it->second;

    // Allocate and initialize a new viaString
    viaString *nstr = new viaString();

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

inline viaTable *viaT_newtable(viaState *, viaTable *meta = nullptr, HashMap<TableKey, viaValue> ilist = {})
{
    // Allocate the table
    viaTable *tbl = new viaTable;

    // Set metadata and frozen state
    tbl->meta = meta;
    tbl->frozen = util::modifiable_once<bool>(false);
    // Copy contents
    tbl->data = ilist;

    return tbl;
}

inline viaFunction *viaT_newfunc(viaState *)
{
    // TODO
    return nullptr;
}

inline viaValue *viaT_newvalue(viaState *, ValueType ty)
{
    viaValue *val = new viaValue;
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

inline viaValue *viaT_newvalue(viaState *V)
{
    viaValue *val = viaT_newvalue(V, ValueType::Nil);
    return val;
}

inline viaValue *viaT_newvalue(viaState *V, viaNumber x)
{
    viaValue *val = viaT_newvalue(V, ValueType::Number);
    val->val_number = x;
    return val;
}

inline viaValue *viaT_newvalue(viaState *V, viaBool b)
{
    viaValue *val = viaT_newvalue(V, ValueType::Bool);
    val->val_boolean = b;

    return val;
}

inline viaValue *viaT_newvalue(viaState *V, viaPointer p)
{
    viaValue *val = viaT_newvalue(V, ValueType::Ptr);
    val->val_pointer = p;

    return val;
}

inline viaValue *viaT_newvalue(viaState *V, viaCFunction *cf)
{
    viaValue *val = viaT_newvalue(V, ValueType::CFunc);
    val->val_cfunction = cf;

    return val;
}

inline viaValue *viaT_newvalue(viaState *V, void (*cf)(viaState *))
{
    viaValue *val = viaT_newvalue(V, ValueType::CFunc);
    val->val_cfunction = new viaCFunction{cf, false};

    return val;
}

template<typename T>
    requires std::same_as<T, const char *>
inline viaValue *viaT_newvalue(viaState *V, T s)
{
    viaValue *val = viaT_newvalue(V, ValueType::String);
    val->val_string = viaT_newstring(V, s);

    return val;
}

inline viaValue *viaT_newvalue(viaState *V, viaString *s)
{
    viaValue *val = viaT_newvalue(V, ValueType::String);
    val->val_string = s;

    return val;
}

inline viaValue *viaT_newvalue(viaState *V, viaTable *t)
{
    viaValue *val = viaT_newvalue(V, ValueType::Table);
    val->val_table = t;

    return val;
}

inline viaValue *viaT_newvalue(viaState *V, viaFunction *f)
{
    viaValue *val = viaT_newvalue(V, ValueType::Func);
    val->val_function = f;

    return val;
}

inline viaValue viaT_stackvalue(viaState *, ValueType ty)
{
    viaValue val;
    val.type = ty;


    return val;
}

inline viaValue viaT_stackvalue(viaState *V)
{
    viaValue val = viaT_stackvalue(V, ValueType::Nil);
    return val;
}

inline viaValue viaT_stackvalue(viaState *V, viaNumber x)
{
    viaValue val = viaT_stackvalue(V, ValueType::Number);
    val.val_number = x;

    return val;
}

inline viaValue viaT_stackvalue(viaState *V, viaBool b)
{
    viaValue val = viaT_stackvalue(V, ValueType::Bool);
    val.val_boolean = b;

    return val;
}

inline viaValue viaT_stackvalue(viaState *V, viaPointer p)
{
    viaValue val = viaT_stackvalue(V, ValueType::Ptr);
    val.val_pointer = p;

    return val;
}

inline viaValue viaT_stackvalue(viaState *V, viaCFunction *cf)
{
    viaValue val = viaT_stackvalue(V, ValueType::CFunc);
    val.val_cfunction = cf;

    return val;
}

template<typename T>
    requires std::same_as<T, void (*)(viaState *)>
inline viaValue viaT_stackvalue(viaState *V, T cf)
{
    viaValue val = viaT_stackvalue(V, ValueType::CFunc);
    val.val_cfunction = new viaCFunction{cf, false};

    return val;
}

template<typename T>
    requires std::same_as<T, const char *>
inline viaValue viaT_stackvalue(viaState *V, T s)
{
    viaValue val = viaT_stackvalue(V, ValueType::String);
    val.val_string = viaT_newstring(V, s);

    return val;
}

inline viaValue viaT_stackvalue(viaState *V, viaString *s)
{
    viaValue val = viaT_stackvalue(V, ValueType::String);
    val.val_string = s;

    return val;
}

inline viaValue viaT_stackvalue(viaState *V, viaTable *t)
{
    viaValue val = viaT_stackvalue(V, ValueType::Table);
    val.val_table = t;

    return val;
}

inline viaValue viaT_stackvalue(viaState *V, viaFunction *f)
{
    viaValue val = viaT_stackvalue(V, ValueType::Func);
    val.val_function = f;

    return val;
}

inline void viaT_cleanupstring(viaState *V, viaString *str)
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

inline void viaT_cleanuptable(viaState *, viaTable *tbl)
{
    tbl->data.clear();
    delete tbl;
}

inline void viaT_cleanupfunc(viaState *, viaFunction *fn)
{
    // I know a function just for this is unnecessary
    // The idea is, the user may not know how to cleanup an object that is instantiated by a wrapper (viaT_newfunc)
    // And this function is just a wrapper for deleting it
    delete fn;
}

// Cleans up a dynamically allocated (specifically heap allocated) viaValue object
// ! Passing a stack-allocated viaValue object will result in undefined behavior
inline void viaT_cleanupval(viaState *V, viaValue *val)
{
    // Cleanup underlying type, if present
    switch (val->type)
    {
    case ValueType::String:
        viaT_cleanupstring(V, val->val_string);
        break;
    case ValueType::Table:
        viaT_cleanuptable(V, val->val_table);
        break;
    case ValueType::Func:
        viaT_cleanupfunc(V, val->val_function);
        break;
    default:
        break;
    }

    delete val;
}

inline bool viaT_checkmonostate(viaState *, viaValue val)
{
    return val.type == ValueType::Monostate;
}

inline bool viaT_checknumber(viaState *, viaValue val)
{
    return val.type == ValueType::Number;
}

inline bool viaT_checkbool(viaState *, viaValue val)
{
    return val.type == ValueType::Bool;
}

inline bool viaT_checknil(viaState *, viaValue val)
{
    return val.type == ValueType::Nil;
}

inline bool viaT_checkptr(viaState *, viaValue val)
{
    return val.type == ValueType::Ptr;
}

inline bool viaT_checkstring(viaState *, viaValue val)
{
    return val.type == ValueType::String;
}

inline bool viaT_checktable(viaState *, viaValue val)
{
    return val.type == ValueType::Table;
}

inline bool viaT_checkcfunction(viaState *, viaValue val)
{
    return val.type == ValueType::CFunc;
}

inline bool viaT_checkfunction(viaState *, viaValue val)
{
    return val.type == ValueType::Func;
}

inline bool viaT_checkempty(viaState *V, viaValue val)
{
    return viaT_checknil(V, val) || viaT_checkmonostate(V, val);
}

inline bool viaT_checkcallable(viaState *V, viaValue val)
{
    return viaT_checkfunction(V, val) || viaT_checkcfunction(V, val);
}

inline bool viaT_checksubscriptable(viaState *V, viaValue val)
{
    return viaT_checktable(V, val) || viaT_checkstring(V, val);
}

} // namespace via
