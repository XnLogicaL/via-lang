// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "state.h"
#include "modifiable_once.h"

namespace via {

enum class ValueType {
    nil,            // Empty values, like null, does not contain a union counterpart
    integer,        // Integer type (i32)
    floating_point, // Float type (f32)
    boolean,        // Bool type, wrapper for `bool`
    string,         // String type, pointer to owning string object
    function,       // Function type (custom object)
    cfunction,      // CFunction type, C function pointer wrapper
    table,          // Table type
    object,
};

// Tagged union that holds a primitive via value
struct TValue {
    ValueType type = ValueType::nil;
    union {
        int val_integer;
        float val_floating_point;
        bool val_boolean;
        void *val_pointer;
    };

    TValue(const TValue &) = delete;            // Copy constructor
    TValue(TValue &&other) noexcept;            // Move constructor
    TValue &operator=(const TValue &) = delete; // Assignment operator
    TValue &operator=(TValue &&) noexcept;      // Move operator
    ~TValue();

    explicit TValue()
        : type(ValueType::nil)
    {
    }

    explicit TValue(int x)
        : type(ValueType::integer)
        , val_integer(x)
    {
    }

    explicit TValue(float x)
        : type(ValueType::floating_point)
        , val_floating_point(x)
    {
    }

    explicit TValue(bool boolean)
        : type(ValueType::boolean)
        , val_boolean(boolean)
    {
    }

    explicit TValue(ValueType type, void *ptr)
        : type(type)
        , val_pointer(ptr)
    {
    }

    TValue clone() const noexcept;

    template<typename T>
    [[nodiscard]] VIA_MAXOPTIMIZE T *cast_ptr() const noexcept
    {
        return reinterpret_cast<T *>(val_pointer);
    }
};

struct TString {
    const char *data = dup_string("");
    U32 len = 0;
    U32 hash = 0;

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
    CFunctionPtr data = nullptr;
    // Tells the VM if the function can handle errors or not.
    // This gets passed down to the replica function that represents this C
    // functions stack frame. Pretty much only used for protected call.
    bool error_handler = false;

    explicit TCFunction(CFunctionPtr data = nullptr, bool error_handler = false);

    TCFunction() = default;
};

struct TTable {
    // Pointer to metatable
    TTable *meta = nullptr;
    // Tells the VM if the table is modifiable
    utils::ModifiableOnce<bool> frozen = false;
    std::unordered_map<U32, TValue> data;

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

VIA_FORCEINLINE U32 hash_string(const char *str)
{
    static const constexpr U32 BASE = 31;
    static const constexpr U32 MOD = 0xFFFFFFFF;

    U32 hash = 0;
    while (char chr = *str++) {
        hash = (hash * BASE + static_cast<U32>(chr)) % MOD;
    }

    return hash;
}

VIA_FORCEINLINE bool check_integer(const TValue &val)
{
    return val.type == ValueType::integer;
}

VIA_FORCEINLINE bool check_floating_point(const TValue &val)
{
    return val.type == ValueType::floating_point;
}

VIA_FORCEINLINE bool check_number(const TValue &val)
{
    return check_integer(val) || check_floating_point(val);
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

VIA_FORCEINLINE bool check_callable(const TValue &val)
{
    return check_function(val) || check_cfunction(val);
}

VIA_FORCEINLINE bool check_subscriptable(const TValue &val)
{
    return check_table(val) || check_string(val);
}

} // namespace via
