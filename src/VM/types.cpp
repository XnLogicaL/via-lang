/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "types.h"

namespace via
{

TValue::TValue()
    : type(ValueType::Nil)
{
}

TValue::TValue(const TValue &other)
    : type(other.type)
{
    switch (other.type)
    {
    case ValueType::Number:
        this->val_number = other.val_number;
        break;
    case ValueType::Bool:
        this->val_boolean = other.val_boolean;
        break;
    case ValueType::String:
        this->val_string = new TString(nullptr, other.val_string->ptr);
        break;
    case ValueType::Table:
        this->val_table = new TTable(other.val_table->meta, other.val_table->frozen.get(), other.val_table->data);
        break;
    case ValueType::Function:
        this->val_function = new TFunction(
            nullptr,
            other.val_function->id,
            other.val_function->ret_addr,
            other.val_function->caller,
            other.val_function->bytecode,
            other.val_function->error_handler,
            other.val_function->is_vararg
        );
        break;
    case ValueType::CFunction:
        this->val_cfunction = new TCFunction(other.val_cfunction->ptr, other.val_cfunction->error_handler);
        break;
    default:
        break;
    }
};

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

TValue::~TValue()
{
    // Cleanup underlying type, if present
    switch (type)
    {
    case ValueType::String:
        if (!val_string)
            break;

        delete val_string;
        this->val_string = nullptr;
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
        StrTable *stable = V->G->stable;
        auto it = stable->find(hash);
        if (it != stable->end())
        { // String already exists, return the existing entry
            // *this = *it->second;
            return;
        }
    }

    size_t slen = std::strlen(str);
    char *sptr = new char[slen + 1];

    // Copy the constant string into the owned string
    std::strcpy(sptr, str);

    this->len = slen;
    this->ptr = sptr;
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
    if (this->ptr)
    {
        delete[] this->ptr;
        this->ptr = nullptr;
    }
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
