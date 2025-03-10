// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_OBJECT_H
#define _VIA_OBJECT_H

#include "common.h"
#include "state.h"

VIA_NAMESPACE_BEGIN

#ifdef VIA_64BIT

using TInteger = I64;
using TFloat   = F64;

#else

using TInteger = I32;
using TFloat   = F32;

#endif

enum class ValueType : U8 {
    nil,            // Empty type, null
    integer,        // Integer type
    floating_point, // Floating point type
    boolean,        // Boolean type
    string,         // String type, pointer to TString
    function,       // Function type, pointer to TFunction
    cfunction,      // CFunction type, pointer to TCFunction
    table,          // Table type, pointer to TTable
    object,         // Object type, pointer to TObject
};

struct VIA_ALIGN_CACHE_LINE TValue {
    ValueType type;
    union {
        TInteger val_integer;        // Integer value
        TFloat   val_floating_point; // Floating point value
        bool     val_boolean;        // Boolean value
        void*    val_pointer;        // Pointer to complex types (string, function, etc.)
    };

    VIA_CUSTOM_DESTRUCTOR(TValue);
    VIA_NON_COPYABLE(TValue);

    TValue(TValue&& other) noexcept;
    TValue& operator=(TValue&&) noexcept;

    /* clang-format off */
    explicit TValue()                           : type(ValueType::nil) {}
    explicit TValue(bool b)                     : type(ValueType::boolean), val_boolean(b) {}
    explicit TValue(TInteger x)                 : type(ValueType::integer), val_integer(x) {}
    explicit TValue(TFloat x)                   : type(ValueType::floating_point), val_floating_point(x) {}
    explicit TValue(ValueType type, void* ptr)  : type(type), val_pointer(ptr) {}
    /* clang-format on */

    VIA_NO_DISCARD TValue clone() const noexcept;

    template<typename T>
    VIA_NO_DISCARD VIA_INLINE_HOT_NODEBUG T* cast_ptr() const noexcept {
        return reinterpret_cast<T*>(val_pointer);
    }
};

struct TString {
    U32         len;
    U32         hash;
    const char* data;

    VIA_NON_DEFAULT_CONSTRUCTIBLE(TString);
    VIA_CUSTOM_DESTRUCTOR(TString);

    explicit TString(State*, const char*);
};

struct THashNode {
    const char* key;
    TValue      value;
    THashNode*  next;
};

struct TTable {
    SIZE arr_capacity   = 64;
    SIZE ht_capacity    = 1024;
    SIZE arr_size_cache = 0;
    SIZE ht_size_cache  = 0;

    bool arr_size_cache_valid = true;
    bool ht_size_cache_valid  = true;

    TValue*     arr_array  = new TValue[arr_capacity]();
    THashNode** ht_buckets = new THashNode*[ht_capacity];

    VIA_DEFAULT_CONSTRUCTOR(TTable);
    VIA_CUSTOM_DESTRUCTOR(TTable);

    TTable(const TTable&);
};

VIA_NAMESPACE_END

#endif
