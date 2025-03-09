// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "strutils.h"
#include "rttypes.h"
#include "object.h"
#include "function.h"

VIA_NAMESPACE_BEGIN

using enum ValueType;

// Move-assignment operator, moves values from other object
TValue& TValue::operator=(TValue&& other) noexcept {
    if (this != &other) {
        // Move the value based on type
        switch (other.type) {
        case integer:
            val_integer = other.val_integer;
            break;
        case floating_point:
            val_floating_point = other.val_floating_point;
            break;
        case boolean:
            val_boolean = other.val_boolean;
            break;
        case string:
        case table:
        case function:
        case cfunction:
            val_pointer = other.val_pointer;
            break;
        default:
            break;
        }

        type       = other.type;
        other.type = nil;
    }

    return *this;
}

// Move constructor, transfer ownership based on type
TValue::TValue(TValue&& other) noexcept : type(other.type) {
    switch (other.type) {
    case integer:
        val_integer = other.val_integer;
        break;
    case floating_point:
        val_floating_point = other.val_floating_point;
        break;
    case boolean:
        val_boolean = other.val_boolean;
        break;
    case string:
    case table:
    case function:
    case cfunction:
        val_pointer = other.val_pointer;
        break;
    default:
        break;
    }

    other.type = nil;
}

// Frees the resources of the TValue depending on type
TValue::~TValue() {
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

    val_pointer = nullptr;
    type        = nil;
}

// Return a clone of the TValue based on its type
VIA_NO_DISCARD TValue TValue::clone() const noexcept {
    switch (type) {
    case integer:
        return TValue(val_integer);
    case floating_point:
        return TValue(val_floating_point);
    case boolean:
        return TValue(val_boolean);
    case string:
        return TValue(string, new TString(*cast_ptr<TString>()));
    case table:
        return TValue(table, new TTable(*cast_ptr<TTable>()));
    case function:
        return TValue(function, new TFunction(*cast_ptr<TFunction>()));
    case cfunction:
        return TValue(cfunction, new TCFunction(*cast_ptr<TCFunction>()));
    default:
        return TValue();
    }

    VIA_UNREACHABLE;
}

// Constructs a new TString object
TString::TString(State* V, const char* str) {
    if (V != nullptr) {
        auto& stable = V->G->stable;
        auto  it     = stable.find(hash);
        if (it != stable.end()) {
            return;
        }
    }

    len  = std::strlen(str);
    data = duplicate_string(str);
    hash = hash_string(str);

    if (V != nullptr) {
        V->G->stable.emplace(hash, this);
    }
}

// Frees TString resources if not already
TString::~TString() {
    if (data) {
        delete[] data;  // Free the allocated string memory
        data = nullptr; // Null data pointer
    }
}

VIA_NAMESPACE_END
