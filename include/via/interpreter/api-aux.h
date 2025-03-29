// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _vl_vaux_h
#define _vl_vaux_h

#include "memory-utility.h"
#include "common.h"
#include "function.h"

// ===========================================================================================
// api-aux.h
//
namespace via::impl {
// ===========================================================================================
// Closure handling

// Automatically resizes upv_obj vector of closure by VIA_UPV_RESIZE_FACTOR.
vl_inline void __closure_upvs_resize(function_obj* _Closure) {
  uint32_t _Current_size = _Closure->upv_count;
  uint32_t _New_size = _Current_size * 2;
  upv_obj* _New_location = new upv_obj[_New_size];

  // Move upvalues to new location
  for (upv_obj* ptr = _Closure->upvs; ptr < _Closure->upvs + _Current_size; ptr++) {
    uint32_t offset = ptr - _Closure->upvs;
    _New_location[offset] = std::move(*ptr);
  }

  // Free old location
  delete[] _Closure->upvs;
  // Update closure
  _Closure->upvs = _New_location;
  _Closure->upv_count = _New_size;
}

// Checks if a given index is within the bounds of the upv_obj vector of the closure.
// Used for resizing.
vl_inline bool __closure_upvs_range_check(function_obj* _Closure, size_t index) {
  return _Closure->upv_count >= index;
}

// Attempts to retrieve upv_obj at index <_Upv_id>.
// Returns nullptr if <_Upv_id> is out of upv_obj vector bounds.
vl_inline upv_obj* __closure_upv_get(function_obj* _Closure, size_t _Upv_id) {
  if (!__closure_upvs_range_check(_Closure, _Upv_id)) {
    return nullptr;
  }

  return &_Closure->upvs[_Upv_id];
}

// Dynamically reassigns upv_obj at index <_Upv_id> the value <_Val>.
vl_inline void __closure_upv_set(function_obj* _Closure, size_t _Upv_id, value_obj& _Val) {
  upv_obj* _Upv = __closure_upv_get(_Closure, _Upv_id);
  if (_Upv != nullptr) {
    if (_Upv->value != nullptr) {
      *_Upv->value = _Val.clone();
    }
    else {
      _Upv->value = &_Val;
    }

    _Upv->is_valid = true;
  }
}

// Loads closure bytecode by iterating over the instruction pipeline.
// Handles sentinel/special opcodes like RETURN or CAPTURE while assembling closure.
vl_inline void __closure_bytecode_load(state* _State, function_obj* _Closure) {
  std::vector<instruction> cache;

  while (_State->pc < _State->iep) {
    // Special case: Terminator opcode
    if (_State->pc->op == opcode::RETURN || _State->pc->op == opcode::RETURNNIL) {
      cache.push_back(*_State->pc);
      ++_State->pc;
      break;
    }

    cache.push_back(*_State->pc);
    ++_State->pc;
  }

  _Closure->ibp = new instruction[cache.size()];
  _Closure->iep = _Closure->ibp + cache.size();

  std::memcpy(_Closure->ibp, cache.data(), cache.size());

  for (value_obj* _Stk_id = _State->sbp + _State->sp; _Stk_id > _State->sbp; _Stk_id--) {
    size_t _Pos = _Stk_id - _State->sbp;

    if (__closure_upvs_range_check(_Closure, _Pos)) {
      __closure_upvs_resize(_Closure);
    }

    _Closure->upvs[_Pos] = {
      .is_open = true,
      .value = _Stk_id,
    };
  }
}

// Moves upvalues of the current closure into the heap, "closing" them.
vl_inline void __closure_close_upvalues(function_obj* _Closure) {
  upv_obj* _Upv_end = _Closure->upvs + _Closure->upv_count;
  for (upv_obj* upv = _Closure->upvs; upv < _Upv_end; upv++) {
    if (upv->is_valid && upv->is_open) {
      upv->heap_value = upv->value->clone();
      upv->value = &upv->heap_value;
      upv->is_open = false;
    }
  }
}

// ======================================================================================
// Table handling

// Hashes a dictionary key using the FNV-1a hashing algorithm.
vl_inline size_t __table_ht_hash_key(const table_obj* _Tbl, const char* _Key) {
  size_t _Hash = 2166136261u;
  while (*_Key) {
    _Hash = (_Hash ^ *_Key++) * 16777619;
  }

  return _Hash % _Tbl->ht_capacity;
}

// Inserts a key-value pair into the hash table component of a given table_obj object.
vl_inline void __table_ht_set(table_obj* _Tbl, const char* _Key, const value_obj& _Val) {
  size_t _Index = __table_ht_hash_key(_Tbl, _Key);

  hash_node_obj* _Next_node = _Tbl->ht_buckets[_Index];
  hash_node_obj* _Node = nullptr;

  // Check if the key already exists in the list
  while (_Next_node != nullptr) {
    if (strcmp(_Next_node->key, _Key) == 0) {
      _Next_node->value = _Val.clone();
      return;
    }
    _Next_node = _Next_node->next;
  }

  _Node = new hash_node_obj{_Key, _Val.clone(), _Tbl->ht_buckets[_Index]};

  _Tbl->ht_size_cache_valid = false;
  _Tbl->ht_buckets[_Index] = _Node;
}

// Performs a look-up on the given table with a given key. Returns nullptr upon lookup failure.
vl_inline value_obj* __table_ht_get(const table_obj* _Tbl, const char* _Key) {
  size_t _Index = __table_ht_hash_key(_Tbl, _Key);

  for (hash_node_obj* _Node = _Tbl->ht_buckets[_Index]; _Node; _Node = _Node->next) {
    if (strcmp(_Node->key, _Key) == 0) {
      return &_Node->value;
    }
  }

  return nullptr; // Not found
}

// Returns the real size_t of the hashtable component of the given table object.
vl_inline size_t __table_ht_size(table_obj* _Tbl) {
  if (_Tbl->ht_size_cache_valid) {
    return _Tbl->ht_size_cache;
  }

  size_t _Size = 0;
  hash_node_obj* _Current_node = *_Tbl->ht_buckets;

  while (_Current_node->next) {
    const value_obj& _Val = _Current_node->value;
    if (!_Val.is_nil()) {
      _Size++;
    }

    _Current_node = _Current_node->next;
  }

  _Tbl->ht_size_cache = _Size;
  _Tbl->ht_size_cache_valid = true;

  return _Size;
}

// Checks if the given index is out of bounds of a given tables array component.
vl_inline bool __table_arr_range_check(const table_obj* _Tbl, size_t _Index) {
  return _Tbl->arr_capacity > _Index;
}

// Dynamically grows and relocates the array component of a given table_obj object.
vl_inline void __table_arr_resize(table_obj* _Tbl) {
  size_t _Old_capacity = _Tbl->arr_capacity;
  size_t _New_capacity = _Old_capacity * 2;

  value_obj* _Old_location = _Tbl->arr_array;
  value_obj* _New_location = new value_obj[_New_capacity]();

  for (value_obj* _Ptr = _Old_location; _Ptr < _Old_location + _Old_capacity; _Ptr++) {
    size_t _Position = _Ptr - _Old_location;
    _New_location[_Position] = std::move(*_Ptr);
  }

  _Tbl->arr_array = _New_location;
  _Tbl->arr_capacity = _New_capacity;

  delete[] _Old_location;
}

// Sets the given index of a table to a given value. Resizes the array component of the table_obj
// object if necessary.
vl_inline void __table_arr_set(table_obj* _Tbl, size_t _Index, const value_obj& _Val) {
  if (!__table_arr_range_check(_Tbl, _Index)) {
    __table_arr_resize(_Tbl);
  }

  _Tbl->arr_size_cache_valid = false;
  _Tbl->arr_array[_Index] = _Val.clone();
}

// Attempts to get the value at the given index of the array component of the table. Returns nullptr
// if the index is out of array capacity range.
vl_inline value_obj* __table_arr_get(const table_obj* _Tbl, size_t _Index) {
  if (!__table_arr_range_check(_Tbl, _Index)) {
    return nullptr;
  }

  return &_Tbl->arr_array[_Index];
}

// Returns the real size_t of the given tables array component.
vl_inline size_t __table_arr_size(table_obj* _Tbl) {
  if (_Tbl->arr_size_cache_valid) {
    return _Tbl->arr_size_cache;
  }

  size_t _Size = 0;

  for (value_obj* _Ptr = _Tbl->arr_array; _Ptr < _Tbl->arr_array + _Tbl->arr_capacity; _Ptr++) {
    if (!_Ptr->is_nil()) {
      _Size++;
    }
  }

  _Tbl->arr_size_cache = _Size;
  _Tbl->arr_size_cache_valid = true;

  return _Size;
}

vl_inline void __table_set(table_obj* _Tbl, const value_obj& _Key, const value_obj& _Val) {
  if (_Key.is_int()) {
    __table_arr_set(_Tbl, _Key.val_integer, _Val);
  }
  else if (_Key.is_string()) {
    const string_obj* _Val_string = _Key.cast_ptr<string_obj>();
    __table_ht_set(_Tbl, _Val_string->data, _Val);
  }
}

vl_inline value_obj __table_get(const table_obj* _Tbl, const value_obj& _Key) {
  value_obj* _Unsafe = nullptr;
  if (_Key.is_int()) {
    _Unsafe = __table_arr_get(_Tbl, _Key.val_integer);
  }
  else if (_Key.is_string()) {
    _Unsafe = __table_ht_get(_Tbl, _Key.cast_ptr<string_obj>()->data);
  }

  return _Unsafe ? _Unsafe->clone() : value_obj();
}

vl_inline size_t __table_size(table_obj* _Tbl) {
  return __table_arr_size(_Tbl) + __table_ht_size(_Tbl);
}

// ==========================================================
// Label handling
vl_inline void __label_allocate(state* _State, size_t _Count) {
  _State->labels = new instruction*[_Count];
}

vl_inline void __label_deallocate(state* _State) {
  if (_State->labels) {
    delete[] _State->labels;
    _State->labels = nullptr;
  }
}

vl_inline instruction* __label_get(state* _State, size_t _Idx) {
  return _State->labels[_Idx];
}

vl_inline void __label_load(state* _State) {
  size_t _Idx = 0;
  for (instruction* _Ip = _State->ibp; _Ip < _State->iep; _Ip++) {
    if (_Ip->op == opcode::LABEL) {
      _State->labels[_Idx++] = _Ip;
    }
  }
}

// ==========================================================
// Stack handling
vl_inline void __stack_allocate(state* _State) {
  _State->sbp = new value_obj[vl_vmstacksize]();
}

vl_inline void __stack_deallocate(state* _State) {
  delete[] _State->sbp;
}

vl_optimize void __push(state* _State, const value_obj& _Val) {
  _State->sbp[_State->sp++] = _Val.clone();
}

vl_optimize value_obj __pop(state* _State) {
  return _State->sbp[_State->sp--].move();
}

vl_optimize void __drop(state* _State) {
  value_obj& _Dropped_val = _State->sbp[_State->sp--];
  _Dropped_val.reset();
}

vl_optimize const value_obj& __get_stack(state* _State, size_t _Offset) {
  return _State->sbp[_Offset];
}

vl_optimize void __set_stack(state* _State, size_t _Offset, value_obj& _Val) {
  _State->sbp[_Offset] = _Val.clone();
}

vl_forceinline value_obj __get_argument(state* vl_restrict _State, size_t _Offset) {
  if (_Offset >= _State->frame->call_info.argc) {
    return value_obj();
  }

  // Compute stack offset in reverse order
  const operand_t _Stk_offset =
    _State->frame->call_info.sp - _State->frame->call_info.argc + _Offset;
  const value_obj& _Val = _State->sbp[_Stk_offset];

  return _Val.clone();
}

// ==========================================================
// Register handling
vl_inline void __register_allocate(state* _State) {
  _State->registers = new value_obj[vl_regcount]();
}

vl_inline void __register_deallocate(state* _State) {
  delete[] _State->registers;
}

vl_optimize void __set_register(state* _State, operand_t _Reg, const value_obj& _Val) {
  value_obj* addr = _State->registers + _Reg;
  *addr = _Val.clone();
}

vl_optimize value_obj* __get_register(state* _State, operand_t _Reg) {
  value_obj* addr = _State->registers + _Reg;
  return addr;
}

} // namespace via::impl

#endif
