/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "types.h"

#define INIT_HEAP \
    { \
        if (V != nullptr) \
        { \
            if (V->heapptr != nullptr) \
                V->heapptr->next = this; \
            this->prev = V->heapptr; \
            V->heapptr = this; \
        } \
    }

namespace via
{

TValue::TValue()
    : type(ValueType::Nil)
{
}

TValue::TValue(TNumber x)
    : type(ValueType::Number)
    , val_number(x)
{
}

TValue::TValue(TBool b)
    : type(ValueType::Bool)
    , val_boolean(b)
{
}

TValue::TValue(TString *str)
    : type(ValueType::String)
    , val_string(str)
{
}

TValue::TValue(TFunction *fun)
    : type(ValueType::Function)
    , val_function(fun)
{
}

TValue::TValue(TCFunction *cfun)
    : type(ValueType::CFunction)
    , val_cfunction(cfun)
{
}

TValue::TValue(TTable *tbl)
    : type(ValueType::Table)
    , val_table(tbl)
{
}

TValue::TValue(RTState *V)
    : type(ValueType::Nil) INIT_HEAP;

TValue::TValue(RTState *V, TNumber x)
    : type(ValueType::Number)
    , val_number(x) INIT_HEAP;

TValue::TValue(RTState *V, TBool b)
    : type(ValueType::Bool)
    , val_boolean(b) INIT_HEAP;

TValue::TValue(RTState *V, TString *str)
    : type(ValueType::String)
    , val_string(str) INIT_HEAP;

TValue::TValue(RTState *V, TFunction *fun)
    : type(ValueType::Function)
    , val_function(fun) INIT_HEAP;

TValue::TValue(RTState *V, TCFunction *cfun)
    : type(ValueType::CFunction)
    , val_cfunction(cfun) INIT_HEAP;

TValue::TValue(RTState *V, TTable *tbl)
    : type(ValueType::Table)
    , val_table(tbl) INIT_HEAP;

TValue::TValue(RTState *V, TValue val)
    : type(val.type)
{
    switch (val.type)
    {
    case ValueType::Number:
        val_number = val.val_number;
        break;
    case ValueType::Bool:
        val_boolean = val.val_boolean;
        break;
    case ValueType::String:
        delete val_string;
        val_string = val.val_string;
        break;
    case ValueType::Function:
        delete val_function;
        val_function = val.val_function;
        break;
    case ValueType::Table:
        delete val_table;
        val_table = val.val_table;
        break;
    case ValueType::CFunction:
        delete val_cfunction;
        val_cfunction = val.val_cfunction;
        break;
    case ValueType::Pointer:
        val_pointer = val.val_pointer;
        break;
    default:
        break;
    }

    INIT_HEAP;
}

TValue::~TValue()
{
    // Cleanup underlying type, if present
    switch (type)
    {
    case ValueType::String:
        delete val_string;
        break;
    case ValueType::Table:
        delete val_table;
        break;
    case ValueType::Function:
        delete val_function;
        break;
    case ValueType::CFunction:
        delete val_cfunction;
        break;
    default:
        break;
    }
}

TString::TString(RTState *V, const char *str)
{
    Hash hash = hashstring(V, str);
    // For compiler compatability
    if (V != nullptr)
    {
        // Retrieve the string table
        StrTable *stable = V->G->stable;
        // Check if the string already exists in the stable
        auto it = stable->find(hash);
        if (it != stable->end())
        { // String already exists, return the existing entry
            *this = *it->second;
            return;
        }
    }

    size_t slen = std::strlen(str);
    // Allocate new string
    char *sptr = new char[slen + 1];

    // Copy the constant string into the owned string
    std::memcpy(sptr, str, slen);

    this->len = slen;
    this->ptr = sptr; // No need for static_cast<const char*>
    this->hash = hash;

    if (V != nullptr)
    {
        StrTable *stable = V->G->stable;
        // Insert the new string into the stable
        (*stable)[hash] = this;
    }
}

TString::~TString()
{
    delete[] ptr;
}

TFunction::TFunction(
    RTState *,
    const char *id,
    Instruction *ret_addr,
    TFunction *caller,
    std::vector<Instruction> bytecode,
    bool is_error_handler,
    bool is_var_arg
)
    : line(std::numeric_limits<size_t>::max())
    , error_handler(is_error_handler)
    , is_vararg(is_var_arg)
    , id(id)
    , caller(caller)
    , ret_addr(ret_addr)
    , bytecode(bytecode)
{
}

TCFunction::TCFunction(TCFunction::Ptr_t ptr, bool error_handler)
    : ptr(ptr)
    , error_handler(error_handler)
{
}

TTable::TTable(TTable *meta, bool frozen, HashMap<TableKey, TValue> init)
    : meta(meta)
    , frozen(frozen)
    , data(init)
{
}

} // namespace via
