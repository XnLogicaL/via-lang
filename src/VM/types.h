/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"

#include "Utils/callable_once.h"
#include "Utils/modifiable_once.h"

namespace via::VM
{

static size_t obj_uid_head = 0;

struct Instruction;
class VirtualMachine;

using via_Number = double;
using via_Bool   = bool;
using via_String = char *;
using via_Nil    = std::nullptr_t;
using via_Ptr    = uintptr_t;
using via_CFunc  = void (*)(VirtualMachine *);

struct via_Value; // Forward declaration
struct via_Table; // Forward declaration

struct via_Func
{
    VM::Instruction *address;
};

struct via_TableKey
{
    enum class KType : uint8_t
    {
        Number,
        String
    };

    KType type;
    union
    {
        via_Number num;
        via_String str;
    };
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
        TableKey,
    };

    via_Value()
        : type(VType::Nil)
        , nil(nullptr)
    {
    }
    via_Value(const via_String &c)
        : type(VType::String)
        , str(strdup(c))
    {
    }

    explicit via_Value(const via_Number &d)
        : type(VType::Number)
        , num(d)
    {
    }
    explicit via_Value(const via_Bool &b)
        : type(VType::Bool)
        , boole(b)
    {
    }
    explicit via_Value(const via_Ptr &p)
        : type(VType::Ptr)
        , ptr(p)
    {
    }
    explicit via_Value(const via_Table &t);
    explicit via_Value(const via_Func &f)
        : type(VType::Func)
        , fun(new via_Func(f))
    {
    }
    explicit via_Value(via_CFunc cf)
        : type(VType::CFunc)
        , cfun(via_CFunc(cf))
    {
    }
    explicit via_Value(const via_TableKey &tk)
        : type(VType::TableKey)
        , tblkey(new via_TableKey(tk))
    {
    }

    via_Value(const via_Value &other);
    via_Value &operator=(const via_Value &other);

    ~via_Value()
    {
        if (type == VType::String)
        {
            std::free(str);
        }
    };

    bool is_const;
    VType type;

    union
    {
        via_Number num;
        via_Bool boole;
        via_String str;
        via_Nil nil;
        via_Ptr ptr;
        via_Func *fun;
        via_CFunc cfun;
        via_Table *tbl;
        via_TableKey *tblkey;
    };

private:
    void cleanup();
};

struct via_Table
{
    struct via_TableKeyHash
    {
        std::size_t operator()(const via_TableKey &key) const;
    };

    struct via_TableKeyEqual
    {
        bool operator()(const via_TableKey &lhs, const via_TableKey &rhs) const;
    };

    size_t uid;
    util::modifiable_once<bool> is_frozen;
    std::unordered_map<via_TableKey, via_Value, via_TableKeyHash, via_TableKeyEqual> data;

    via_Value &get(const via_TableKey &key);
    void set(const via_TableKey &key, const via_Value &val);

    via_Table()
        : uid(obj_uid_head++)
        , is_frozen(false)
        , data({})
    {
    }
};

using ValueType = via_Value::VType;

} // namespace via::VM
