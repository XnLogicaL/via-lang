/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "types.h"

namespace via
{

TValue &TValue::operator=(TValue &&other) noexcept
{
    if (this != &other)
    {
        switch (type)
        {
        case ValueType::String:
            if (!val_string)
                break;

            delete val_string;
            this->val_string = nullptr;
            break;
        case ValueType::Table:
            if (!val_table)
                break;

            delete val_table;
            break;
        case ValueType::Function:
            if (!val_table)
                break;

            delete val_function;
            break;
        case ValueType::CFunction:
            if (!val_cfunction)
                break;

            delete val_cfunction;
            break;
        default:
            break;
        }

        // Move the new state
        type = other.type;
        switch (type)
        {
        case ValueType::Number:
            val_number = other.val_number;
            break;
        case ValueType::Bool:
            val_boolean = other.val_boolean;
            break;
        case ValueType::String:
            val_string = other.val_string;
            other.val_string = nullptr;
            break;
        case ValueType::Table:
            val_table = other.val_table;
            other.val_table = nullptr;
            break;
        case ValueType::Function:
            val_function = other.val_function;
            other.val_function = nullptr;
            break;
        case ValueType::CFunction:
            val_cfunction = other.val_cfunction;
            other.val_cfunction = nullptr;
            break;
        default:
            break;
        }
        other.type = ValueType::Monostate;
    }
    return *this;
}

TValue::TValue(TValue &&other) noexcept
    : type(other.type)
{
    switch (type)
    {
    case ValueType::Number:
        this->val_number = other.val_number;
        break;
    case ValueType::Bool:
        this->val_boolean = other.val_boolean;
        break;
    case ValueType::String:
        this->val_string = other.val_string;
        other.val_string = nullptr;
        break;
    case ValueType::Table:
        this->val_table = other.val_table;
        other.val_table = nullptr;
        break;
    case ValueType::Function:
        this->val_function = other.val_function;
        other.val_function = nullptr;
        break;
    case ValueType::CFunction:
        this->val_cfunction = other.val_cfunction;
        other.val_cfunction = nullptr;
        break;
    default:
        break;
    }

    other.type = ValueType::Monostate;
}

TValue::TValue(const Operand &operand)
{
    switch (operand.type)
    {
    case OperandType::Number:
        this->val_number = operand.val_number;
        this->type = ValueType::Number;
        break;
    case OperandType::Bool:
        this->val_boolean = operand.val_boolean;
        this->type = ValueType::Bool;
        break;
    case OperandType::String:
        this->val_string = new TString(nullptr, operand.val_string);
        this->type = ValueType::String;
        break;
    case via::OperandType::Nil:
        this->type = ValueType::Nil;
        break;
    default:
        VIA_ASSERT(false, "Failed to construct TValue from Operand: invalid data type")
        break;
    }
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
        if (!val_table)
            break;

        delete val_table;
        break;
    case ValueType::Function:
        if (!val_table)
            break;

        delete val_function;
        break;
    case ValueType::CFunction:
        if (!val_cfunction)
            break;

        delete val_cfunction;
        break;
    default:
        break;
    }
}

TValue TValue::clone() const
{
    TValue copy;
    switch (type)
    {
    case ValueType::Number:
        copy.val_number = this->val_number;
        break;
    case ValueType::Bool:
        copy.val_boolean = this->val_boolean;
        break;
    case ValueType::String:
        copy.val_string = new TString(*this->val_string);
        break;
    case ValueType::Table:
        copy.val_table = new TTable(*this->val_table);
        break;
    case ValueType::Function:
        copy.val_function = new TFunction(*this->val_function);
        break;
    case ValueType::CFunction:
        copy.val_cfunction = new TCFunction(*this->val_cfunction);
        break;
    default:
        break;
    }
    copy.type = this->type;
    return copy;
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

TTable::TTable(TTable *meta, bool frozen, std::unordered_map<TableKey, TValue> init)
    : meta(meta)
    , frozen(frozen)
    , data(init)
{
}

} // namespace via
