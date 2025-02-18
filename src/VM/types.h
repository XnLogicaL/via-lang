/* This file is a part of the via programming language at
 * https://github.com/XnLogicaL/via-lang, see LICENSE for license
 * information */

#pragma once

#include "common.h"
#include "state.h"
#include "modifiable_once.h"

namespace via {

enum class ValueType {
    monostate, // Represents uninitialized values
    nil, // Empty values, like null, does not contain a union counterpart
    number,    // Unified number type, f64
    boolean,   // Bool type, wrapper for `bool`
    string,    // String type, pointer to owning string object
    function,  // Function type (custom object)
    cfunction, // CFunction type, C function pointer wrapper
    table,     // Table type (custom object)
};

// Tagged union that holds a primitive via value
struct TValue {
    ValueType type = ValueType::monostate;
    union {
        TNumber val_number;
        TBool val_boolean;
        TString *val_string;
        TFunction *val_function;
        TCFunction *val_cfunction;
        TTable *val_table;
    };

    TValue(const TValue &) = delete;            // Copy constructor
    TValue(TValue &&other) noexcept;            // Move constructor
    TValue &operator=(const TValue &) = delete; // Assignment operator
    TValue &operator=(TValue &&) noexcept;      // Move operator
    TValue(const Operand &);
    ~TValue();

    explicit TValue()
        : type(ValueType::nil)
    {
    }

    explicit TValue(TNumber x)
        : type(ValueType::number)
        , val_number(x)
    {
    }

    explicit TValue(TBool b)
        : type(ValueType::boolean)
        , val_boolean(b)
    {
    }

    explicit TValue(TString *str)
        : type(ValueType::string)
        , val_string(str)
    {
    }

    explicit TValue(TFunction *fun)
        : type(ValueType::function)
        , val_function(fun)
    {
    }

    explicit TValue(TCFunction *cfun)
        : type(ValueType::cfunction)
        , val_cfunction(cfun)
    {
    }

    explicit TValue(TTable *tbl)
        : type(ValueType::table)
        , val_table(tbl)
    {
    }

    TValue clone() const noexcept;
};

struct TString {
    const char *ptr = nullptr;
    U32 len = 0;
    Hash hash = 0;

    explicit TString(State *, const char *);
    TString() = default;
    ~TString();
};

struct TFunction {
    // Line information, determined during compile time,
    // Inherited from the instruction that declares this function
    size_t line = 0;
    // Tells the VM wether if this caller can handle an error
    bool error_handler = false;
    // Tells the VM if the last argument is a variadic argument
    bool is_vararg = false;
    // Function identifier
    std::string id = "<anonymous-function>";
    TFunction *caller = nullptr;
    Instruction *ret_addr = nullptr;
    std::vector<Instruction> bytecode = {};

    explicit TFunction(
        State *,
        std::string,
        Instruction *,
        TFunction *,
        std::vector<Instruction>,
        bool,
        bool
    );

    TFunction() = default;
};

struct TCFunction {
    using CFunctionPtr = void (*)(State *);

    // Function pointer
    CFunctionPtr ptr = nullptr;
    // Tells the VM if the function can handle errors or not.
    // This gets passed down to the replica function that represents this C
    // functions stack frame. Pretty much only used for protected call.
    bool error_handler = false;

    explicit TCFunction(
        CFunctionPtr ptr = nullptr,
        bool error_handler = false
    );

    TCFunction() = default;
};

struct TTable {
    // Pointer to metatable
    TTable *meta = nullptr;
    // Tells the VM if the table is modifiable
    utils::ModifiableOnce<bool> frozen = false;
    std::unordered_map<TableKey, TValue> data;

    TTable() = default;
    TTable(const TTable &other)
        : meta(other.meta)
        , frozen(other.frozen)
    {
        for (const auto &pair : other.data) {
            data.emplace(pair.first, pair.second.clone());
        }
    }
};

// Random hashing algo, may need to be replaced later
VIA_FORCEINLINE Hash hash_string(const char *str)
{
    Hash hash = 0;
    while (*str) {
        hash = (hash * 31) + *str++;
    }

    return hash;
}

VIA_FORCEINLINE bool check_monostate(const TValue &val)
{
    return val.type == ValueType::monostate;
}

VIA_FORCEINLINE bool check_number(const TValue &val)
{
    return val.type == ValueType::number;
}

VIA_FORCEINLINE bool check_bool(const TValue &val)
{
    return val.type == ValueType::boolean;
}

VIA_FORCEINLINE bool check_nil(const TValue &val)
{
    return val.type == ValueType::nil;
}

VIA_FORCEINLINE bool check_string(const TValue &val)
{
    return val.type == ValueType::string;
}

VIA_FORCEINLINE bool check_table(const TValue &val)
{
    return val.type == ValueType::table;
}

VIA_FORCEINLINE bool check_cfunction(const TValue &val)
{
    return val.type == ValueType::cfunction;
}

VIA_FORCEINLINE bool check_function(const TValue &val)
{
    return val.type == ValueType::function;
}

VIA_FORCEINLINE bool check_empty(const TValue &val)
{
    return check_nil(val) || check_monostate(val);
}

VIA_FORCEINLINE bool check_callable(const TValue &val)
{
    return check_function(val) || check_cfunction(val);
}

VIA_FORCEINLINE bool check_subscriptable(const TValue &val)
{
    return check_table(val) || check_string(val);
}

} // namespace via
