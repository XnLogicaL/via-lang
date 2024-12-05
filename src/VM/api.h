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

namespace via
{

// Manually sets the VM exit data
// ! Internal usage only, made public for libraries
inline void via_setexitdata(viaState *V, viaExitCode_t exitc, const std::string &exitm) noexcept
{
    V->exitc = exitc;
    // I don't know why but `strdup` is deprecated but `_strdup` isn't
    // This is the completely opposite of the deprecation convention, but sure...
    V->exitm = strdup(exitm.c_str());
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
inline void via_jmp(viaState *V, viaJmpOffset_t offset)
{
    viaInstruction *addr = V->ip + offset;
    via_jmpto(V, addr);
    return;
}

// Appends a pointer to the garbage collector free list
// The pointer should be malloc-ed, otherwise this has undefined behavior
// ! The value cannot be removed from the free list!
template<typename T>
inline void via_gcadd(viaState *V, T ptr) noexcept
{
    viaGC_add(V->gc, ptr);
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
}

//* viaRegister operations

// Sets register <R> to the given value <v>
template<typename T = viaValue>
inline void via_setregister(viaState *V, viaRegister R, T val) noexcept
{
    VIA_ASSERT(!std::is_pointer_v<T>, "via_setregister(): Expected non-pointer value to assign to register");
    viaR_setregister(V->ralloc, R, val);
}

// Returns the value of register <R>
inline viaValue *via_getregister(viaState *V, viaRegister R) noexcept
{
    return viaR_getregister(V->ralloc, R);
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
        return v0.val_number == v1.val_number;
    case viaValueType::Bool:
        return v0.val_boolean == v1.val_boolean;
    case viaValueType::String:
        if (v0.val_string->ptr && v1.val_string->ptr)
        {
            if (strlen(v0.val_string->ptr) != strlen(v1.val_string->ptr))
                return false;
            return !strcmp(v0.val_string->ptr, v1.val_string->ptr);
        }
        return v0.val_string->ptr == v1.val_string->ptr;
    case viaValueType::Nil:
        return true; // Nil values are always equal
    case viaValueType::Ptr:
        return v0.val_pointer == v1.val_pointer;
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
        return V0.val_boolean == V1.val_boolean;
    case viaValueType::Number:
        return V0.val_number == V1.val_number;
    case viaValueType::Nil:
        // Nil values are always equal
        return true;
    case viaValueType::String:
        return !strcmp(V0.val_string->ptr, V1.val_string->ptr);
    case viaValueType::Ptr:
        return V0.val_pointer == V1.val_pointer;
    case viaValueType::Func:
        return V0.val_function == V1.val_function;
    case viaValueType::CFunc:
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
inline viaValue via_getvariable(viaState *V, viaVariableIdentifier_t id)
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
inline viaValue via_loadvariable(viaState *V, viaVariableIdentifier_t id, viaRegister reg)
{
    viaValue val = via_getvariable(V, id);
    via_setregister(V, reg, val);

    return val;
}

// Sets the variable declared with <id>
inline void via_setvariable(viaState *V, viaVariableIdentifier_t id, viaValue val)
{
    for (viaFunction *frame : *V->stack)
    {
        viaValue var = frame->locals[id];

        // Check if the variable has been declared or not
        // It can never be monostate again if declared once
        if (viaT_checkmonostate(V, var))
            continue;

        frame->locals[id] = val;
    }
}

inline void via_setconst(viaState *V, viaVariableIdentifier_t id, viaValue val)
{
    viaFunction *top = viaS_top(V->stack);
    viaValue existing = top->consts.at(id);

    if (!viaT_checkmonostate(V, existing))
        return;

    top->consts[id] = val;
}

inline viaValue via_getconst(viaState *V, viaVariableIdentifier_t id)
{
    viaFunction *top = viaS_top(V->stack);
    return top->consts.at(id);
}

inline viaValue via_loadconst(viaState *V, viaVariableIdentifier_t id, viaRegister R)
{
    viaFunction *top = viaS_top(V->stack);
    viaValue val = top->consts.at(id);

    via_setregister(V, R, val);

    return val;
}

// Similar to `via_getvariable` but explicitly looks for the variable in the global scope, a.k.a the root caller
inline viaValue *via_getglobal(viaState *V, viaVariableIdentifier_t id)
{
    // Yes, this is kinda hacky but it should do
    // Same as *V->stack->sbp
    viaFunction *global = *V->stack->end();
    viaValue *val = &global->locals[id];
    return val;
}

// Similar to `via_loadvariable` but explicitly looks for the variable in the global scope
inline viaValue *via_loadglobal(viaState *V, viaVariableIdentifier_t id, viaRegister reg)
{
    viaValue *val = via_getglobal(V, id);
    via_setregister(V, reg, *val);
    return val;
}

// Similar to `via_setvariable` but explicitly sets the variable in the global scope
inline void via_setglobal(viaState *V, viaVariableIdentifier_t id, viaValue val)
{
    viaFunction *global = *V->stack->end();
    global->locals[id] = val;
}

//* viaValue operations

// Returns a value that contains a String that represents the stringified version of <v>
// ! The return value is guaranteed to be a String
inline viaValue &via_tostring(viaState *V, viaValue &val)
{
    if (viaT_checkstring(V, val))
        return val;

    switch (val.type)
    {
    case viaValueType::Number:
    {
        viaString *str = viaT_newstring(V, std::to_string(val.val_number).c_str());
        val.val_string = str;
        break;
    }
    case viaValueType::Bool:
    {
        viaString *str = viaT_newstring(V, val.val_boolean ? "true" : "false");
        val.val_string = str;
        break;
    }
    case viaValueType::Table:
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
    case viaValueType::Func:
    {
        const void *faddr = val.val_function;
        viaString *str = viaT_newstring(V, std::format("<function@{}>", faddr).c_str());
        val.val_string = str;
        break;
    }
    case viaValueType::CFunc:
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

    val.type = viaValueType::String;
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
    case viaValueType::Nil:
        val.val_boolean = false;
        break;
    default:
        val.val_boolean = true;
        break;
    }

    val.type = viaValueType::Bool;
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
    case viaValueType::String:
        val.val_number = std::stod(val.val_string->ptr);
        break;
    case viaValueType::Bool:
        val.val_number = val.val_boolean ? 1.0f : 0.0f;
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
inline void via_settableindex(viaState *V, viaTable *T, viaTableKey key, viaValue val)
{
    if (viaT_checknil(V, val))
    {
        if (via_gettableindex(V, T, key, false)->type != viaValueType::Nil)
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
        // Save VM state to be restored when the function stops executing
        V->sstate = new viaState(*V);
        // Fucking hack, why are these iterators
        V->ihp = f->bytecode.begin()._Ptr;
        V->ibp = f->bytecode.end()._Ptr;
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

    viaFunction *freplica = new viaFunction{
        0,
        cf->error_handler,
        buffer,
        *V->stack->begin(),
        {},
        {},
        {},
    };

    viaS_push(V->stack, freplica);
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
    via_gcadd(V, str);
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
    V->argc = V->arguments->size;

    if (viaT_checkfunction(V, val))
        via_callf(V, val.val_function, true);
    else if (viaT_checkcfunction(V, val))
        via_callc(V, val.val_cfunction);
    else if (viaT_checktable(V, val))
    {
        viaTableKey mhash = viaT_hashstring(V, "__call");
        viaValue *mmcall = via_gettableindex(V, val.val_table, mhash, true);

        // Push self argument
        via_pushargument(V, *mmcall);
        via_call(V, *mmcall);
    }

    viaValue callt = via_type(V, val);
    via_setexitdata(V, 1, std::format("Attempt to call a {} value", callt.val_string->ptr));
    V->abrt = true;
}

inline void via_fastcall1(viaState *V, viaValue val, viaRegister arg0)
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
        viaTableKey mhash = viaT_hashstring(V, "__len");
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
inline viaValue *via_loadtableindex(viaState *V, viaTable *T, viaTableKey key, viaRegister R)
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
inline void via_callmethod(viaState *V, viaTable *T, viaTableKey key)
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
inline viaValue via_toviavalue(viaState *V, const viaOperand &operand)
{
    switch (operand.type)
    {
    case viaOperandType_t::Number:
        return viaT_stackvalue(V, operand.val_number);
    case viaOperandType_t::Bool:
        return viaT_stackvalue(V, operand.val_boolean);
    case viaOperandType_t::String:
        return viaT_stackvalue(V, viaT_newstring(V, operand.val_string));
    default:
        via_setexitdata(V, 1, std::format("Cannot interpret operand '{}' as a data type", ENUM_NAME(operand.type)));
        V->abrt = true;
        break;
    }

    return viaT_stackvalue(V);
}

// Loads a static library to the global environment
// If called during runtime, it will terminate the VM
inline void via_loadlib(viaState *V, viaVariableIdentifier_t id, viaValue lib) noexcept
{
    if (V->tstate == viaThreadState::RUNNING)
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

} // namespace via