/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "bytecode.h"
#include "core.h"
#include "except.h"
#include "gc.h"
#include "global.h"
#include "opcode.h"
#include "register.h"
#include "shared.h"
#include "stack.h"
#include "state.h"
#include "types.h"
#include <string.h>
#include <type_traits>

namespace via
{

inline viaGlobalState *via_newgstate()
{
    viaGlobalState *G = new viaGlobalState;

    G->global = new Global();
    G->stable = new std::unordered_map<viaTableKey, viaString *>();

    return G;
}

// Initializes and returns a new viaState object
inline viaState *via_newstate(const std::vector<viaInstruction> &pipeline)
{
    viaState *V = new viaState;

    V->id = __thread_id++;

    V->G = via_newgstate();

    V->ihp = new viaInstruction[pipeline.size()]; // Allocate ihp    (Instruction head pointer)
    V->ibp = V->ihp + pipeline.size();            // Initialize ibp  (Instruction base pointer)
    V->ip = V->ihp;                               // Initialize ip   (Instruction pointer)

    // Copy instructions into the instruction pipeline
    std::copy(pipeline.begin(), pipeline.end(), V->ip);

    V->stack = new Stack<StackFrame>();
    V->labels = new std::unordered_map<std::string_view, viaInstruction *>();
    V->ralloc = new RegisterAllocator();
    V->gc = new viaGCState();
    // Initialize stack, set return address to the end of the pipeline to ensure if the frame is popped the program exits immediately
    V->stack->push(StackFrame(V->ibp, V->gc));

    V->exitc = 0;
    V->exitm = "";

    V->abrt = false;
    V->skip = false;
    V->yield = false;
    V->restorestate = false;

    V->yieldfor = 0.0f;
    V->argc = 0;

    V->ts = viaThreadState::PAUSED;
    V->sstate = nullptr;

    return V;
}

inline void via_cleanupgstate(viaGlobalState *G)
{
    delete G->global;
    delete G->stable;
    delete G;
}

inline void via_cleanupstate(viaState *V)
{
    via_cleanupgstate(V->G);

    // This automatically invalidates both ip and ibp
    // No need to clean them up seperately
    delete V->ihp;
    delete V->stack;
    delete V->labels;
    delete V->ralloc;

    viaGC_cleanup(V->gc);
}

// Manually sets the VM exit data
// ! Internal usage only, made public for libraries
inline void via_setexitdata(viaState *V, viaExitCode exitc, const std::string &exitm) noexcept
{
    V->exitc = exitc;
    // I don't know why but `strdup` is deprecated but `_strdup` isn't
    // This is the completely opposite of the deprecation convention, but sure...
    V->exitm = _strdup(exitm.c_str());
    return;
}

inline bool via_validjmpaddr(viaState *V, const viaInstruction *addr) noexcept
{
    return (addr >= V->ihp) && (addr <= V->ibp);
}

inline void via_jmpto(viaState *V, const viaInstruction *addr)
{
    if (!via_validjmpaddr(V, addr))
    {
        via_setexitdata(V, 1, "Illegal jump: jump address out of bounds");
        V->abrt = true;
        return;
    }

    V->ip = const_cast<viaInstruction *>(addr);
    return;
}

// Jumps a given offset
inline void via_jmp(viaState *V, viaJmpOffset offset)
{
    viaInstruction *addr = V->ip + offset;
    via_jmpto(V, addr);
    return;
}

// Appends a pointer to the garbage collector free list
// The pointer should be malloc-ed, otherwise this has undefined behavior
// ! The value cannot be removed from the free list!
template<typename T>
inline void via_gcadd(viaState *V, T p) noexcept
{
    viaGC_add(V->gc, p);
    return;
}

// Invokes garbage collection
// Should be called in intervals
inline void via_gccol(viaState *V) noexcept
{
    viaGC_collect(V->gc);
    return;
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
    return;
}

//* viaRegister operations

// Sets register <R> to the given value <v>
template<typename T = viaValue>
inline void via_setregister(viaState *V, viaRegister R, T v) noexcept
{
    VIA_ASSERT(!std::is_pointer<T>::value, "via_setregister(): Expected non-pointer value to assign register");

    *V->ralloc->get<T>(R) = v;
    return;
}

// Returns the value of register <R>
template<typename T = viaValue>
inline constexpr T *via_getregister(viaState *V, viaRegister R) noexcept
{
    T *addr = V->ralloc->get<T>(R);
    return addr;
}

// Compares the value of two registers and returns wether if they are equivalent
inline constexpr bool via_cmpregister(viaState *V, viaRegister R0, viaRegister R1) noexcept
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
    case viaValueType::Number:
        return v0.num == v1.num;
    case viaValueType::Bool:
        return v0.boole == v1.boole;
    case viaValueType::String:
        if (v0.str->ptr && v1.str->ptr)
        {
            if (strlen(v0.str->ptr) != strlen(v1.str->ptr))
                return false;
            return !strcmp(v0.str->ptr, v1.str->ptr);
        }
        return v0.str->ptr == v1.str->ptr;
    case viaValueType::Nil:
        return true; // Nil values are always equal
    case viaValueType::Ptr:
        return v0.ptr == v1.ptr;
    default:
        return false; // Unique objects (CFunc, Func, viaTable, viaTableKey) are never equal
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
    case viaValueType::Bool:
        return V0.boole == V1.boole;
    case viaValueType::Number:
        return V0.num == V1.num;
    case viaValueType::Nil:
        // Nil values are always equal
        return true;
    case viaValueType::String:
        return !strcmp(V0.str->ptr, V1.str->ptr);
    case viaValueType::Ptr:
        return V0.ptr == V1.ptr;
    case viaValueType::Func:
        return V0.fun == V1.fun;
    case viaValueType::CFunc:
        return V0.cfun == V1.cfun;
    default:
        // Unique objects (such as table) are never equal
        return false;
    }

    return false;
};

//* Stack/Global operations

// Creates a new global variable with identifier <k> and value <v>
// Terminates the VM if the identifier already exists in the global enviornment
inline void via_setglobal(viaState *V, viaGlobalIdentifier id, viaValue v) noexcept
{
    // Since there's no way for the `set_global` function to signal an error
    // I made it so that it returns a success code, which is a "C style" (reversed) boolean
    int success = V->G->global->set_global(V, id, v);

    if (success != 0)
    {
        via_setexitdata(V, 1, std::format("Global '{}' already exists", id));
        V->abrt = true;
    }
}

// Returns the value of identifier <k> if found in the global enviornment
inline viaValue *via_getglobal(viaState *V, viaGlobalIdentifier id) noexcept
{
    if (id.length() == 0)
    {
        via_setexitdata(V, 1, "Invalid global identifier");
        V->abrt = true;
        return viaT_newvalue(V);
    }

    return V->G->global->get_global(V, id);
}

// Loads and returns the value of <k> into register <R> if it's found in the global enviornment
inline void via_loadglobal(viaState *V, viaGlobalIdentifier id, viaRegister R) noexcept
{
    viaValue *val = via_getglobal(V, id);
    via_setregister(V, R, val);
    return;
}

// Sets the value of <id> to value <v> in the current stack frame
inline void via_setlocal(viaState *V, viaLocalIdentifier id, viaValue val)
{
    V->stack->top().set_local(V, id, val);
    return;
}

// Returns the value of <id> if found in the current stack frame
inline viaValue *via_getlocal(viaState *V, viaLocalIdentifier id)
{
    return V->stack->top().get_local(V, id);
}

// Loads and returns the value of <id> into register <R> if found in the current stack frame
inline viaValue *via_loadlocal(viaState *V, viaLocalIdentifier id, viaRegister R) noexcept
{
    viaValue *val = via_getlocal(V, id);
    via_setregister(V, R, val);
    return val;
}

//* viaValue operations

// Returns a value that contains a String that represents the stringified version of <v>
// ! The return value is guaranteed to be a String
inline viaValue &via_tostring(viaState *V, viaValue &val)
{
    if (val.type == viaValueType::String)
        return val;

    switch (val.type)
    {
    case viaValueType::Number:
    {
        viaString *str = viaT_newstring(V, std::to_string(val.num).c_str());
        val.str = str;
        break;
    }
    case viaValueType::Bool:
    {
        viaString *str = viaT_newstring(V, val.boole ? "true" : "false");
        val.str = str;
        break;
    }
    case viaValueType::Table:
    {
        std::string str = "{";

        for (auto elem : val.tbl->data)
        {
            str += via_tostring(V, elem.second).str->ptr;
            str += ", ";
        }

        if (str.back() == ' ')
            str += "\b\b";

        str += "}";

        val.str = viaT_newstring(V, str.c_str());
        break;
    }
    case viaValueType::Func:
    {
        const void *faddr = val.fun;
        viaString *str = viaT_newstring(V, std::format("function {}", faddr).c_str());
        val.str = str;
        break;
    }
    case viaValueType::CFunc:
    {
        // This has to be explicitly casted because function pointers be weird
        const void *cfaddr = reinterpret_cast<const void *>(val.cfun);
        viaString *str = viaT_newstring(V, std::format("cfunction {}", cfaddr).c_str());
        val.str = str;
        break;
    }
    default:
        val.str = viaT_newstring(V, "nil");
        break;
    }

    val.type = viaValueType::String;
    return val;
}

// Returns the truthiness of value <v>
// ! Guaranteed to be a Bool
inline viaValue &via_tobool(viaState *, viaValue &val)
{
    if (val.type == viaValueType::Bool)
        return val;

    switch (val.type)
    {
    case viaValueType::Nil:
        val.boole = false;
        break;
    default:
        val.boole = true;
        break;
    }

    val.type = viaValueType::Bool;
    return val;
}

// Returns the number representation of value <v>
// ! Returns Nil if impossible, unlike `vtostring` or `vtobool`
inline viaValue &via_tonumber(viaState *, viaValue &val)
{
    if (val.type == viaValueType::Number)
        return val;

    switch (val.type)
    {
    case viaValueType::String:
        val.num = std::stod(val.str->ptr);
        break;
    case viaValueType::Bool:
        val.num = val.boole ? 1.0f : 0.0f;
        break;
    default:
        return val;
    }

    val.type = viaValueType::Number;
    return val;
}

// Utility function for quick table indexing
// Returns the value of key <k> if present in table <t>
inline viaValue *via_gettableindex(viaState *V, viaTable *T, viaTableKey key, bool search_meta)
{
    auto it = T->data.find(key);

    if (it != T->data.end())
        return &it->second;
    else if (search_meta && T->meta)
        // Disable metatable search to prevent chain searching
        // Which can cause infinite loops, crashes among other bugs
        return via_gettableindex(V, T->meta, key, false);

    // This has to be a pointer because we don't know if the value is a non-pointer primitive type
    return viaT_newvalue(V);
}

// Assigns the given value <v> to key <k> in table <t>
inline void via_settableindex(viaState *V, viaTable *T, viaTableKey key, viaValue v)
{
    if (v.type == viaValueType::Nil)
    {
        if (via_gettableindex(V, T, key, false)->type != viaValueType::Nil)
            T->data.erase(key);

        return;
    }

    T->data[key] = v;
    return;
}

// Calls a Func type, terminates if uncallable
// Uses the FASTCALL convention
// Arguments and return values are loaded onto registers of those respective types
// For example, argument register #1 would be;
// { RegisterType::AR, 0 }
// This allows for extremely fast function calls and an alternative to stack based calling conventions
inline void via_callf(viaState *V, viaFunc *f)
{
    if (!via_validjmpaddr(V, f->addr))
    {
        via_setexitdata(V, 1, "Invalid function jump address");
        V->abrt = true;
        return;
    }

    if (f->addr->op != OpCode::FUNC)
    {
        via_setexitdata(V, 1, "Function jump address points to non-function opcode");
        V->abrt = true;
        return;
    }

    V->stack->push(StackFrame(V->ip, V->gc));
    V->ralloc->flush(viaRegisterType::RR);

    via_jmpto(V, f->addr);

    return;
}

// Calls a C function pointer
// This allows for a more flexible datatype, and avoids another layer of pointers in the viaValue union
// C functions in via must return void and are called with VirtualMachine *
// Type: void(*)(VirtualMachine *)
inline void via_callc(viaState *V, const viaCFunc &cf)
{
    viaInstruction *ipc = V->ip;
    V->stack->push(StackFrame(ipc, V->gc));

    cf(V);

    V->stack->pop();

    return;
}

// Returns the primitive type of value <v>
inline viaValue via_type(viaState *V, viaValue v)
{
    auto enum_name = ENUM_NAME(v.type);
    std::string stdstr = std::string(enum_name);
    const char *str = stdstr.c_str();
    via_gcadd(V, str);
    return viaT_stackvalue(V, viaT_newstring(V, str));
}

// Generalized call interface
// Used to call a nil-able viaValue
// Terminates if the value is not callable
// Callable types include;
// - Func
// - CFunc
// - viaTable (if __call method is present)
inline void via_call(viaState *V, viaValue v, bool has_self)
{
    V->hasself = has_self;

    if (v.type == viaValueType::Func)
        via_callf(V, v.fun);
    else if (v.type == viaValueType::CFunc)
        via_callc(V, v.cfun);
    else if (v.type == viaValueType::Table)
    {
        viaTableKey mhash = viaT_hashstring(V, "__call");
        viaValue *mmcall = via_gettableindex(V, v.tbl, mhash, true);
        via_call(V, *mmcall, has_self);
    }
    else
    {
        viaValue callt = via_type(V, v);
        via_setexitdata(V, 1, std::format("Attempt to call a {} value", callt.str->ptr));
        V->abrt = true;
    }

    return;
}

// Returns the length of value <v>, nil if impossible
inline viaValue via_len(viaState *V, viaValue val)
{
    if (val.type == viaValueType::String)
        return viaT_stackvalue(V, static_cast<viaNumber>(strlen(val.str->ptr)));
    else if (val.type == viaValueType::Table)
    {
        viaTableKey mhash = viaT_hashstring(V, "__len");
        viaValue *mmlen = via_gettableindex(V, val.tbl, mhash, true);

        if (mmlen->type == viaValueType::Nil)
            return viaT_stackvalue(V, static_cast<viaNumber>(val.tbl->data.size()));

        via_setregister(V, {viaRegisterType::AR, 0}, val);
        via_call(V, *mmlen, true);

        return *via_getregister(V, {viaRegisterType::RR, 0});
    }

    return viaT_stackvalue(V);
}

// Pushes value <v> to the back of table <t>
inline void via_inserttable(viaState *V, viaTable *T, viaValue &v)
{
    // No need to make this a pointer
    viaValue Tv = viaT_stackvalue(V, T);
    viaValue len = via_len(V, Tv);
    viaTableKey lhash = static_cast<viaTableKey>(len.num);
    via_settableindex(V, T, lhash, v);
    return;
}

// Loads the value of key <k> in table <t> into register <R>, if present in table
inline viaValue *via_loadtableindex(viaState *V, viaTable *T, viaTableKey key, viaRegister R)
{
    viaValue *val = via_gettableindex(V, T, key, true);
    via_setregister(V, R, val);
    return val;
}

// Returns the complex type of value <v>
// Practically the same as `vtype()`, but returns
// the `__type` value if the given table has one
inline viaValue via_typeof(viaState *V, viaValue v)
{
    if (v.type == viaValueType::Table)
    {
        viaTable *T = v.tbl;
        viaValue *ty = via_gettableindex(V, T, viaT_hashstring(V, "__type"), true);

        if (ty->type == viaValueType::Nil)
            return via_type(V, v);

        viaString *tystr = viaT_newstring(V, ty->str->ptr);

        return viaT_stackvalue(V, tystr);
    }

    return via_type(V, v);
}

// Calls the value of key <k> in table <t>, if callable
// Uses the `call` method internally
inline void via_callmethod(viaState *V, viaTable *T, viaTableKey key)
{
    viaValue *at = via_gettableindex(V, T, key, true);
    // ! This doesn't need to be a pointer because the underlying table type is already a pointer
    viaValue self = viaT_stackvalue(V, T);
    via_setregister(V, {viaRegisterType::AR, 0}, self);
    via_call(V, *at, true);

    return;
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

// I don't particularly like doing this
// However, it makes the code a lot cleaner
using VMStack = Stack<StackFrame>;
using Labels = std::unordered_map<std::string_view, viaInstruction *>;

//* Utility
inline viaValue via_toviavalue(viaState *V, const viaOperand &o)
{
    switch (o.type)
    {
    case viaOperandType::Number:
        return viaT_stackvalue(V, o.num);
    case viaOperandType::Bool:
        return viaT_stackvalue(V, o.boole);
    case viaOperandType::String:
        return viaT_stackvalue(V, viaT_newstring(V, o.str));
    default:
        via_setexitdata(V, 1, std::format("Cannot interpret operand '{}' as a data type", ENUM_NAME(o.type)));
        V->abrt = true;
        break;
    }

    return viaT_stackvalue(V);
}

// Loads a static library to the global environment
// If called during runtime, it will terminate the VM
inline void via_loadlib(viaState *V, viaGlobalIdentifier id, viaValue lib) noexcept
{
    if (V->ts == viaThreadState::RUNNING)
    {
        via_setexitdata(V, 1, "Attempt to load library during runtime");
        V->abrt = true;
        return;
    }

    if (via_getglobal(V, id)->type != viaValueType::Nil)
    {
        via_setexitdata(V, 1, std::format("Attempt to load library '{}' twice", id));
        V->abrt = true;
        return;
    }

    via_setglobal(V, id, lib);

    return;
}

inline void via_yield(viaState *V, float ms)
{
    if (V->ts == viaThreadState::RUNNING)
        V->yield = ms;

    return;
}

inline void via_savestate(viaState *V)
{
    V->sstate = new viaState(*V);
    return;
}

inline void via_restorestate(viaState *V)
{
    *V = *V->sstate;
    V->sstate = nullptr;
    return;
}

} // namespace via