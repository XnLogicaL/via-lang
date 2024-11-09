#ifndef VIA_TYPES_H
#define VIA_TYPES_H

#include "common.h"
#include "util/modifiable_once.h"
#include "util/callable_once.h"

namespace via {
namespace VM {

struct Instruction;

using via_Number = double;
using via_Bool = bool;
using via_String = char*;
using via_Nil = std::nullptr_t;
using via_Ptr = std::uintptr_t;
using via_CFunc = std::function<void(void)>;

template <typename ...T>
using via_Variant = std::variant<T...>;

struct via_Value;  // Forward declaration
struct via_Table;  // Forward declaration

struct via_Func
{
    VM::Instruction* address;
};

struct via_TableKey
{
    enum class KType : uint8_t
    {
        Number,
        String
    };

    KType type;
    union {
        via_Number num_val;
        via_String str_val;
    };
};

struct via_TableKeyHash
{
    std::size_t operator()(const via_TableKey& key) const;
};

struct via_TableKeyEqual
{
    bool operator()(const via_TableKey& lhs, const via_TableKey& rhs) const;
};

struct via_Value
{
    enum class VType : uint8_t
    {
        Number,
        Bool,
        String,
        Nil,
        Ptr,
        Func,
        CFunc,
        Table,
        Vector,
        TableKey,
    };

    via_Value() : type(VType::Nil), nil_val(nullptr) {}
    explicit via_Value(const via_Number& d) : type(VType::Number), num_val(d) {}
    explicit via_Value(const via_Bool& b) : type(VType::Bool), bool_val(b) {}
    via_Value(const via_String& c) : type(VType::String), str_val(strdup(c)) {}
    explicit via_Value(const via_Ptr& p) : type(VType::Ptr), ptr_val(p) {}
    explicit via_Value(const via_Table& t);
    explicit via_Value(const via_Func& f) : type(VType::Func), fun_val(new via_Func(f)) {}
    explicit via_Value(const via_CFunc& cf) : type(VType::CFunc), cfun_val(new via_CFunc(cf)) {}
    explicit via_Value(const via_TableKey& tk) : type(VType::TableKey), tblkey_val(new via_TableKey(tk)) {}

    via_Value(const via_Value& other);
    via_Value& operator=(const via_Value& other);

    ~via_Value() {
        if (type == VType::String)
        {
            std::free(str_val);
        }
    };

    VType type;
    union {
        via_Number num_val;
        via_Bool bool_val;
        via_String str_val;
        via_Nil nil_val;
        via_Ptr ptr_val;
        via_Func* fun_val;
        via_CFunc* cfun_val;
        via_Table* tbl_val;
        via_TableKey* tblkey_val;
    };

private:
    void cleanup();
};

struct via_Table
{
    util::modifiable_once<bool> is_frozen;
    std::unordered_map<via_TableKey, via_Value, via_TableKeyHash, via_TableKeyEqual> data;

    via_Value& get(const via_TableKey& key);
    void set(const via_TableKey& key, const via_Value& val);
};

} // namespace VM
} // namespace via

#endif // VIA_TYPES_H
