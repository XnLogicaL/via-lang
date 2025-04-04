// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef vl_has_header_vaux_h
#define vl_has_header_vaux_h

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
vl_implement void __closure_upvs_resize(function_obj* closure) {
  uint32_t current_size = closure->upv_count;
  uint32_t new_size = current_size * 2;
  upv_obj* new_location = new upv_obj[new_size];

  // Move upvalues to new location
  for (upv_obj* ptr = closure->upvs; ptr < closure->upvs + current_size; ptr++) {
    uint32_t offset = ptr - closure->upvs;
    new_location[offset] = std::move(*ptr);
  }

  // Free old location
  delete[] closure->upvs;
  // Update closure
  closure->upvs = new_location;
  closure->upv_count = new_size;
}

// Checks if a given index is within the bounds of the upv_obj vector of the closure.
// Used for resizing.
vl_implement bool __closure_upvs_range_check(function_obj* closure, size_t index) {
  return closure->upv_count >= index;
}

// Attempts to retrieve upv_obj at index <upv_id>.
// Returns nullptr if <upv_id> is out of upv_obj vector bounds.
vl_implement upv_obj* __closure_upv_get(function_obj* closure, size_t upv_id) {
  if (!__closure_upvs_range_check(closure, upv_id)) {
    return nullptr;
  }

  return &closure->upvs[upv_id];
}

// Dynamically reassigns upv_obj at index <upv_id> the value <val>.
vl_implement void __closure_upv_set(function_obj* closure, size_t upv_id, value_obj& val) {
  upv_obj* _Upv = __closure_upv_get(closure, upv_id);
  if (_Upv != nullptr) {
    if (_Upv->value != nullptr) {
      *_Upv->value = val.clone();
    }
    else {
      _Upv->value = &val;
    }

    _Upv->is_valid = true;
  }
}

// Loads closure bytecode by iterating over the instruction pipeline.
// Handles sentinel/special opcodes like RETURN or CAPTURE while assembling closure.
vl_implement void __closure_bytecode_load(state* state, function_obj* closure) {
  std::vector<instruction> cache;

  while (state->pc < state->iep) {
    // Special case: Terminator opcode
    if (state->pc->op == opcode::RETURN || state->pc->op == opcode::RETURNNIL) {
      cache.push_back(*state->pc);
      ++state->pc;
      break;
    }

    cache.push_back(*state->pc);
    ++state->pc;
  }

  closure->ibp = new instruction[cache.size()];
  closure->iep = closure->ibp + cache.size();

  std::memcpy(closure->ibp, cache.data(), cache.size());

  for (value_obj* _Stk_id = state->sbp + state->sp; _Stk_id > state->sbp; _Stk_id--) {
    size_t _Pos = _Stk_id - state->sbp;

    if (__closure_upvs_range_check(closure, _Pos)) {
      __closure_upvs_resize(closure);
    }

    closure->upvs[_Pos] = {
      .is_open = true,
      .value = _Stk_id,
    };
  }
}

// Moves upvalues of the current closure into the heap, "closing" them.
vl_implement void __closure_close_upvalues(function_obj* closure) {
  upv_obj* upv_end = closure->upvs + closure->upv_count;
  for (upv_obj* upv = closure->upvs; upv < upv_end; upv++) {
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
vl_implement size_t __table_ht_hash_key(const table_obj* table, const char* key) {
  size_t hash = 2166136261u;
  while (*key) {
    hash = (hash ^ *key++) * 16777619;
  }

  return hash % table->ht_capacity;
}

// Inserts a key-value pair into the hash table component of a given table_obj object.
vl_implement void __table_ht_set(table_obj* table, const char* key, const value_obj& val) {
  size_t index = __table_ht_hash_key(table, key);

  hash_node_obj* next_node = table->ht_buckets[index];
  hash_node_obj* node = nullptr;

  // Check if the key already exists in the list
  while (next_node != nullptr) {
    if (strcmp(next_node->key, key) == 0) {
      next_node->value = val.clone();
      return;
    }
    next_node = next_node->next;
  }

  node = new hash_node_obj{key, val.clone(), table->ht_buckets[index]};

  table->ht_size_cache_valid = false;
  table->ht_buckets[index] = node;
}

// Performs a look-up on the given table with a given key. Returns nullptr upon lookup failure.
vl_implement value_obj* __table_ht_get(const table_obj* table, const char* key) {
  size_t index = __table_ht_hash_key(table, key);

  for (hash_node_obj* node = table->ht_buckets[index]; node; node = node->next) {
    if (strcmp(node->key, key) == 0) {
      return &node->value;
    }
  }

  return nullptr; // Not found
}

// Returns the real size_t of the hashtable component of the given table object.
vl_implement size_t __table_ht_size(table_obj* table) {
  if (table->ht_size_cache_valid) {
    return table->ht_size_cache;
  }

  size_t size = 0;
  hash_node_obj* current_node = *table->ht_buckets;

  while (current_node->next) {
    const value_obj& val = current_node->value;
    if (!val.is_nil()) {
      size++;
    }

    current_node = current_node->next;
  }

  table->ht_size_cache = size;
  table->ht_size_cache_valid = true;

  return size;
}

// Checks if the given index is out of bounds of a given tables array component.
vl_implement bool __table_arr_range_check(const table_obj* table, size_t index) {
  return table->arr_capacity > index;
}

// Dynamically grows and relocates the array component of a given table_obj object.
vl_implement void __table_arr_resize(table_obj* table) {
  size_t old_capacity = table->arr_capacity;
  size_t new_capacity = old_capacity * 2;

  value_obj* old_location = table->arr_array;
  value_obj* new_location = new value_obj[new_capacity]();

  for (value_obj* ptr = old_location; ptr < old_location + old_capacity; ptr++) {
    size_t position = ptr - old_location;
    new_location[position] = std::move(*ptr);
  }

  table->arr_array = new_location;
  table->arr_capacity = new_capacity;

  delete[] old_location;
}

// Sets the given index of a table to a given value. Resizes the array component of the table_obj
// object if necessary.
vl_implement void __table_arr_set(table_obj* table, size_t index, const value_obj& val) {
  if (!__table_arr_range_check(table, index)) {
    __table_arr_resize(table);
  }

  table->arr_size_cache_valid = false;
  table->arr_array[index] = val.clone();
}

// Attempts to get the value at the given index of the array component of the table. Returns nullptr
// if the index is out of array capacity range.
vl_implement value_obj* __table_arr_get(const table_obj* table, size_t index) {
  if (!__table_arr_range_check(table, index)) {
    return nullptr;
  }

  return &table->arr_array[index];
}

// Returns the real size_t of the given tables array component.
vl_implement size_t __table_arr_size(table_obj* table) {
  if (table->arr_size_cache_valid) {
    return table->arr_size_cache;
  }

  size_t size = 0;

  for (value_obj* ptr = table->arr_array; ptr < table->arr_array + table->arr_capacity; ptr++) {
    if (!ptr->is_nil()) {
      size++;
    }
  }

  table->arr_size_cache = size;
  table->arr_size_cache_valid = true;

  return size;
}

vl_implement void __table_set(table_obj* table, const value_obj& key, const value_obj& val) {
  if (key.is_int()) {
    __table_arr_set(table, key.val_integer, val);
  }
  else if (key.is_string()) {
    const string_obj* _Val_string = key.cast_ptr<string_obj>();
    __table_ht_set(table, _Val_string->data, val);
  }
}

vl_implement value_obj __table_get(const table_obj* table, const value_obj& key) {
  value_obj* unsafe = nullptr;
  if (key.is_int()) {
    unsafe = __table_arr_get(table, key.val_integer);
  }
  else if (key.is_string()) {
    unsafe = __table_ht_get(table, key.cast_ptr<string_obj>()->data);
  }

  return unsafe ? unsafe->clone() : value_obj();
}

vl_implement size_t __table_size(table_obj* table) {
  return __table_arr_size(table) + __table_ht_size(table);
}

// ==========================================================
// Label handling
vl_implement void __label_allocate(state* state, size_t count) {
  state->labels = new instruction*[count];
}

vl_implement void __label_deallocate(state* state) {
  if (state->labels) {
    delete[] state->labels;
    state->labels = nullptr;
  }
}

vl_implement instruction* __label_get(state* state, size_t index) {
  return state->labels[index];
}

vl_implement void __label_load(state* state) {
  size_t index = 0;
  for (instruction* pc = state->ibp; pc < state->iep; pc++) {
    if (pc->op == opcode::LABEL) {
      state->labels[index++] = pc;
    }
  }
}

// ==========================================================
// Stack handling
vl_implement void __stack_allocate(state* state) {
  state->sbp = new value_obj[vl_vmstacksize]();
}

vl_implement void __stack_deallocate(state* state) {
  delete[] state->sbp;
}

vl_implement void __push(state* state, const value_obj& val) {
  state->sbp[state->sp++] = val.clone();
}

vl_implement value_obj __pop(state* state) {
  return state->sbp[state->sp--].move();
}

vl_implement void __drop(state* state) {
  value_obj& dropped = state->sbp[state->sp--];
  dropped.reset();
}

vl_implement const value_obj& __get_stack(state* state, size_t offset) {
  return state->sbp[offset];
}

vl_implement void __set_stack(state* state, size_t offset, value_obj& val) {
  state->sbp[offset] = val.clone();
}

vl_implement value_obj __get_argument(state* vl_restrict state, size_t offset) {
  if (offset >= state->frame->call_info.argc) {
    return value_obj();
  }

  // Compute stack offset in reverse order
  const operand_t stk_offset = state->frame->call_info.sp - state->frame->call_info.argc + offset;
  const value_obj& val = state->sbp[stk_offset];

  return val.clone();
}

// ==========================================================
// Register handling
vl_implement void __register_allocate(state* state) {
  state->registers = new value_obj[vl_regcount]();
}

vl_implement void __register_deallocate(state* state) {
  delete[] state->registers;
}

vl_implement void __set_register(state* state, operand_t reg, const value_obj& val) {
  value_obj* addr = state->registers + reg;
  *addr = val.clone();
}

vl_implement value_obj* __get_register(state* state, operand_t reg) {
  value_obj* addr = state->registers + reg;
  return addr;
}

} // namespace via::impl

#endif
