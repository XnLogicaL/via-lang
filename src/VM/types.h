/* This file is a part of the via programming language at
 * https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "Utils/modifiable_once.h"
#include "common.h"
#include "state.h"
#include "shared.h"

namespace via
{

using viaNumber = double;
using viaBool = bool;
using viaPtr = uintptr_t;
using viaCFunc = void (*)(viaState *);
// Type alias for viaHash
using viaTableKey = viaHash;

struct viaValue; // Forward declaration
struct viaTable; // Forward declaration

struct viaString
{
    const char *ptr;
    uint32_t len;
    uint32_t hash;
};

struct viaFunc
{
    const char *id;
    const viaInstruction *addr;
};

// Tagged union that holds a primitive via value
struct viaValue
{
    enum class __type : uint8_t
    {
        Number,
        Bool,
        String,
        Nil,
        Ptr,
        Func,
        CFunc,
        Table,
    };

    bool _const;
    __type type;

    union
    {
        viaNumber num;
        viaBool boole;
        viaPtr ptr;
        viaCFunc cfun;
        viaString *str;
        viaFunc *fun;
        viaTable *tbl;
    };

private:
    void cleanup();
};

struct viaTable
{
    viaTable *meta;
    util::modifiable_once<bool> frozen;
    viaHashMap<viaTableKey, viaValue> data;
};

using viaValueType = viaValue::__type;

// ! Random hashing algo, may need to be replaced later
inline uint32_t viaT_hashstring(viaState *, const char *s)
{
    uint32_t hash = 0;

    while (*s)
        hash = (hash * 31) + *s++;

    return hash;
}

inline viaString *viaT_newstring(viaState *V = nullptr, const char *s = "")
{
    // Retrieve the string table
    STable *stable = V->G->stable;

    // Compute the hash first
    uint32_t hash = viaT_hashstring(V, s);

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

inline viaTable *viaT_newtable(viaState *, viaTable *meta = nullptr, viaHashMap<viaTableKey, viaValue> ilist = {})
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

inline viaFunc *viaT_newfunc(viaState *, viaInstruction *addr = nullptr, const char *id = "<anonymous-function>")
{
    viaFunc *fn = new viaFunc;
    fn->id = id;
    fn->addr = addr;

    return fn;
}

inline viaValue *viaT_newvalue(viaState *, viaValueType ty)
{
    viaValue *val = new viaValue;
    val->type = ty;

    return val;
}

inline viaValue *viaT_newvalue(viaState *)
{
    viaValue *val = new viaValue;
    val->type = viaValueType::Nil;

    return val;
}

inline viaValue *viaT_newvalue(viaState *, viaNumber x)
{
    viaValue *val = new viaValue;
    val->type = viaValueType::Number;
    val->num = x;

    return val;
}

inline viaValue *viaT_newvalue(viaState *, viaBool b)
{
    viaValue *val = new viaValue;
    val->type = viaValueType::Bool;
    val->boole = b;

    return val;
}

inline viaValue *viaT_newvalue(viaState *, viaPtr p)
{
    viaValue *val = new viaValue;
    val->type = viaValueType::Ptr;
    val->ptr = p;

    return val;
}

inline viaValue *viaT_newvalue(viaState *, viaCFunc cf)
{
    viaValue *val = new viaValue;
    val->type = viaValueType::CFunc;
    val->boole = cf;

    return val;
}

inline viaValue *viaT_newvalue(viaState *V, const char *s)
{
    viaValue *val = new viaValue;
    val->type = viaValueType::String;
    val->str = viaT_newstring(V, s);

    return val;
}

inline viaValue *viaT_newvalue(viaState *, viaString *s)
{
    viaValue *val = new viaValue;
    val->type = viaValueType::String;
    val->str = s;

    return val;
}

inline viaValue *viaT_newvalue(viaState *, viaTable *t)
{
    viaValue *val = new viaValue;
    val->type = viaValueType::Table;
    val->tbl = t;

    return val;
}

inline viaValue *viaT_newvalue(viaState *, viaFunc *f)
{
    viaValue *val = new viaValue;
    val->type = viaValueType::Func;
    val->fun = f;

    return val;
}

inline viaValue viaT_stackvalue(viaState *, viaValueType ty)
{
    viaValue val;
    val.type = ty;

    return val;
}

inline viaValue viaT_stackvalue(viaState *)
{
    viaValue val;
    val.type = viaValueType::Nil;

    return val;
}

inline viaValue viaT_stackvalue(viaState *, viaNumber x)
{
    viaValue val;
    val.type = viaValueType::Number;
    val.num = x;

    return val;
}

inline viaValue viaT_stackvalue(viaState *, viaBool b)
{
    viaValue val;
    val.type = viaValueType::Bool;
    val.boole = b;

    return val;
}

inline viaValue viaT_stackvalue(viaState *, viaPtr p)
{
    viaValue val;
    val.type = viaValueType::Ptr;
    val.ptr = p;

    return val;
}

inline viaValue viaT_stackvalue(viaState *, viaCFunc cf)
{
    viaValue val;
    val.type = viaValueType::CFunc;
    val.boole = cf;

    return val;
}

inline viaValue viaT_stackvalue(viaState *V, const char *s)
{
    viaValue val;
    val.type = viaValueType::String;
    val.str = viaT_newstring(V, s);

    return val;
}

inline viaValue viaT_stackvalue(viaState *, viaString *s)
{
    viaValue val;
    val.type = viaValueType::String;
    val.str = s;

    return val;
}

inline viaValue viaT_stackvalue(viaState *, viaTable *t)
{
    viaValue val;
    val.type = viaValueType::Table;
    val.tbl = t;

    return val;
}

inline viaValue viaT_stackvalue(viaState *, viaFunc *f)
{
    viaValue val;
    val.type = viaValueType::Func;
    val.fun = f;

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

inline void viaT_cleanupfunc(viaState *, viaFunc *fn)
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
        viaT_cleanupstring(V, val->str);
        break;
    case viaValueType::Table:
        viaT_cleanuptable(V, val->tbl);
        break;
    case viaValueType::Func:
        viaT_cleanupfunc(V, val->fun);
        break;
    default:
        break;
    }

    delete val;
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

} // namespace via
