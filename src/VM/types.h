/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "state.h"
#include "Utils/modifiable_once.h"

namespace via
{

enum class ValueType
{
    Monostate, // Represents uninitialized values
    Nil,       // Empty values, like null, does not contain a union counterpart
    Number,    // Unified number type, f64
    Bool,      // Bool type, wrapper for `bool`
    String,    // String type, pointer to owning string object
    Pointer,   // Pointer type, wrapper for `uintptr_t`
    Function,  // Function type (custom object)
    CFunction, // CFunction type, C function pointer wrapper
    Table,     // Table type (custom object)
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

    // Stack value constructors
    explicit TValue();
    explicit TValue(TNumber);
    explicit TValue(TBool);
    explicit TValue(TPointer);
    explicit TValue(TString *);
    explicit TValue(TFunction *);
    explicit TValue(TCFunction *);
    explicit TValue(TTable *);

    // Heap value constructors
    explicit TValue(RTState *);
    explicit TValue(RTState *, TNumber);
    explicit TValue(RTState *, TBool);
    explicit TValue(RTState *, TPointer);
    explicit TValue(RTState *, TString *);
    explicit TValue(RTState *, TFunction *);
    explicit TValue(RTState *, TCFunction *);
    explicit TValue(RTState *, TTable *);
    explicit TValue(RTState *, TValue);

    ~TValue();
};

struct TString
{
    const char *ptr = nullptr;
    uint32_t len = 0;
    Hash hash = 0;

    explicit TString(RTState *, const char *);
    ~TString();
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

    explicit TFunction(RTState *, const char *, Instruction *, TFunction *caller, std::vector<Instruction>, bool, bool);
    ~TFunction() = default;
};

struct TCFunction
{
    using Ptr_t = void (*)(RTState *);

    // Function pointer
    Ptr_t ptr = nullptr;
    // Tells the VM if the function can handle errors or not
    // This gets passed down to the replica function that represents this C functions stack frame
    // Pretty much only used for pcall
    bool error_handler = false;

    explicit TCFunction(Ptr_t ptr = nullptr, bool error_handler = false);
    ~TCFunction() = default;
};

struct TTable
{
    // Pointer to metatable
    TTable *meta = nullptr;
    // Tells the VM if the table is modifiable
    utils::modifiable_once<bool> frozen = false;
    HashMap<TableKey, TValue> data = {};

    TTable(TTable *meta = nullptr, bool frozen = false, HashMap<TableKey, TValue> init = {});
    ~TTable() = default;
};

// Random hashing algo, may need to be replaced later
VIA_FORCEINLINE Hash hashstring(RTState *, const char *str)
{
    Hash hash = 0;
    while (*str)
        hash = (hash * 31) + *str++;

    return hash;
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
    return val.type == ValueType::Pointer;
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
    return val.type == ValueType::CFunction;
}

VIA_FORCEINLINE bool checkfunction(RTState *, TValue val)
{
    return val.type == ValueType::Function;
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
