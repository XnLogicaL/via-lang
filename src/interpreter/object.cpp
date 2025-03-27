// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "string-utility.h"
#include "object.h"
#include "function.h"
#include "api-aux.h"

VIA_NAMESPACE_BEGIN

using enum ValueType;

// Move-assignment operator, moves values from other object
TValue& TValue::operator=(TValue&& other) {
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
TValue::TValue(TValue&& other)
    : type(other.type) {
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
TValue TValue::clone() const {
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

void TValue::reset() {}

bool TValue::compare(const TValue&) const {
    return false;
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
    hash = hash_string_custom(str);

    if (V != nullptr) {
        V->G->stable.emplace(hash, this);
    }
}

TString::TString(const TString& other)
    : len(other.len),
      hash(other.hash) {
    data = duplicate_string(other.data);
}

TString::~TString() {
    delete[] data;
}

size_t TString::size() {
    return len;
}

void TString::set_string(size_t position, const TValue& value) {
    VIA_ASSERT(position < len, "String index position out of bounds");
    VIA_ASSERT(value.is_string(), "Setting string index to non-string value");

    const TString* val = value.cast_ptr<TString>();

    VIA_ASSERT(val->len == 1, "Setting string index to non-character string");

    data[position] = value.cast_ptr<TString>()->data[0];
}

TValue TString::get_string(size_t position) {
    VIA_ASSERT(position < len, "String index position out of bounds");
    char     chr  = data[position];
    TString* tstr = new TString(nullptr, &chr);
    return TValue(tstr);
}

TTable::~TTable() {
    if (ht_buckets) {
        for (size_t i = 0; i < ht_capacity; ++i) {
            THashNode* next = ht_buckets[i];
            while (next) {
                THashNode* current = next;
                next               = next->next;
                delete current;
            }
            ht_buckets[i] = nullptr;
        }
    }

    delete[] arr_array;
    delete[] ht_buckets;
}

TTable::TTable(const TTable& other)
    : arr_capacity(other.arr_capacity),
      ht_capacity(other.ht_capacity),
      arr_size_cache_valid(other.arr_size_cache_valid),
      ht_size_cache_valid(other.ht_size_cache_valid) {

    if (other.arr_array) {
        arr_array = new TValue[arr_capacity];
        for (size_t i = 0; i < arr_capacity; ++i) {
            arr_array[i] = other.arr_array[i].clone();
        }
    }
    else {
        arr_array = nullptr;
    }

    if (other.ht_buckets) {
        ht_buckets = new THashNode*[ht_capacity]();

        for (size_t i = 0; i < ht_capacity; ++i) {
            THashNode*  src = other.ht_buckets[i];
            THashNode** dst = &ht_buckets[i];

            while (src) {
                *dst = new THashNode{src->key, src->value.clone(), nullptr};
                dst  = &((*dst)->next);
                src  = src->next;
            }
        }
    }
    else {
        ht_buckets = nullptr;
    }
}

size_t TTable::size() {
    return impl::__table_size(this);
}

void TTable::set_table(const char* key, const TValue& value) {
    TValue index(new TString(nullptr, key));
    impl::__table_set(this, index, value);
}

void TTable::set_table(size_t position, const TValue& value) {
    TValue index(static_cast<TInteger>(position));
    impl::__table_set(this, index, value);
}

TValue TTable::get_table(const char* key) {
    TValue index(new TString(nullptr, key));
    return impl::__table_get(this, index);
}

TValue TTable::get_table(size_t position) {
    TValue index(static_cast<TInteger>(position));
    return impl::__table_get(this, index);
}

TObject::~TObject() {
    if (fields) {
        delete[] fields;
        fields = nullptr;
    }
}

VIA_NAMESPACE_END
