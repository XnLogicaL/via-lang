// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "types.h"

namespace via {

using enum ValueType;

TValue &TValue::operator=(TValue &&other) noexcept
{
    if (this != &other) {
        if (!val_pointer) {
            return *this;
        }

        switch (type) {
        case string:
            delete cast_ptr<TString>();
            break;
        case table:
            delete cast_ptr<TTable>();
            break;
        case function:
            delete cast_ptr<TFunction>();
            break;
        case cfunction:
            delete cast_ptr<TCFunction>();
            break;
        default:
            break;
        }

        this->val_pointer = nullptr;
        this->type = other.type;

        switch (type) {
        case integer:
            this->val_integer = other.val_integer;
            break;
        case floating_point:
            this->val_floating_point = other.val_floating_point;
            break;
        case boolean:
            this->val_boolean = other.val_boolean;
            break;
        case string:
        case table:
        case function:
        case cfunction:
            this->val_pointer = other.val_pointer;
            break;
        default:
            break;
        }

        other.type = nil;
    }
    return *this;
}

TValue::TValue(TValue &&other) noexcept
    : type(other.type)
{
    switch (type) {
    case integer:
        this->val_integer = other.val_integer;
        break;
    case floating_point:
        this->val_floating_point = other.val_floating_point;
        break;
    case boolean:
        this->val_boolean = other.val_boolean;
        break;
    case string:
    case table:
    case function:
    case cfunction:
        this->val_pointer = other.val_pointer;
        break;
    default:
        break;
    }

    other.type = nil;
}

TValue::~TValue()
{
    // Cleanup underlying type, if present
    if (!val_pointer) {
        return;
    }

    switch (type) {
    case string:
        delete cast_ptr<TString>();
        break;
    case table:
        delete cast_ptr<TTable>();
        break;
    case function:
        delete cast_ptr<TFunction>();
        break;
    case cfunction:
        delete cast_ptr<TCFunction>();
        break;
    default:
        break;
    }

    this->val_pointer = nullptr;
    this->type = nil;
}

TValue TValue::clone() const noexcept
{
    TValue copy;
    switch (type) {
    case integer:
        copy.val_integer = this->val_integer;
        break;
    case floating_point:
        copy.val_floating_point = this->val_floating_point;
        break;
    case boolean:
        copy.val_boolean = this->val_boolean;
        break;
    case string:
        copy.val_pointer = new TString(*this->cast_ptr<TString>());
        break;
    case table:
        copy.val_pointer = new TTable(*this->cast_ptr<TTable>());
        break;
    case function:
        copy.val_pointer = new TFunction(*this->cast_ptr<TFunction>());
        break;
    case cfunction:
        copy.val_pointer = new TCFunction(*this->cast_ptr<TCFunction>());
        break;
    default:
        break;
    }

    copy.type = this->type;
    return copy;
}

TString::TString(State *V, const char *str)
{
    U32 hash = hash_string(str);
    // For compiler compatability
    if (V != nullptr) {
        std::unordered_map<U32, TString *> stable = V->G->stable;
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
    this->data = sptr;
    this->hash = hash;

    if (V != nullptr) {
        std::unordered_map<U32, TString *> &stable = V->G->stable;
        // Insert the new string into the stable
        stable.emplace(hash, this);
    }
}

TString::~TString()
{
    if (this->data) {
        delete[] this->data;
        this->data = nullptr;
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

TCFunction::TCFunction(TCFunction::CFunctionPtr data, bool error_handler)
    : data(data)
    , error_handler(error_handler)
{
}

} // namespace via
