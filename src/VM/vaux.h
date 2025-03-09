// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_VMAPI_AUX_H
#define _VIA_VMAPI_AUX_H

#include "common.h"
#include "function.h"
#include "rttypes.h"

// ===========================================================================================
// vaux.h
//
VIA_NAMESPACE_IMPL_BEGIN
// ===========================================================================================
// Closure handling

// Automatically resizes upvalue vector of closure by VIA_UPV_RESIZE_FACTOR.
VIA_INLINE void __closure_upvs_resize(TFunction* _Closure) {
    U32      _Current_size = _Closure->upv_count;
    U32      _New_size     = _Current_size * 2;
    UpValue* _New_location = new UpValue[_New_size];

    // Move upvalues to new location
    for (UpValue* ptr = _Closure->upvs; ptr < _Closure->upvs + _Current_size; ptr++) {
        U32 offset            = ptr - _Closure->upvs;
        _New_location[offset] = std::move(*ptr);
    }

    // Free old location
    delete[] _Closure->upvs;
    // Update closure
    _Closure->upvs      = _New_location;
    _Closure->upv_count = _New_size;
}

// Checks if a given index is within the bounds of the upvalue vector of the closure.
// Used for resizing.
VIA_INLINE bool __closure_upvs_range_check(TFunction* _Closure, SIZE index) {
    return _Closure->upv_count >= index;
}

// Attempts to retrieve upvalue at index <_Upv_id>.
// Returns nullptr if <_Upv_id> is out of upvalue vector bounds.
VIA_INLINE UpValue* __closure_upv_get(TFunction* _Closure, SIZE _Upv_id) {
    if (!__closure_upvs_range_check(_Closure, _Upv_id)) {
        return nullptr;
    }

    return &_Closure->upvs[_Upv_id];
}

// Dynamically reassigns upvalue at index <_Upv_id> the value <_Val>.
VIA_INLINE void __closure_upv_set(TFunction* _Closure, SIZE _Upv_id, TValue& _Val) {
    UpValue* _Upv = __closure_upv_get(_Closure, _Upv_id);
    if (_Upv != nullptr) {
        if (_Upv->value != nullptr) {
            *_Upv->value = _Val.clone();
        }
        else {
            _Upv->value = &_Val;
        }
    }
}

// Loads closure bytecode by iterating over the instruction pipeline.
// Handles sentinel/special opcodes like RETURN or CAPTURE while assembling closure.
VIA_INLINE void __closure_bytecode_load(State* _State, TFunction* _Closure) {
    std::vector<Instruction> cache;

    while (_State->ip <= _State->iep) {
        // Special case: Terminator opcode
        if (_State->ip->op == OpCode::RETURN) {
            cache.push_back(*_State->ip++);
            break;
        }
        // Special case: Closure assembly-time capturing
        // Captures stack variables while the closure is being constructed
        // to ensure that no dangling references occur.
        else if (_State->ip->op == OpCode::CAPTURE) {
            Operand stk_id = _State->ip->operand0;
            Operand upv_id = _State->ip->operand1;

            TFunction* closure = _State->frame;
            TValue&    stk_val = _State->sbp[stk_id];

            // Check whether if the upvalue vector needs to be resized.
            if (__closure_upvs_range_check(closure, upv_id)) {
                __closure_upvs_resize(closure);
            }

            __closure_upv_set(_Closure, upv_id, stk_val);

            _State->ip++;
            continue;
        }

        cache.push_back(*_State->ip++);
    }

    _Closure->bytecode_len = cache.size();
    _Closure->bytecode     = new Instruction[cache.size()];

    std::memcpy(_Closure->bytecode, cache.data(), cache.size());
}

// Moves upvalues of the current closure into the heap, "closing" them.
VIA_INLINE void __closure_close_upvalues(TFunction* _Closure) {
    UpValue* _Upv_end = _Closure->upvs + _Closure->upv_count;
    for (UpValue* upv = _Closure->upvs; upv < _Upv_end; upv++) {
        if (upv->is_open) {
            upv->heap_value = upv->value->clone();
            upv->value      = &upv->heap_value;
            upv->is_open    = false;
        }
    }
}

// ======================================================================================
// Table handling

// Hashes a dictionary key using the FNV-1a hashing algorithm.
VIA_INLINE SIZE __table_ht_hash_key(TTable* _Tbl, const char* _Key) noexcept {
    SIZE _Hash = 2166136261u;

    while (*_Key) {
        _Hash = (_Hash ^ *_Key++) * 16777619;
    }

    return _Hash % _Tbl->ht_capacity;
}

// Inserts a key-value pair into the hash table component of a given TTable object.
VIA_INLINE void __table_ht_set(TTable* _Tbl, const char* _Key, const TValue& _Val) noexcept {
    SIZE _Index = __table_ht_hash_key(_Tbl, _Key);

    THashNode* _Next_node = _Tbl->ht_buckets[_Index];
    THashNode* _Node      = nullptr;

    // Check if the key already exists in the list
    while (_Next_node != nullptr) {
        if (strcmp(_Next_node->key, _Key) == 0) {
            _Next_node->value = _Val.clone();
            return;
        }
        _Next_node = _Next_node->next;
    }

    _Node = new THashNode{_Key, _Val.clone(), _Tbl->ht_buckets[_Index]};

    _Tbl->ht_size_cache_valid = false;
    _Tbl->ht_buckets[_Index]  = _Node;
}

// Performs a look-up on the given table with a given key. Returns nullptr upon lookup failure.
VIA_INLINE TValue* __table_ht_get(TTable* _Tbl, const char* _Key) noexcept {
    SIZE _Index = __table_ht_hash_key(_Tbl, _Key);

    for (THashNode* _Node = _Tbl->ht_buckets[_Index]; _Node; _Node = _Node->next) {
        if (strcmp(_Node->key, _Key) == 0) {
            return &_Node->value;
        }
    }

    return nullptr; // Not found
}

// Returns the real size of the hashtable component of the given table object.
VIA_INLINE SIZE __table_ht_size(TTable* _Tbl) noexcept {
    if (_Tbl->ht_size_cache_valid) {
        return _Tbl->ht_size_cache;
    }

    SIZE       _Size         = 0;
    THashNode* _Current_node = *_Tbl->ht_buckets;

    while (_Current_node->next) {
        const TValue& _Val = _Current_node->value;
        if (!check_nil(_Val)) {
            _Size++;
        }

        _Current_node = _Current_node->next;
    }

    _Tbl->ht_size_cache       = _Size;
    _Tbl->ht_size_cache_valid = true;

    return _Size;
}

// Checks if the given index is out of bounds of a given tables array component.
VIA_INLINE bool __table_arr_range_check(TTable* _Tbl, SIZE _Index) noexcept {
    return _Tbl->arr_capacity > _Index;
}

// Dynamically grows and relocates the array component of a given TTable object.
VIA_INLINE void __table_arr_resize(TTable* _Tbl) noexcept {
    SIZE _Old_capacity = _Tbl->arr_capacity;
    SIZE _New_capacity = _Old_capacity * 2;

    TValue* _Old_location = _Tbl->arr_array;
    TValue* _New_location = new TValue[_New_capacity]();

    for (TValue* _Ptr = _Old_location; _Ptr < _Old_location + _Old_capacity; _Ptr++) {
        SIZE _Position           = _Ptr - _Old_location;
        _New_location[_Position] = std::move(*_Ptr);
    }

    _Tbl->arr_array    = _New_location;
    _Tbl->arr_capacity = _New_capacity;

    delete[] _Old_location;
}

// Sets the given index of a table to a given value. Resizes the array component of the TTable
// object if necessary.
VIA_INLINE void __table_arr_set(TTable* _Tbl, SIZE _Index, const TValue& _Val) noexcept {
    if (!__table_arr_range_check(_Tbl, _Index)) {
        __table_arr_resize(_Tbl);
    }

    _Tbl->arr_size_cache_valid = false;
    _Tbl->arr_array[_Index]    = _Val.clone();
}

// Attempts to get the value at the given index of the array component of the table. Returns nullptr
// if the index is out of array capacity range.
VIA_INLINE TValue* __table_arr_get(TTable* _Tbl, SIZE _Index) noexcept {
    if (!__table_arr_range_check(_Tbl, _Index)) {
        return nullptr;
    }

    return &_Tbl->arr_array[_Index];
}

// Returns the real size of the given tables array component.
VIA_INLINE SIZE __table_arr_size(TTable* _Tbl) noexcept {
    if (_Tbl->arr_size_cache_valid) {
        return _Tbl->arr_size_cache;
    }

    SIZE _Size = 0;

    for (TValue* _Ptr = _Tbl->arr_array; _Ptr < _Tbl->arr_array + _Tbl->arr_capacity; _Ptr++) {
        if (!check_nil(*_Ptr)) {
            _Size++;
        }
    }

    _Tbl->arr_size_cache       = _Size;
    _Tbl->arr_size_cache_valid = true;

    return _Size;
}

VIA_INLINE void __table_set(TTable* _Tbl, const TValue& _Key, const TValue& _Val) {
    if (check_integer(_Key)) {
        __table_arr_set(_Tbl, _Key.val_integer, _Val);
    }
    else if (check_string(_Key)) {
        TString* _Val_string = _Key.cast_ptr<TString>();
        __table_ht_set(_Tbl, _Val_string->data, _Val);
    }
}

VIA_INLINE TValue* __table_get(TTable* _Tbl, const TValue& _Key) {
    static thread_local TValue nil;

    TValue* _Unsafe = nullptr;
    if (check_integer(_Key)) {
        _Unsafe = __table_arr_get(_Tbl, _Key.val_integer);
    }
    else if (check_string(_Key)) {
        _Unsafe = __table_ht_get(_Tbl, _Key.cast_ptr<TString>()->data);
    }

    return _Unsafe ? _Unsafe : &nil;
}

VIA_INLINE SIZE __table_size(TTable* _Tbl) {
    return __table_arr_size(_Tbl) + __table_ht_size(_Tbl);
}

VIA_NAMESPACE_END

#endif
