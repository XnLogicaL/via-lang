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
    Function,  // Function type (custom object)
    CFunction, // CFunction type, C function pointer wrapper
    Table,     // Table type (custom object)
};

// Tagged union that holds a primitive via value
struct TValue
{
    ValueType type = ValueType::Monostate;
    union
    {
        TNumber val_number;
        TBool val_boolean;
        // These are pointers because their size is larger than 4 bytes,
        // which is what registers are supposed to hold
        TString *val_string;
        TFunction *val_function;
        TCFunction *val_cfunction;
        TTable *val_table;
    };

    TValue(const TValue &) = delete;            // Delete copy constructor
    TValue(TValue &&other) noexcept;            // Declare move constructor
    TValue &operator=(const TValue &) = delete; // Delete copy assignment operator
    TValue &operator=(TValue &&) noexcept;      // Declare move operator
    TValue(const Operand &);                    // Declare from-operand constructor
    ~TValue();                                  // Declare destructor

    explicit TValue()
        : type(ValueType::Nil)
    {
    }

    explicit TValue(TNumber x)
        : type(ValueType::Number)
        , val_number(x)
    {
    }

    explicit TValue(TBool b)
        : type(ValueType::Bool)
        , val_boolean(b)
    {
    }

    explicit TValue(TString *str)
        : type(ValueType::String)
        , val_string(str)
    {
    }

    explicit TValue(TFunction *fun)
        : type(ValueType::Function)
        , val_function(fun)
    {
    }

    explicit TValue(TCFunction *cfun)
        : type(ValueType::CFunction)
        , val_cfunction(cfun)
    {
    }

    explicit TValue(TTable *tbl)
        : type(ValueType::Table)
        , val_table(tbl)
    {
    }

    TValue clone() const;
};

struct TString
{
    const char *ptr = nullptr;
    uint32_t len = 0;
    Hash hash = 0;

    explicit TString(State *, const char *);
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

    explicit TFunction(State *, const char *, Instruction *, TFunction *caller, std::vector<Instruction>, bool, bool);
    ~TFunction() = default;
};

struct TCFunction
{
    using Ptr_t = void (*)(State *);

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
    std::unordered_map<TableKey, TValue> data = {};

    TTable(TTable *meta = nullptr, bool frozen = false, std::unordered_map<TableKey, TValue> init = {});
};

// Random hashing algo, may need to be replaced later
VIA_FORCEINLINE Hash hashstring(State *, const char *str)
{
    Hash hash = 0;
    while (*str)
        hash = (hash * 31) + *str++;

    return hash;
}

VIA_FORCEINLINE bool checkmonostate(State *, TValue &val)
{
    return val.type == ValueType::Monostate;
}

VIA_FORCEINLINE bool checknumber(State *, TValue &val)
{
    return val.type == ValueType::Number;
}

VIA_FORCEINLINE bool checkbool(State *, TValue &val)
{
    return val.type == ValueType::Bool;
}

VIA_FORCEINLINE bool checknil(State *, TValue &val)
{
    return val.type == ValueType::Nil;
}

VIA_FORCEINLINE bool checkstring(State *, TValue &val)
{
    return val.type == ValueType::String;
}

VIA_FORCEINLINE bool checktable(State *, TValue &val)
{
    return val.type == ValueType::Table;
}

VIA_FORCEINLINE bool checkcfunction(State *, TValue &val)
{
    return val.type == ValueType::CFunction;
}

VIA_FORCEINLINE bool checkfunction(State *, TValue &val)
{
    return val.type == ValueType::Function;
}

VIA_FORCEINLINE bool checkempty(State *V, TValue &val)
{
    return checknil(V, val) || checkmonostate(V, val);
}

VIA_FORCEINLINE bool checkcallable(State *V, TValue &val)
{
    return checkfunction(V, val) || checkcfunction(V, val);
}

VIA_FORCEINLINE bool checksubscriptable(State *V, TValue &val)
{
    return checktable(V, val) || checkstring(V, val);
}

} // namespace via
