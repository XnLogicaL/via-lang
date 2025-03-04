// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "types.h"
#include "object.h"
#include "function.h"

namespace via {

using enum ValueType;

TValue &TValue::operator=(TValue &&other) noexcept
{
    if (this != &other) {
        switch (other.type) {
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

        this->type = other.type;
        other.type = nil;
    }

    return *this;
}

TValue::TValue(TValue &&other) noexcept
    : type(other.type)
{
    switch (other.type) {
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
    switch (type) {
    case integer:
        return TValue(val_integer);
    case floating_point:
        return TValue(val_floating_point);
    case boolean:
        return TValue(val_boolean);
    case string:
        return TValue(string, new TString(*this->cast_ptr<TString>()));
    case table:
        return TValue(table, new TTable(*this->cast_ptr<TTable>()));
    case function:
        return TValue(function, new TFunction(*this->cast_ptr<TFunction>()));
    case cfunction:
        return TValue(cfunction, new TCFunction(*this->cast_ptr<TCFunction>()));
    default:
        return TValue();
    }

    VIA_UNREACHABLE;
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

} // namespace via
