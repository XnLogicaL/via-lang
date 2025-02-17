/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "types.h"

namespace via {

TValue &TValue::operator=(TValue &&other) noexcept
{
    if (this != &other) {
        switch (type) {
        case ValueType::string:
            if (!val_string)
                break;

            delete val_string;
            this->val_string = nullptr;
            break;
        case ValueType::table:
            if (!val_table)
                break;

            delete val_table;
            break;
        case ValueType::function:
            if (!val_table)
                break;

            delete val_function;
            break;
        case ValueType::cfunction:
            if (!val_cfunction)
                break;

            delete val_cfunction;
            break;
        default:
            break;
        }

        // Move the new state
        type = other.type;
        switch (type) {
        case ValueType::number:
            val_number = other.val_number;
            break;
        case ValueType::boolean:
            val_boolean = other.val_boolean;
            break;
        case ValueType::string:
            val_string = other.val_string;
            other.val_string = nullptr;
            break;
        case ValueType::table:
            val_table = other.val_table;
            other.val_table = nullptr;
            break;
        case ValueType::function:
            val_function = other.val_function;
            other.val_function = nullptr;
            break;
        case ValueType::cfunction:
            val_cfunction = other.val_cfunction;
            other.val_cfunction = nullptr;
            break;
        default:
            break;
        }
        other.type = ValueType::monostate;
    }
    return *this;
}

TValue::TValue(TValue &&other) noexcept
    : type(other.type)
{
    switch (type) {
    case ValueType::number:
        this->val_number = other.val_number;
        break;
    case ValueType::boolean:
        this->val_boolean = other.val_boolean;
        break;
    case ValueType::string:
        this->val_string = other.val_string;
        other.val_string = nullptr;
        break;
    case ValueType::table:
        this->val_table = other.val_table;
        other.val_table = nullptr;
        break;
    case ValueType::function:
        this->val_function = other.val_function;
        other.val_function = nullptr;
        break;
    case ValueType::cfunction:
        this->val_cfunction = other.val_cfunction;
        other.val_cfunction = nullptr;
        break;
    default:
        break;
    }

    other.type = ValueType::monostate;
}

TValue::TValue(const Operand &operand)
{
    switch (operand.type) {
    case OperandType::Number:
        this->val_number = operand.val_number;
        this->type = ValueType::number;
        break;
    case OperandType::Bool:
        this->val_boolean = operand.val_boolean;
        this->type = ValueType::boolean;
        break;
    case OperandType::String:
        this->val_string = new TString(nullptr, operand.val_string);
        this->type = ValueType::string;
        break;
    case via::OperandType::Nil:
        this->type = ValueType::nil;
        break;
    default:
        VIA_ASSERT(false, "Failed to construct TValue from Operand: invalid data type")
        break;
    }
}

TValue::~TValue()
{
    // Cleanup underlying type, if present
    switch (type) {
    case ValueType::string:
        if (!val_string)
            break;

        delete val_string;
        this->val_string = nullptr;
        break;
    case ValueType::table:
        if (!val_table)
            break;

        delete val_table;
        break;
    case ValueType::function:
        if (!val_table)
            break;

        delete val_function;
        break;
    case ValueType::cfunction:
        if (!val_cfunction)
            break;

        delete val_cfunction;
        break;
    default:
        break;
    }
}

TValue TValue::clone() const noexcept
{
    TValue copy;
    switch (type) {
    case ValueType::number:
        copy.val_number = this->val_number;
        break;
    case ValueType::boolean:
        copy.val_boolean = this->val_boolean;
        break;
    case ValueType::string:
        copy.val_string = new TString(*this->val_string);
        break;
    case ValueType::table:
        copy.val_table = new TTable(*this->val_table);
        break;
    case ValueType::function:
        copy.val_function = new TFunction(*this->val_function);
        break;
    case ValueType::cfunction:
        copy.val_cfunction = new TCFunction(*this->val_cfunction);
        break;
    default:
        break;
    }
    copy.type = this->type;
    return copy;
}

TString::TString(State *V, const char *str)
{
    Hash hash = hash_string(str);
    // For compiler compatability
    if (V != nullptr) {
        StrTable stable = V->G->stable;
        auto it = stable.find(hash);
        if (it != stable.end()) { // String already exists, return the existing entry
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

    if (V != nullptr) {
        StrTable &stable = V->G->stable;
        // Insert the new string into the stable
        stable.emplace(hash, this);
    }
}

TString::~TString()
{
    if (this->ptr) {
        delete[] this->ptr;
        this->ptr = nullptr;
    }
}

TFunction::TFunction(
    State *,
    std::string id,
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

TCFunction::TCFunction(TCFunction::CFunctionPtr ptr, bool error_handler)
    : ptr(ptr)
    , error_handler(error_handler)
{
}

} // namespace via
