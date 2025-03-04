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

struct TTable {
    bool is_frozen = false;

    TTable *meta = nullptr;
    std::unordered_map<U32, TValue> data;

    TTable() = default;
    TTable(const TTable &other)
        : is_frozen(other.is_frozen)
        , meta(other.meta)
    {
        for (const auto &pair : other.data) {
            data.emplace(pair.first, pair.second.clone());
        }
    }
};

} // namespace via
