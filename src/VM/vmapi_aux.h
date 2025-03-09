// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_VMAPI_AUX_H
#define _VIA_VMAPI_AUX_H

#include "common.h"
#include "function.h"

#define VIA_UPV_RESIZE_FACTOR 2

VIA_NAMESPACE_IMPL_BEGIN

// Automatically resizes upvalue vector of closure by VIA_UPV_RESIZE_FACTOR.
VIA_INLINE void __closure_upvs_resize(TFunction* _Closure) {
    U32      _Current_size = _Closure->upv_count;
    U32      _New_size     = _Current_size * VIA_UPV_RESIZE_FACTOR;
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

VIA_NAMESPACE_END

#endif
