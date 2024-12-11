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
// Type alias for viaHash_t
using viaTableKey = viaHash_t;

// Forward declarations
struct viaValue;
struct viaTable;
struct viaString;
struct viaFunction;
struct viaCFunction;

enum class viaValueType : uint8_t
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
    viaValueType type = viaValueType::Monostate;
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
    viaRawString_t ptr = nullptr;
    uint32_t len = 0;
    viaHash_t hash = 0;
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
    viaRawString_t id = "<anonymous-function>";
    viaFunction *caller = nullptr;
    std::unordered_map<viaVariableIdentifier_t, viaValue> locals = {};
    std::vector<viaInstruction> bytecode = {};
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
    viaHashMap_t<viaTableKey, viaValue> data = {};
};

// ! Random hashing algo, may need to be replaced later
inline viaHash_t viaT_hashstring(viaState *, viaRawString_t str)
{
    viaHash_t hash = 0;

    // Automatically breaks when it hits the nullbyte, quite clever
    while (*str)
        hash = (hash * 31) + *str++;

    return hash;
}

inline viaString *viaT_newstring(viaState *V = nullptr, viaRawString_t s = "")
{
    // Retrieve the string table
    viaSTable *stable = V->G->stable;

    // Compute the hash first
    viaHash_t hash = viaT_hashstring(V, s);

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

inline viaTable *viaT_newtable(viaState *, viaTable *meta = nullptr, viaHashMap_t<viaTableKey, viaValue> ilist = {})
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

inline viaValue *viaT_newvalue(viaState *, viaValueType ty)
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
    viaValue *val = viaT_newvalue(V, viaValueType::Nil);
    return val;
}

inline viaValue *viaT_newvalue(viaState *V, viaNumber x)
{
    viaValue *val = viaT_newvalue(V, viaValueType::Number);
    val->val_number = x;
    return val;
}

inline viaValue *viaT_newvalue(viaState *V, viaBool b)
{
    viaValue *val = viaT_newvalue(V, viaValueType::Bool);
    val->val_boolean = b;

    return val;
}

inline viaValue *viaT_newvalue(viaState *V, viaPointer p)
{
    viaValue *val = viaT_newvalue(V, viaValueType::Ptr);
    val->val_pointer = p;

    return val;
}

inline viaValue *viaT_newvalue(viaState *V, viaCFunction *cf)
{
    viaValue *val = viaT_newvalue(V, viaValueType::CFunc);
    val->val_cfunction = cf;

    return val;
}

inline viaValue *viaT_newvalue(viaState *V, void (*cf)(viaState *))
{
    viaValue *val = viaT_newvalue(V, viaValueType::CFunc);
    val->val_cfunction = new viaCFunction{cf, false};

    return val;
}

template<typename T>
    requires std::same_as<T, viaRawString_t>
inline viaValue *viaT_newvalue(viaState *V, T s)
{
    viaValue *val = viaT_newvalue(V, viaValueType::String);
    val->val_string = viaT_newstring(V, s);

    return val;
}

inline viaValue *viaT_newvalue(viaState *V, viaString *s)
{
    viaValue *val = viaT_newvalue(V, viaValueType::String);
    val->val_string = s;

    return val;
}

inline viaValue *viaT_newvalue(viaState *V, viaTable *t)
{
    viaValue *val = viaT_newvalue(V, viaValueType::Table);
    val->val_table = t;

    return val;
}

inline viaValue *viaT_newvalue(viaState *V, viaFunction *f)
{
    viaValue *val = viaT_newvalue(V, viaValueType::Func);
    val->val_function = f;

    return val;
}

inline viaValue viaT_stackvalue(viaState *, viaValueType ty)
{
    viaValue val;
    val.type = ty;


    return val;
}

inline viaValue viaT_stackvalue(viaState *V)
{
    viaValue val = viaT_stackvalue(V, viaValueType::Nil);
    return val;
}

inline viaValue viaT_stackvalue(viaState *V, viaNumber x)
{
    viaValue val = viaT_stackvalue(V, viaValueType::Number);
    val.val_number = x;

    return val;
}

inline viaValue viaT_stackvalue(viaState *V, viaBool b)
{
    viaValue val = viaT_stackvalue(V, viaValueType::Bool);
    val.val_boolean = b;

    return val;
}

inline viaValue viaT_stackvalue(viaState *V, viaPointer p)
{
    viaValue val = viaT_stackvalue(V, viaValueType::Ptr);
    val.val_pointer = p;

    return val;
}

inline viaValue viaT_stackvalue(viaState *V, viaCFunction *cf)
{
    viaValue val = viaT_stackvalue(V, viaValueType::CFunc);
    val.val_cfunction = cf;

    return val;
}

template<typename T>
    requires std::same_as<T, void (*)(viaState *)>
inline viaValue viaT_stackvalue(viaState *V, T cf)
{
    viaValue val = viaT_stackvalue(V, viaValueType::CFunc);
    val.val_cfunction = new viaCFunction{cf, false};

    return val;
}

template<typename T>
    requires std::same_as<T, viaRawString_t>
inline viaValue viaT_stackvalue(viaState *V, T s)
{
    viaValue val = viaT_stackvalue(V, viaValueType::String);
    val.val_string = viaT_newstring(V, s);

    return val;
}

inline viaValue viaT_stackvalue(viaState *V, viaString *s)
{
    viaValue val = viaT_stackvalue(V, viaValueType::String);
    val.val_string = s;

    return val;
}

inline viaValue viaT_stackvalue(viaState *V, viaTable *t)
{
    viaValue val = viaT_stackvalue(V, viaValueType::Table);
    val.val_table = t;

    return val;
}

inline viaValue viaT_stackvalue(viaState *V, viaFunction *f)
{
    viaValue val = viaT_stackvalue(V, viaValueType::Func);
    val.val_function = f;

    return val;
}

inline void viaT_cleanupstring(viaState *V, viaString *str)
{
    viaSTable *stable = V->G->stable;
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
    case viaValueType::String:
        viaT_cleanupstring(V, val->val_string);
        break;
    case viaValueType::Table:
        viaT_cleanuptable(V, val->val_table);
        break;
    case viaValueType::Func:
        viaT_cleanupfunc(V, val->val_function);
        break;
    default:
        break;
    }

    delete val;
}

inline bool viaT_checkmonostate(viaState *, viaValue val)
{
    return val.type == viaValueType::Monostate;
}

inline bool viaT_checknumber(viaState *, viaValue val)
{
    return val.type == viaValueType::Number;
}

inline bool viaT_checkbool(viaState *, viaValue val)
{
    return val.type == viaValueType::Bool;
}

inline bool viaT_checknil(viaState *, viaValue val)
{
    return val.type == viaValueType::Nil;
}

inline bool viaT_checkptr(viaState *, viaValue val)
{
    return val.type == viaValueType::Ptr;
}

inline bool viaT_checkstring(viaState *, viaValue val)
{
    return val.type == viaValueType::String;
}

inline bool viaT_checktable(viaState *, viaValue val)
{
    return val.type == viaValueType::Table;
}

inline bool viaT_checkcfunction(viaState *, viaValue val)
{
    return val.type == viaValueType::CFunc;
}

inline bool viaT_checkfunction(viaState *, viaValue val)
{
    return val.type == viaValueType::Func;
}

inline bool viaT_checkempty(viaState *V, viaValue val)
{
    return viaT_checknil(V, val) || viaT_checkmonostate(V, val);
}

} // namespace via
